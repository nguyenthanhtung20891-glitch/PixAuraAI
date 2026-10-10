#include "geometry.hpp"
#include "tone.hpp"
#include <algorithm>
namespace pixaura::geometry {
namespace {
void need(bool ok,int32_t code=7) { if(!ok) throw document::Failure{code}; }
uint64_t add(uint64_t a,uint64_t b) { need(b<=UINT64_MAX-a,8); return a+b; }
void extent(Extent e) { need(e.width&&e.height); }
}
uint64_t admit(Extent e,const pixaura_working_limits& l) {
    working::validate(l);extent(e);
    const auto pixels=decode::multiply(e.width,e.height),row=decode::multiply(e.width,16);
    const auto bytes=decode::multiply(row,e.height);
    need(e.width<=l.width&&e.height<=l.height&&pixels<=l.pixel_count&&row<=l.row_stride&&bytes<=l.image_bytes&&bytes<=SIZE_MAX,8);
    return bytes;
}
Rect crop(Extent e,document::Crop c) {
    extent(e);constexpr uint64_t m=1000000;
    need(c.x_ppm<m&&c.y_ppm<m&&c.width_ppm&&c.height_ppm&&c.width_ppm<=m&&c.height_ppm<=m);
    const auto xe=add(c.x_ppm,c.width_ppm),ye=add(c.y_ppm,c.height_ppm);
    need(xe<=m&&ye<=m);
    const auto x0=decode::multiply(e.width,c.x_ppm)/m,y0=decode::multiply(e.height,c.y_ppm)/m;
    const auto x1=add(decode::multiply(e.width,xe),m-1)/m,y1=add(decode::multiply(e.height,ye),m-1)/m;
    need(x0<x1&&y0<y1&&x1<=e.width&&y1<=e.height);
    return {static_cast<uint32_t>(x0),static_cast<uint32_t>(y0),static_cast<uint32_t>(x1),static_cast<uint32_t>(y1)};
}
Extent rotated(Extent e,uint32_t t) { extent(e);need(t<=3);return t%2?Extent{e.height,e.width}:e; }
Point forward(Extent e,uint32_t t,Point p) {
    rotated(e,t);need(p.x<e.width&&p.y<e.height);
    switch(t) {case 1:return {e.height-1-p.y,p.x};case 2:return {e.width-1-p.x,e.height-1-p.y};case 3:return {p.y,e.width-1-p.x};default:return p;}
}
Point inverse(Extent e,uint32_t t,Point p) {const auto d=rotated(e,t);return forward(d,(4-t)%4,p);}
uint32_t tile_count(Extent e) {
    admit(e,working::defaults());
    const auto cols=(uint64_t(e.width)+127)/128,rows=(uint64_t(e.height)+127)/128;
    const auto count=decode::multiply(cols,rows);need(count<=PIXAURA_MAX_TILES,8);return static_cast<uint32_t>(count);
}
Rect tile(Extent e,uint32_t index) {
    need(index<tile_count(e));const uint64_t cols=(uint64_t(e.width)+127)/128;
    const auto x=decode::multiply(index%cols,128),y=decode::multiply(index/cols,128);
    return {static_cast<uint32_t>(x),static_cast<uint32_t>(y),static_cast<uint32_t>(std::min(add(x,128),uint64_t(e.width))),static_cast<uint32_t>(std::min(add(y,128),uint64_t(e.height)))};
}
std::size_t pixel_offset(Extent e,Point p) {
    extent(e);need(p.x<e.width&&p.y<e.height);
    const auto index=decode::multiply(add(decode::multiply(p.y,e.width),p.x),4);
    need(index<=SIZE_MAX-3,8);return static_cast<std::size_t>(index);
}
Point crop_source(const Stage& stage,Point p) {
    need(p.x<stage.output.width&&p.y<stage.output.height);
    const auto x=add(stage.region.x0,p.x),y=add(stage.region.y0,p.y);
    need(x<stage.input.width&&y<stage.input.height);
    return {static_cast<uint32_t>(x),static_cast<uint32_t>(y)};
}
Plan plan(Extent e,const evaluation::Stack& stack,const pixaura_working_limits& l) {
    const auto initial_bytes=admit(e,l);need(stack.size()<=256,8);Plan out;
    out.max_raster_bytes=initial_bytes;out.max_pixels=decode::multiply(e.width,e.height);
    out.max_width=e.width;out.max_height=e.height;
    out.stages.reserve(stack.size());need(out.stages.capacity()<=256,8);uint64_t visits=0;
    for(std::size_t i=0;i<stack.size();++i) {
        const auto& op=stack[i];need(op.operation_version==1&&op.parameter_version==1,5);
        for(std::size_t j=0;j<i;++j)need(op.id!=stack[j].id,6);
        Stage stage{e,e,{0,0,e.width,e.height},0};
        if(op.type=="pixaura.crop") {const auto* c=std::get_if<document::Crop>(&op.parameters);need(c!=nullptr);stage.region=crop(e,*c);stage.output={stage.region.x1-stage.region.x0,stage.region.y1-stage.region.y0};}
        else if(op.type=="pixaura.rotate") {const auto* r=std::get_if<document::Rotate>(&op.parameters);need(r!=nullptr);stage.turns=r->quarter_turns;stage.output=rotated(e,r->quarter_turns);
            if(stage.turns)out.scratch_bytes=std::max(out.scratch_bytes,add(decode::multiply(e.width,e.height),7)/8);}
        else if(op.type=="pixaura.exposure") {const auto* p=std::get_if<document::Exposure>(&op.parameters);need(p&&p->milli_ev>=-5000&&p->milli_ev<=5000);}
        else if(const auto* spec=tone::find(op.type)){const auto* p=std::get_if<document::Tone>(&op.parameters);need(p&&p->value>=spec->low&&p->value<=spec->high);}
        else need(false,5);
        out.max_raster_bytes=std::max(out.max_raster_bytes,admit(stage.output,l));
        tile_count(stage.output);
        out.max_width=std::max(out.max_width,stage.output.width);out.max_height=std::max(out.max_height,stage.output.height);
        out.max_pixels=std::max(out.max_pixels,decode::multiply(stage.output.width,stage.output.height));
        visits=add(visits,decode::multiply(e.width,e.height));need(visits<=2147483648ULL,8);
        out.stages.push_back(stage);e=stage.output;
    }
    out.summary={1,sizeof(pixaura_geometry_plan),e.width,e.height,static_cast<uint32_t>(stack.size()),tile_count(e),128,1u,admit(e,l),decode::multiply(out.stages.capacity(),sizeof(Stage)),visits};
    return out;
}
}
