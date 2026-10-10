#include "evaluation.hpp"
#include "exposure_gain.hpp"
#include "geometry.hpp"
#include "tone_math.hpp"
#include "detail.hpp"
#include "detail_math.hpp"
#include <cmath>
#include <limits>
#include <cfenv>
#include <cstring>
namespace pixaura::evaluation {
static_assert(sizeof(double)==8 && std::numeric_limits<double>::is_iec559, "IEEE binary64 required");
namespace {
void need(bool ok, int32_t code) { if (!ok) throw document::Failure{code}; }
bool finite(float value) {
    uint32_t bits=0;
    std::memcpy(&bits,&value,sizeof(bits));
    return (bits & 0x7f800000u) != 0x7f800000u;
}
}
double gain(int32_t ev) {
    need(ev >= -5000 && ev <= 5000, 7);
    // Floor quotient, nonnegative remainder: exact power-of-two scale of a
    // frozen binary64 constant, with no runtime transcendental function.
    return exposure_gain(ev);
}
void validate(const Stack& stack) {
    need(stack.size() <= PIXAURA_EVALUATION_MAX_OPERATIONS, 8);
    for (std::size_t i = 0; i < stack.size(); ++i) {
        const auto& op = stack[i];
        need(op.operation_version == 1 && op.parameter_version == 1, 5);
        if(op.type=="pixaura.exposure") {
            const auto* p = std::get_if<document::Exposure>(&op.parameters);
            need(p && p->milli_ev >= -5000 && p->milli_ev <= 5000, 7);
        } else if(op.type=="pixaura.crop") {
            const auto* p=std::get_if<document::Crop>(&op.parameters);need(p!=nullptr,7);
            geometry::crop({1,1},*p);
        } else if(op.type=="pixaura.rotate") {
            const auto* p=std::get_if<document::Rotate>(&op.parameters);need(p&&p->quarter_turns<=3,7);
        } else if(const auto* spec=tone::find(op.type)){const auto* p=std::get_if<document::Tone>(&op.parameters);need(p&&p->value>=spec->low&&p->value<=spec->high,7);}
        else if(const auto* detail_spec=detail::find(op.type)){const auto* p=std::get_if<document::Detail>(&op.parameters);need(p&&p->value>=detail_spec->low&&p->value<=detail_spec->high,7);}
        else need(false,5);
        for (std::size_t j = 0; j < i; ++j) need(op.id != stack[j].id, 6);
    }
}
Stack parse(std::string_view bytes) {
    auto parsed = document::parse_evaluation(bytes);
    if (parsed.code) throw document::Failure{parsed.code, parsed.index};
    validate(parsed.value);
    return std::move(parsed.value);
}
void checkpoint(const Cancellation* cancel, Checkpoint point, Observer observer, void* state) {
    if(observer) observer(point,state);
    need(!cancel || (!cancel->requested.load(std::memory_order_acquire) &&
        (!cancel->generation || (cancel->generation->load(std::memory_order_acquire)==cancel->expected &&
          !cancel->revoked->load(std::memory_order_acquire)))),13);
}
uint64_t reservation(uint32_t width,uint32_t height,const Stack& stack,const pixaura_working_limits& limits) {
    const auto plan=geometry::plan({width,height},stack,limits);
    need(plan.scratch_bytes<=1048576,8);
    need(plan.max_raster_bytes<=UINT64_MAX-plan.scratch_bytes,8);
    return plan.max_raster_bytes+plan.scratch_bytes;
}
working::Image evaluate(const working::Image& source, const Stack& stack, const pixaura_working_limits& limits,
    const Cancellation* cancel, Observer observer, void* state) {
    const auto check=[&](Checkpoint p){checkpoint(cancel,p,observer,state);};
    check(Checkpoint::admission);
    validate(stack);
    need(std::fegetround() == FE_TONEAREST, 7);
    const auto& m=source.metadata;
    need(m.api_version==1&&m.struct_size==sizeof(m)&&m.orientation==1&&m.pixel_format==1,17);
    const geometry::Extent extent{m.width,m.height};
    const auto bytes=geometry::admit(extent,limits);
    need(m.row_stride==decode::multiply(m.width,16)&&m.image_bytes==bytes&&source.pixels&&source.pixels->size()==bytes/4,19);
    const auto plan=geometry::plan(extent,stack,limits);
    check(Checkpoint::allocation);
    working::Image out;out.metadata=m;
    out.pixels=std::make_unique<decode::Vector<float>>(static_cast<std::size_t>(bytes/4));
    need(decode::multiply(out.pixels->capacity(),4)<=bytes,8);
    decode::Vector<uint8_t> visited;
    if(plan.scratch_bytes){check(Checkpoint::allocation);visited.resize(static_cast<std::size_t>(plan.scratch_bytes));need(visited.capacity()<=1048576&&visited.capacity()<=plan.scratch_bytes,8);}
    auto& pixels = *out.pixels;
    const auto traverse=[&](geometry::Extent size,auto&& fn){
        for(uint32_t t=0;t<geometry::tile_count(size);++t) {
            check(Checkpoint::tile);const auto r=geometry::tile(size,t);
            for(uint32_t y=r.y0;y<r.y1;++y)for(uint32_t x=r.x0;x<r.x1;++x) {
                const auto i=geometry::pixel_offset(size,{x,y});
                fn(i);
            }
        }
    };
    traverse(extent,[&](std::size_t i) {
        std::memcpy(pixels.data()+i,source.pixels->data()+i,4*sizeof(float));
        const float a = pixels[i + 3];
        need(finite(a) && a >= 0 && a <= 1, 7);
        for (unsigned c = 0; c < 3; ++c) {
            need(finite(pixels[i + c]), 7);
            need(a != 0 || pixels[i + c] == 0, 7);
        }
    });
    for (std::size_t stage_index=0;stage_index<stack.size();++stage_index) {
        const auto& op=stack[stage_index];const auto& stage=plan.stages[stage_index];
        check(Checkpoint::admission);
        if(op.type=="pixaura.crop") {
            // Increasing destination offsets are required for safe compaction:
            // each source offset >= its destination; future source rows remain unread.
            for(uint32_t y=0;y<stage.output.height;++y) {
                check(Checkpoint::tile);
                const auto dst=geometry::pixel_offset(stage.output,{0,y});
                const auto src=geometry::pixel_offset(stage.input,geometry::crop_source(stage,{0,y}));
                const auto row=decode::multiply(stage.output.width,16);
                need(dst<=src&&row<=SIZE_MAX,8);
                std::memmove(pixels.data()+dst,pixels.data()+src,static_cast<std::size_t>(row));
            }
        } else if(op.type=="pixaura.rotate") {
            if(stage.turns) {
                check(Checkpoint::allocation);std::fill(visited.begin(),visited.end(),uint8_t{0});
                unsigned moves=0;
                traverse(stage.output,[&](std::size_t root_offset){
                    const auto root=root_offset/4;
                    if(visited[root/8]&(1u<<(root%8)))return;
                    uint8_t saved[16];std::memcpy(saved,pixels.data()+root_offset,sizeof(saved));
                    auto at=root;
                    for(;;) {
                        if(moves++%1024==0)check(Checkpoint::tile);
                        const geometry::Point destination{static_cast<uint32_t>(at%stage.output.width),static_cast<uint32_t>(at/stage.output.width)};
                        const auto source_point=geometry::inverse(stage.input,stage.turns,destination);
                        const auto destination_offset=geometry::pixel_offset(stage.output,destination);
                        const auto source_offset=geometry::pixel_offset(stage.input,source_point),next=source_offset/4;
                        visited[at/8]|=static_cast<uint8_t>(1u<<(at%8));
                        if(next==root){std::memcpy(pixels.data()+destination_offset,saved,sizeof(saved));break;}
                        need(!(visited[next/8]&(1u<<(next%8))),14);
                        std::memcpy(pixels.data()+destination_offset,pixels.data()+source_offset,sizeof(saved));at=next;
                    }
                });
            }
        } else if(detail::find(op.type)) {
            detail::apply(pixels,stage.input.width,stage.input.height,op.type=="pixaura.sharpen",std::get<document::Detail>(op.parameters).value,visited,[&](){check(Checkpoint::tile);});
        } else if(const auto* spec=tone::find(op.type)) {
            const tone::Kernel kernel(*spec,std::get<document::Tone>(op.parameters).value);
            if(kernel.neutral())continue;
            traverse(stage.input,[&](std::size_t i){kernel.apply(pixels.data()+i);});
        } else {
            const auto ev = std::get<document::Exposure>(op.parameters).milli_ev;
            if (ev == 0) continue; // Exact identity, including signed zero/subnormals.
            const double multiplier = gain(ev);
            traverse(stage.input,[&](std::size_t i) {
                for (unsigned c = 0; c < 3; ++c) {
                    const double value = static_cast<double>(pixels[i + c]) * multiplier;
                    need(std::isfinite(value) && std::abs(value) <= std::numeric_limits<float>::max(), 7);
                    pixels[i + c] = static_cast<float>(value);
                }
            });
        }
        out.metadata.width=stage.output.width;out.metadata.height=stage.output.height;
        out.metadata.row_stride=decode::multiply(stage.output.width,16);
        out.metadata.image_bytes=decode::multiply(out.metadata.row_stride,stage.output.height);
        pixels.resize(static_cast<std::size_t>(out.metadata.image_bytes/4));
    }
    check(Checkpoint::publication);
    return out;
}
}
