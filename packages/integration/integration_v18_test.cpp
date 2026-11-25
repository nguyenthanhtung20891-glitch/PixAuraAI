#include "stage_plan_v2.hpp"
#include "stream_writer_v2.hpp"
#include "revision_projection_v2.hpp"
#include "preview_v2.hpp"
#include "runtime_budget_v2.hpp"
#include "../core/src/storage.hpp"
#include <filesystem>
#include <chrono>
#include "../core/src/preview.hpp"
#include "../core/src/srgb_table.hpp"
#include <zlib.h>
#include "../core/src/decode_codec.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iterator>
using namespace pixaura;
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"integration check %d\n",__LINE__);std::exit(1);}}while(false)
static std::vector<uint8_t> png(uint32_t w,uint32_t h){
    std::vector<uint8_t> rgba(std::size_t(w)*h*4);
    for(uint32_t y=0;y<h;++y)for(uint32_t x=0;x<w;++x){const auto i=(std::size_t(y)*w+x)*4;rgba[i]=uint8_t(x%256);rgba[i+1]=uint8_t(y%256);rgba[i+2]=uint8_t((x+y)%256);rgba[i+3]=uint8_t((x+y)%5?255:0);}
    std::vector<uint8_t> encoded(std::size_t(w)*h*5+65536);size_t written=0;
    CHECK(!pixaura_codec_encode_png(rgba.data(),w,h,encoded.data(),encoded.size(),&written,32u<<20));
    encoded.resize(written);encoded.shrink_to_fit();return encoded;
}
static std::vector<uint8_t> chunk(std::vector<uint8_t> input,const char* type,const std::vector<uint8_t>& payload){
    std::vector<uint8_t> record(8+payload.size()+4);const auto n=payload.size();
    for(unsigned i=0;i<4;++i)record[i]=uint8_t(n>>(24-i*8));
    std::memcpy(record.data()+4,type,4);std::memcpy(record.data()+8,payload.data(),n);
    const auto crc=crc32(0,record.data()+4,static_cast<uInt>(n+4));
    for(unsigned i=0;i<4;++i)record[8+n+i]=uint8_t(crc>>(24-i*8));
    input.insert(input.begin()+33,record.begin(),record.end());return input;
}
struct Bytes:integration::StagedSinkV2{
    std::vector<uint8_t> bytes;bool sealed=false,aborted=false;bool fail=false;
    void write(const uint8_t* data,std::size_t count)override{if(fail)throw document::Failure{12};bytes.insert(bytes.end(),data,data+count);}
    void seal()override{sealed=true;}void abort()noexcept override{aborted=true;bytes.clear();}
};
struct FileBytes:integration::StagedSinkV2{
    std::string path;std::ofstream file;bool sealed=false,aborted=false;
    explicit FileBytes(std::string name):path(std::move(name)),file(path,std::ios::binary|std::ios::trunc){CHECK(file);}
    void write(const uint8_t* data,std::size_t count)override{file.write(reinterpret_cast<const char*>(data),static_cast<std::streamsize>(count));if(!file)throw document::Failure{12};}
    void seal()override{file.flush();if(!file)throw document::Failure{12};file.close();if(!file)throw document::Failure{12};sealed=true;}
    void abort()noexcept override{if(file.is_open())file.close();std::error_code error;std::filesystem::remove(path,error);aborted=true;}
};
static std::string command(const document::ImageDocument& doc,const char* kind,const std::string& extra=""){
    return "{\"command_version\":1,\"kind\":\""+std::string(kind)+"\",\"expected_revision_id\":\""+doc.current().text()+"\",\"expected_session_id\":\""+doc.session().id.text()+"\",\"expected_generation\":"+std::to_string(doc.session().generation)+extra+"}";
}
struct Input:storage::Reader{
    const std::vector<uint8_t>& data;std::size_t offset=0;
    explicit Input(const std::vector<uint8_t>& bytes):data(bytes){}
    std::size_t read(uint8_t* output,std::size_t count)override{count=std::min(count,data.size()-offset);std::memcpy(output,data.data()+offset,count);offset+=count;return count;}
};
static std::string operation(const char* body,int id){char s[1024];std::snprintf(s,sizeof s,"{\"id\":\"%032x\",\"operation_version\":1,\"parameter_version\":1,%s}",id,body);return s;}
static int independent_oracle(){
    std::ifstream input("v18-real12mp-2.png",std::ios::binary|std::ios::ate);CHECK(input);const auto size=input.tellg();CHECK(size>0);input.seekg(0);
    std::vector<uint8_t> encoded(static_cast<std::size_t>(size));CHECK(input.read(reinterpret_cast<char*>(encoded.data()),size));
    tiled::Budget budget(134217728,encoded.capacity());
    auto decoded_lease=budget.reserve(48000000+(32ULL<<20)+65536);
    auto admitted=decode::admit(encoded.data(),encoded.size(),decode::defaults());
    CHECK(admitted.value.width==4000&&admitted.value.height==3000&&admitted.value.orientation==1);
    auto encoded_copy_lease=budget.reserve(encoded.size());
    decode::Source output(decode::Vector<uint8_t>(encoded.begin(),encoded.end()),std::move(admitted));
    CHECK(output.encoded().capacity()<=encoded.size());
    auto decoded=decode::execute(output,decode::defaults());
    CHECK(decoded->size()==48000000&&decoded->capacity()<=48000000);auto oracle_lease=budget.reserve(1048576);
    const auto stack=evaluation::parse("{\"operations\":["+operation("\"type\":\"pixaura.exposure\",\"parameters\":{\"milli_ev\":1000}",11)+","+operation("\"type\":\"pixaura.blur\",\"parameters\":{\"milli_strength\":250}",12)+"]}");
    decode::Metadata metadata;auto& m=metadata.value;m.api_version=1;m.struct_size=sizeof(m);m.width=m.height=m.display_width=m.display_height=130;m.orientation=1;m.pixel_format=1;m.bit_depth=8;m.channels=4;m.has_alpha=1;m.row_stride=520;m.decoded_bytes=67600;m.frame_count=1;
    uint64_t compared=0;std::vector<uint8_t> source(67600);
    for(uint32_t y=0;y<3000;y+=128)for(uint32_t x=0;x<4000;x+=128){
        for(uint32_t row=0;row<130;++row)for(uint32_t col=0;col<130;++col){
            const auto xx=static_cast<uint32_t>(std::clamp<int64_t>(int64_t(x)+col-1,0,3999));const auto yy=static_cast<uint32_t>(std::clamp<int64_t>(int64_t(y)+row-1,0,2999));
            auto* p=source.data()+(std::size_t(row)*130+col)*4;p[0]=uint8_t(xx%256);p[1]=uint8_t(yy%256);p[2]=uint8_t((xx+yy)%256);p[3]=uint8_t((xx+yy)%5?255:0);
        }
        // Independent fixed-neighborhood whole-raster v1 oracle. This never calls
        // v2 stage/region planning, and remains below every frozen v1 raster cap.
        auto original=working::normalize(metadata,source.data(),source.size(),working::defaults());
        auto edited=evaluation::evaluate(original,stack,working::defaults());auto display=preview::render(edited);
        for(uint32_t row=0;row<std::min(128u,3000-y);++row)for(uint32_t col=0;col<std::min(128u,4000-x);++col){
            const auto* actual=decoded->data()+(std::size_t(y+row)*4000+x+col)*4;
            const auto* pixel=display.pixels->data()+(std::size_t(row+1)*130+col+1)*4;
            CHECK(!std::memcmp(actual,pixel,4));++compared;
        }
    }
    CHECK(compared==12000000);
    std::printf("real12MP decoded PNG independent v1 fixed-neighborhood oracle %llu/12000000 pixels bitwise PASS\n",static_cast<unsigned long long>(compared));return 0;
}
int main(int argc,char** argv){
    if(argc==2&&std::string(argv[1])=="--oracle-only"){
        try{return independent_oracle();}
        catch(const document::Failure& error){std::fprintf(stderr,"independent PNG oracle native failure code=%d\n",error.code);return 1;}
    }
    const bool real_only=argc==2&&std::string(argv[1])=="--real-only";
    if(!real_only){
    auto encoded=png(259,131);tiled::Budget budget(134217728,encoded.capacity());
    {
        integration::OriginalReaderV2 reader(encoded.data(),encoded.size(),budget);
        auto m=decode::admit(encoded.data(),encoded.size(),decode::defaults());decode::Source original(decode::Vector<uint8_t>(encoded.begin(),encoded.end()),std::move(m));
        auto rgba=decode::execute(original,decode::defaults());auto reference=working::normalize(original.metadata(),rgba->data(),rgba->size(),working::defaults());
        for(uint32_t y=0;y<reader.height();++y)for(uint32_t x=0;x<reader.width();++x){float pixel[4];reader.read(x,y,pixel);CHECK(!std::memcmp(pixel,reference.pixels->data()+(std::size_t(y)*reader.width()+x)*4,16));}
        const auto crop=operation("\"type\":\"pixaura.crop\",\"parameters\":{\"x_ppm\":12345,\"y_ppm\":17321,\"width_ppm\":943210,\"height_ppm\":912345}",1);
        const auto rotate=operation("\"type\":\"pixaura.rotate\",\"parameters\":{\"quarter_turns\":1}",2);
        const auto rotate2=operation("\"type\":\"pixaura.rotate\",\"parameters\":{\"quarter_turns\":2}",5);
        const auto rotate3=operation("\"type\":\"pixaura.rotate\",\"parameters\":{\"quarter_turns\":3}",6);
        const auto blur=operation("\"type\":\"pixaura.blur\",\"parameters\":{\"milli_strength\":750}",3);
        const auto sharpen=operation("\"type\":\"pixaura.sharpen\",\"parameters\":{\"milli_amount\":300}",4);
        unsigned cases=0;
        for(const auto& body:std::vector<std::string>{"",crop,rotate,rotate2,rotate3,crop+","+rotate2,crop+","+rotate3,blur,blur+","+crop+","+rotate+","+sharpen,rotate+","+sharpen+","+crop+","+blur}){
            auto stack=evaluation::parse("{\"operations\":["+body+"]}");
            auto expected=evaluation::evaluate(reference,stack,working::defaults());
            integration::StagePlanV2 plan(reader,stack,budget);CHECK(plan.extent().width==expected.metadata.width&&plan.extent().height==expected.metadata.height);
            for(uint32_t y=0;y<plan.extent().height;y+=128)for(uint32_t x=0;x<plan.extent().width;x+=128){
                const geometry::Rect region{x,y,std::min(x+128,plan.extent().width),std::min(y+128,plan.extent().height)};
                auto actual=plan.render(region,budget);
                for(uint32_t row=0;row<actual.image.metadata.height;++row){
                    CHECK(!std::memcmp(actual.image.pixels->data()+std::size_t(row)*actual.image.metadata.width*4,expected.pixels->data()+(std::size_t(y+row)*expected.metadata.width+x)*4,actual.image.metadata.width*16));
                }
            }
            ++cases;
        }
        std::printf("v2 real PNG source +mixed geometry/local bitwise parity %u PASS\n",cases);
    }
    CHECK(budget.inflight()==0);
    for(unsigned orientation=1;orientation<=8;++orientation){
        std::vector<uint8_t> exif={'I','I',42,0,8,0,0,0,1,0,0x12,1,3,0,1,0,0,0,uint8_t(orientation),0,0,0,0,0,0,0};
        auto oriented=chunk(png(7,11),"eXIf",exif);tiled::Budget b(134217728,oriented.capacity());
        integration::OriginalReaderV2 reader(oriented.data(),oriented.size(),b);
        auto meta=decode::admit(oriented.data(),oriented.size(),decode::defaults());decode::Source source(decode::Vector<uint8_t>(oriented.begin(),oriented.end()),std::move(meta));
        auto pixels=decode::execute(source,decode::defaults());auto expected=working::normalize(source.metadata(),pixels->data(),pixels->size(),working::defaults());
        integration::StagePlanV2 plan(reader,{},b);auto actual=plan.render({0,0,reader.width(),reader.height()},b);
        CHECK(actual.image.metadata.width==expected.metadata.width&&actual.image.metadata.height==expected.metadata.height);
        CHECK(!std::memcmp(actual.image.pixels->data(),expected.pixels->data(),expected.metadata.image_bytes));
        auto preview=preview::render(expected);
        const pixaura_preview_request fit{1,sizeof(pixaura_preview_request),PIXAURA_PREVIEW_FIT,3,5,0};
        auto expected_fit=preview::render(expected,fit);auto actual_fit=integration::fit_preview_v2(plan,3,5,b);
        CHECK(actual_fit.width==expected_fit.metadata.width&&actual_fit.height==expected_fit.metadata.height);
        CHECK(actual_fit.pixels.size()==expected_fit.pixels->size()&&!std::memcmp(actual_fit.pixels.data(),expected_fit.pixels->data(),actual_fit.pixels.size()));
        for(uint32_t format:{1u,2u}){
            std::vector<uint8_t> encoded_reference(65536);size_t written=0;
            const auto code=format==1?pixaura_codec_encode_jpeg(preview.pixels->data(),reader.width(),reader.height(),93,encoded_reference.data(),encoded_reference.size(),&written,32u<<20):pixaura_codec_encode_png(preview.pixels->data(),reader.width(),reader.height(),encoded_reference.data(),encoded_reference.size(),&written,32u<<20);
            CHECK(!code);encoded_reference.resize(written);Bytes bytes;
            CHECK(integration::stream_export_v2(plan,format,93,b,bytes)==written&&bytes.sealed&&!bytes.aborted&&bytes.bytes==encoded_reference);
            if(format==2){Bytes png_zero;CHECK(integration::stream_export_v2(plan,format,0,b,png_zero)==written&&png_zero.sealed&&!png_zero.aborted&&png_zero.bytes==encoded_reference);}
            if(format==1){
                integration::OriginalReaderV2 jpeg(bytes.bytes.data(),bytes.bytes.size(),b);
                auto admitted=decode::admit(bytes.bytes.data(),bytes.bytes.size(),decode::defaults());decode::Source original(decode::Vector<uint8_t>(bytes.bytes.begin(),bytes.bytes.end()),std::move(admitted));
                auto decoded=decode::execute(original,decode::defaults());auto normalized=working::normalize(original.metadata(),decoded->data(),decoded->size(),working::defaults());
                for(uint32_t y=0;y<jpeg.height();++y)for(uint32_t x=0;x<jpeg.width();++x){float value[4];jpeg.read(x,y,value);CHECK(!std::memcmp(value,normalized.pixels->data()+(std::size_t(y)*jpeg.width()+x)*4,16));}
            }
        }
    }
    {
        auto unsupported=chunk(png(7,11),"gAMA",{0,0,0xb1,0x8f});tiled::Budget b(134217728,unsupported.capacity());
        try{integration::OriginalReaderV2 reader(unsupported.data(),unsupported.size(),b);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==16);}
        CHECK(b.inflight()==0);
    }
    {
        auto explicit_srgb=chunk(png(7,11),"sRGB",{0});tiled::Budget b(134217728,explicit_srgb.capacity());
        integration::OriginalReaderV2 reader(explicit_srgb.data(),explicit_srgb.size(),b);CHECK(reader.metadata().value.profile_type==2);
        evaluation::Cancellation cancelled;cancelled.requested=true;
        try{integration::OriginalReaderV2 denied(explicit_srgb.data(),explicit_srgb.size(),b,&cancelled);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==13);}
        integration::StagePlanV2 plan(reader,{},b);const auto held=b.inflight();
        std::atomic<uint64_t> generation{1};evaluation::Cancellation malformed;malformed.generation=&generation;
        Bytes target;try{integration::stream_export_v2(plan,2,93,b,target,&malformed);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==1&&target.aborted);}
        CHECK(b.inflight()==held);
        try{reader.read(7,0,nullptr);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==7);}
    }
    return 0;
    }
    auto large=png(4000,3000);
    pixaura_capability_profile profile{};CHECK(!pixaura_capability_baseline(1,&profile));
    integration::RuntimeBudgetV2 runtime(profile,large.capacity());auto& mobile=runtime.owner();
    CHECK(runtime.policy().runtime_cap_bytes<=134217728);
    {auto severe=profile;severe.runtime.memory_pressure=PIXAURA_PRESSURE_SEVERE;
        integration::RuntimeBudgetV2 pressure(severe,large.capacity());CHECK(pressure.policy().runtime_cap_bytes<=67108864);
        try{integration::OriginalReaderV2 denied(large.data(),large.size(),pressure.owner());CHECK(false);}catch(const document::Failure& e){CHECK(e.code==8);}
        CHECK(pressure.owner().inflight()==0);
    }
    {
        integration::OriginalReaderV2 reader(large.data(),large.size(),mobile);CHECK(reader.width()==4000&&reader.height()==3000);
        // Actual immutable managed source + ordinary Schema3 native history.
        // Existing storage owner is used unchanged, never an invented history class.
        const auto root=(std::filesystem::current_path()/("v18-project-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))).string();
        CHECK(std::filesystem::create_directory(root));
        {storage::AssetStore assets(root);Input input(large);const auto asset=assets.ingest(input,large.size(),reader.digest());CHECK(asset.digest==reader.digest());}
        const std::string manifest="{\"schema_version\":1,\"document_id\":\"00000000000000000000000000000002\",\"project_id\":\"00000000000000000000000000000001\",\"current_revision_id\":\"00000000000000000000000000000100\",\"source\":{\"sha256\":\""+reader.digest()+"\",\"byte_length\":"+std::to_string(large.size())+",\"metadata\":{\"width\":4000,\"height\":3000,\"orientation\":1,\"codec\":\"png\",\"has_alpha\":true,\"icc_sha256\":null}},\"operations\":[],\"revisions\":[{\"id\":\"00000000000000000000000000000100\",\"parent_id\":null,\"actor\":\"import\",\"plan_id\":null,\"stack\":[]}],\"redo\":[]}";
        auto initial=document::create_document(manifest,"00000000000000000000000000000500");CHECK(!initial.code);
        document::Snapshot committed;
        {
            storage::Repository repository(root);repository.create(*initial.value);repository.migrate(1,2);repository.migrate(2,3);CHECK(repository.version()==3);
            auto prior=repository.read();
            const auto edit=operation("\"type\":\"pixaura.exposure\",\"parameters\":{\"milli_ev\":1000}",11)+","+operation("\"type\":\"pixaura.blur\",\"parameters\":{\"milli_strength\":250}",12);
            const auto apply=command(*prior.snapshot,"commit",",\"revision_id\":\"00000000000000000000000000000101\",\"actor\":\"manual\",\"plan_id\":null,\"operations\":["+edit+"],\"stack\":[\"0000000000000000000000000000000b\",\"0000000000000000000000000000000c\"]");
            auto candidate=document::transition(*prior.snapshot,apply);CHECK(!candidate.code);
            integration::RevisionProjectionV2 preview_only(candidate.value,reader,mobile);auto tile=preview_only.plan().render({0,0,128,128},mobile);
            CHECK(repository.read().snapshot->revisions().size()==1); // preview is never Apply
            repository.apply(apply,prior.epoch);auto saved=repository.read();CHECK(saved.snapshot->revisions().size()==2);
            repository.apply(command(*saved.snapshot,"undo"),saved.epoch);auto undone=repository.read();CHECK(undone.snapshot->current()==prior.snapshot->current());
            repository.apply(command(*undone.snapshot,"redo"),undone.epoch);CHECK(repository.read().snapshot->current()==saved.snapshot->current());
        }
        {
            storage::Repository reopened(root);auto acquired=reopened.open_session("00000000000000000000000000000501");committed=acquired.snapshot;
            CHECK(committed->revisions().size()==2&&committed->session().id.text()=="00000000000000000000000000000501");
        }
        integration::RevisionProjectionV2 projection(committed,reader,mobile);const auto& plan=projection.plan();uint64_t pixels=0;
        std::printf("real12MP native ordinary history Apply/Undo/Redo/reopen PASS project%s revision%s\n",root.c_str(),committed->current().text().c_str());
        pixels=uint64_t(plan.extent().width)*plan.extent().height;
        CHECK(pixels==12000000&&mobile.peak()<=134217728);
        for(uint32_t format:{1u,2u}){
            FileBytes bytes("v18-real12mp-"+std::to_string(format)+(format==1?".jpg":".png"));
            const auto written=integration::stream_export_v2(plan,format,93,mobile,bytes);
            CHECK(bytes.sealed&&!bytes.aborted);
            auto readback_lease=mobile.reserve(written+65536);
            std::vector<uint8_t> raw(static_cast<std::size_t>(written));CHECK(raw.capacity()==written);
            std::ifstream input(bytes.path,std::ios::binary);CHECK(input.read(reinterpret_cast<char*>(raw.data()),static_cast<std::streamsize>(written)));
            const auto admitted=decode::admit(raw.data(),raw.size(),decode::defaults());
            CHECK(admitted.value.width==4000&&admitted.value.height==3000);
            storage::Sha256 hash;hash.update(raw.data(),raw.size());
            std::printf("real12MP stream format%u bytes%llu SHA256%s PASS\n",format,static_cast<unsigned long long>(written),hash.finish().c_str());
        }
        {auto display=integration::fit_preview_v2(plan,64,48,mobile);CHECK(display.width==64&&display.height==48&&display.source.width==4000&&display.source.height==3000);}
        evaluation::Cancellation cancelled;cancelled.requested=true;Bytes stopped;
        try{integration::stream_export_v2(plan,2,93,mobile,stopped,&cancelled);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==13&&stopped.aborted&&!stopped.sealed);}
        Bytes failed;failed.fail=true;
        try{integration::stream_export_v2(plan,2,93,mobile,failed);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==12&&failed.aborted&&!failed.sealed);}

        std::printf("v2 actual4000x3000 PNG decode/normalized traversal pixels %llu reservation %llu; host native integration only, not app/device qualification\n",static_cast<unsigned long long>(pixels),static_cast<unsigned long long>(mobile.peak()));
    }
    CHECK(mobile.inflight()==0);
    for(uint32_t format:{1u,2u}){
        const auto path="v18-real12mp-"+std::to_string(format)+(format==1?".jpg":".png");
        std::ifstream input(path,std::ios::binary|std::ios::ate);CHECK(input);
        const auto size=input.tellg();CHECK(size>0);input.seekg(0);
        std::vector<uint8_t> bytes(static_cast<std::size_t>(size));CHECK(input.read(reinterpret_cast<char*>(bytes.data()),size));
        tiled::Budget readback(134217728,bytes.capacity()+large.capacity());
        integration::OriginalReaderV2 decoded(bytes.data(),bytes.size(),readback);
        CHECK(decoded.width()==4000&&decoded.height()==3000&&decoded.metadata().value.decoded_bytes==48000000);
        float last[4];decoded.read(3999,2999,last);CHECK(last[3]>=0&&last[3]<=1);
        std::printf("real12MP format%u full entropy decode48000000bytes PASS\n",format);
    }
    return 0;
}
