#include "../src/decode.hpp"
#include "../src/working.hpp"
#include "../src/storage.hpp"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <filesystem>
#include <iostream>
#include <future>
#include <cstring>
#include <array>
#include <atomic>
#include <zlib.h>
using namespace pixaura;
namespace fs=std::filesystem;
static std::atomic<unsigned> checks{0};
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"decode check "<<__LINE__<<": "<<#x<<'\n';std::exit(1);}}while(false)
template<class F> void error(int code,F fn){try{fn();CHECK(false);}catch(const document::Failure& f){CHECK(f.code==code);}}
using Bytes=decode::Vector<uint8_t>;
Bytes png(){return {std::begin(decode_png),std::end(decode_png)};}
Bytes jpeg(){return {std::begin(decode_jpeg),std::end(decode_jpeg)};}
void be32(Bytes& b,std::size_t at,uint32_t n){for(unsigned i=0;i<4;++i)b[at+i]=uint8_t(n>>(24-8*i));}
void crc(Bytes& b,std::size_t at,std::size_t n){be32(b,at+8+n,uint32_t(crc32(0,b.data()+at+4,static_cast<uInt>(n+4))));}
Bytes tiff(unsigned o){return {'I','I',42,0,8,0,0,0,1,0,18,1,3,0,1,0,0,0,uint8_t(o),0,0,0,0,0,0,0};}
Bytes orient(Bytes b,unsigned o,bool is_png){const auto t=tiff(o);Bytes part;
    if(is_png){part.resize(t.size()+12);be32(part,0,uint32_t(t.size()));std::memcpy(part.data()+4,"eXIf",4);std::memcpy(part.data()+8,t.data(),t.size());crc(part,0,t.size());b.insert(b.begin()+33,part.begin(),part.end());}
    else{part={255,225,0,uint8_t(t.size()+8),'E','x','i','f',0,0};part.insert(part.end(),t.begin(),t.end());b.insert(b.begin()+2,part.begin(),part.end());}return b;
}
Bytes chunk(const char* type,const Bytes& payload){Bytes b(payload.size()+12);be32(b,0,uint32_t(payload.size()));std::memcpy(b.data()+4,type,4);std::copy(payload.begin(),payload.end(),b.begin()+8);crc(b,0,payload.size());return b;}
Bytes inject(Bytes b,const Bytes& part){b.insert(b.begin()+33,part.begin(),part.end());return b;}
Bytes icc(){Bytes b(132);be32(b,0,132);b[8]=4;std::memcpy(b.data()+16,"RGB ",4);std::memcpy(b.data()+20,"XYZ ",4);std::memcpy(b.data()+36,"acsp",4);return b;}
struct Reader:storage::Reader{const Bytes& b;std::size_t at=0;explicit Reader(const Bytes& input):b(input){}std::size_t read(uint8_t* out,std::size_t cap)override{const auto n=std::min(cap,b.size()-at);std::memcpy(out,b.data()+at,n);at+=n;return n;}};
struct Context {
    pixaura_decode_context c{};
    Context(char identity='a',pixaura_decode_limits l=decode::defaults()){const std::string id(32,identity);CHECK(pixaura_decode_context_init(1,&c,sizeof(c),reinterpret_cast<const uint8_t*>(id.data()),32,&l)==0);}
    ~Context(){CHECK(pixaura_decode_context_destroy(&c)==0);}
};
int main(int argc,char** argv){CHECK(argc==2);auto l=decode::defaults();
    for(const auto& input:{std::pair<const uint8_t*,std::size_t>{decode_gray1,sizeof(decode_gray1)},
        {decode_rgb,sizeof(decode_rgb)},{decode_grayalpha,sizeof(decode_grayalpha)},{decode_palette,sizeof(decode_palette)},{decode_adam7,sizeof(decode_adam7)}}){
        const Bytes encoded(input.first,input.first+input.second);auto info=decode::admit(encoded.data(),encoded.size(),l);decode::Source source(encoded,std::move(info));const auto pixels=decode::execute(source,l);CHECK(pixels->size()==source.metadata().value.decoded_bytes);
        if(input.first==decode_adam7){auto plain=png();auto pm=decode::admit(plain.data(),plain.size(),l);decode::Source ps(plain,std::move(pm));CHECK(*pixels==*decode::execute(ps,l));}
        if(input.first==decode_palette)CHECK((*pixels)[0]==0&&(*pixels)[1]==255&&(*pixels)[3]==128);
        if(input.first==decode_gray1)CHECK((*pixels)[0]==255&&(*pixels)[3]==255);
    }
    for(const auto& b:{png(),jpeg()}){auto m=decode::admit(b.data(),b.size(),l);std::cerr<<"format "<<m.value.format<<" orientations and failure sweep\n";CHECK(m.value.width==2&&m.value.height==3&&m.value.decoded_bytes==24);decode::Source s(b,std::move(m));const auto pixels=decode::execute(s,l);CHECK(pixels->size()==24);if(s.metadata().value.format==1)for(auto x:*pixels)CHECK(x==128||x==255);else CHECK((*pixels)[0]==255&&(*pixels)[23]==128);
        for(unsigned o=1;o<=8;++o){auto v=orient(b,o,s.metadata().value.format==2);auto info=decode::admit(v.data(),v.size(),l);CHECK(info.value.orientation==o&&info.value.display_width==(o>=5?3u:2u));decode::Source source(v,std::move(info));const auto decoded=decode::execute(source,l);CHECK(*decoded==*pixels);auto normalized=working::normalize(source.metadata(),decoded->data(),decoded->size(),working::defaults());CHECK(normalized.metadata.width==source.metadata().value.display_width&&normalized.metadata.height==source.metadata().value.display_height&&normalized.metadata.orientation==1);}
        auto bad=orient(b,9,s.metadata().value.format==2);error(17,[&]{decode::admit(bad.data(),bad.size(),l);});bad=orient(b,0,s.metadata().value.format==2);error(17,[&]{decode::admit(bad.data(),bad.size(),l);});
        for(int fault:{1,2}){decode::execution_fault=fault;error(fault==1?17:19,[&]{decode::execute(s,l);});}decode::execution_fault=0;
        unsigned failures=0;for(int n=0;n<100;++n){decode::allocation_fail_after=n;try{CHECK(*decode::execute(s,l)==*pixels);break;}catch(const document::Failure& f){CHECK(f.code==8);++failures;}}CHECK(failures>0);decode::allocation_fail_after=-1;CHECK(*decode::execute(s,l)==*pixels);auto tiny_scratch=l;tiny_scratch.scratch_bytes=1;error(8,[&]{decode::execute(s,tiny_scratch);});
        for(std::size_t n=0;n<b.size();++n){try{decode::admit(b.data(),n,l);CHECK(false);}catch(const document::Failure& f){CHECK(f.code==17||f.code==18||f.code==16);}}
    }
    std::cerr<<"arithmetic and corpus\n";error(8,[]{decode::multiply(UINT64_MAX,2);});CHECK(decode::multiply(0,UINT64_MAX)==0);
    for(const auto& dims:{std::pair<uint32_t,uint32_t>{0,1},{1,0},{UINT32_MAX,UINT32_MAX},{16384,16384}}){auto b=png();be32(b,16,dims.first);be32(b,20,dims.second);crc(b,8,13);error(dims.first==0||dims.second==0?17:8,[&]{decode::admit(b.data(),b.size(),l);});}
    auto b=png();for(unsigned which=0;which<6;++which){auto small=l;if(which==0)small.row_stride=7;if(which==1)small.decoded_bytes=23;if(which==2)small.pixel_count=5;if(which==3)small.metadata_bytes=12;if(which==4)small.marker_count=2;if(which==5)small.encoded_bytes=b.size()-1;error(8,[&]{decode::admit(b.data(),b.size(),small);});}
    b=png();b.push_back(0);error(17,[&]{decode::admit(b.data(),b.size(),l);});b=jpeg();b.push_back(0);error(17,[&]{decode::admit(b.data(),b.size(),l);});
    b=png();be32(b,8,UINT32_MAX);error(18,[&]{decode::admit(b.data(),b.size(),l);});
    b=png();b[29]^=1;error(17,[&]{decode::admit(b.data(),b.size(),l);});
    b=inject(png(),chunk("acTL",{0,0,0,2,0,0,0,0}));error(16,[&]{decode::admit(b.data(),b.size(),l);});
    b=inject(png(),chunk("ABCD",{}));error(16,[&]{decode::admit(b.data(),b.size(),l);});
    b=orient(orient(png(),1,true),2,true);error(17,[&]{decode::admit(b.data(),b.size(),l);});
    b=orient(orient(jpeg(),1,false),2,false);error(17,[&]{decode::admit(b.data(),b.size(),l);});
    auto malformed_tiff=tiff(1);malformed_tiff[10]=0;malformed_tiff[11]=0;b=inject(png(),chunk("eXIf",malformed_tiff));CHECK(decode::admit(b.data(),b.size(),l).value.orientation==1);
    malformed_tiff=tiff(1);malformed_tiff[14]=2;b=inject(png(),chunk("eXIf",malformed_tiff));error(17,[&]{decode::admit(b.data(),b.size(),l);});
    malformed_tiff=tiff(1);malformed_tiff[22]=8;b=inject(png(),chunk("eXIf",malformed_tiff));error(17,[&]{decode::admit(b.data(),b.size(),l);});
    Bytes payload={'P',0,0};const auto profile=icc();Bytes compressed={120,1,1,132,0,123,255};compressed.insert(compressed.end(),profile.begin(),profile.end());const auto end=compressed.size();compressed.resize(end+4);be32(compressed,end,uint32_t(adler32(1,profile.data(),static_cast<uInt>(profile.size()))));payload.insert(payload.end(),compressed.begin(),compressed.end());
    b=inject(png(),chunk("iCCP",payload));auto prof=decode::admit(b.data(),b.size(),l);CHECK(prof.profile==profile&&prof.value.profile_type==1&&prof.value.profile_bytes==132);decode::Source prof_source(b,std::move(prof));CHECK(decode::execute(prof_source,l)->size()==24);
    auto profile_limit=l;profile_limit.profile_bytes=131;error(8,[&]{decode::admit(b.data(),b.size(),profile_limit);});
    auto duplicate=inject(b,chunk("iCCP",payload));error(17,[&]{decode::admit(duplicate.data(),duplicate.size(),l);});payload.back()^=1;duplicate=inject(png(),chunk("iCCP",payload));error(17,[&]{decode::admit(duplicate.data(),duplicate.size(),l);});
    auto gray_profile=profile;std::memcpy(gray_profile.data()+16,"GRAY",4);Bytes jpeg_profile={255,226,0,uint8_t(gray_profile.size()+16),'I','C','C','_','P','R','O','F','I','L','E',0,1,1};jpeg_profile.insert(jpeg_profile.end(),gray_profile.begin(),gray_profile.end());auto jp=jpeg();jp.insert(jp.begin()+2,jpeg_profile.begin(),jpeg_profile.end());auto jm=decode::admit(jp.data(),jp.size(),l);CHECK(jm.profile==gray_profile);decode::Source jps(jp,std::move(jm));CHECK(decode::execute(jps,l)->size()==24);jp.insert(jp.begin()+2,jpeg_profile.begin(),jpeg_profile.end());error(17,[&]{decode::admit(jp.data(),jp.size(),l);});
    auto exif_limit=l;exif_limit.exif_bytes=25;auto ex=orient(png(),1,true);error(8,[&]{decode::admit(ex.data(),ex.size(),exif_limit);});
    auto depth=png();depth[24]=16;crc(depth,8,13);error(16,[&]{decode::admit(depth.data(),depth.size(),l);});
    auto prog=jpeg();for(std::size_t i=0;i+1<prog.size();++i)if(prog[i]==255&&prog[i+1]==192){prog[i+1]=194;break;}error(16,[&]{decode::admit(prog.data(),prog.size(),l);});
    // A structurally complete file with corrupted compressed payload fails execution.
    auto entropy=png();entropy[43]^=255;const auto idat_size=(uint32_t(entropy[33])<<24)|(uint32_t(entropy[34])<<16)|(uint32_t(entropy[35])<<8)|entropy[36];crc(entropy,33,idat_size);auto entmeta=decode::admit(entropy.data(),entropy.size(),l);decode::Source entsource(entropy,std::move(entmeta));error(17,[&]{decode::execute(entsource,l);});
    const uint8_t gif[]={'G','I','F','8','9','a'};error(16,[&]{decode::admit(gif,sizeof(gif),l);});
    // Fixed-seed mutation/random corpus: at most 4096 inputs, 256 bytes each.
    uint32_t rng=0x50495834;for(unsigned i=0;i<4096;++i){Bytes fuzz=i%2?png():jpeg();rng=rng*1664525+1013904223;fuzz[rng%fuzz.size()]^=uint8_t((rng>>16)|1);try{auto m=decode::admit(fuzz.data(),fuzz.size(),l);decode::Source s(std::move(fuzz),std::move(m));decode::execute(s,l);}catch(const document::Failure& f){CHECK(f.code==8||f.code==16||f.code==17||f.code==18||f.code==19);}CHECK(true);}
    for(unsigned i=0;i<1024;++i){Bytes fuzz(64);for(auto& x:fuzz){rng=rng*1664525+1013904223;x=uint8_t(rng>>24);}try{decode::admit(fuzz.data(),fuzz.size(),l);CHECK(false);}catch(const document::Failure& f){CHECK(f.code==16||f.code==17||f.code==18);}CHECK(decode::multiply(rng,4)==uint64_t(rng)*4);}
    const fs::path root=fs::absolute(fs::path(argv[1])/"decode-test-root");fs::create_directories(root);storage::AssetStore store(root.generic_string());b=png();Reader reader(b);const auto asset=store.ingest(reader,b.size(),{},"mislabelled.jpeg");
    auto open=[&](Context& c,pixaura_decode_handle& h){const auto path=root.generic_string();return pixaura_decode_open(&c.c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),asset.digest.size(),asset.bytes,&h);};
    Context c; pixaura_decode_handle source{},image{};CHECK(open(c,source)==0);pixaura_decode_metadata m{};CHECK(pixaura_decode_query(&c.c,&source,&m)==0&&m.format==2);CHECK(pixaura_decode_image(&c.c,&source,&image)==0);
    const auto wl=working::defaults();pixaura_decode_handle normalized{},evaluated{};
    CHECK(pixaura_working_normalize(&c.c,&image,&wl,&normalized)==0);
    pixaura_working_metadata wm{};CHECK(pixaura_working_query(&c.c,&normalized,&wm)==0&&wm.width==2&&wm.height==3&&wm.orientation==1&&wm.row_stride==32);
    std::array<float,24> wp{};CHECK(pixaura_working_copy(&c.c,&normalized,0,wp.data(),24)==0&&wp[0]==1&&wp[23]>0.5f);
    auto sentinel_working=normalized;auto tiny=wl;tiny.image_bytes=95;CHECK(pixaura_working_normalize(&c.c,&image,&tiny,&sentinel_working)==8&&std::memcmp(&normalized,&sentinel_working,sizeof(normalized))==0);
    CHECK(pixaura_working_normalize(&c.c,&source,&wl,&sentinel_working)==3&&std::memcmp(&normalized,&sentinel_working,sizeof(normalized))==0);
    CHECK(pixaura_working_identity(&c.c,&normalized,&wl,&evaluated)==0&&evaluated.serial!=normalized.serial);
    CHECK(pixaura_working_copy(&c.c,&normalized,24,wp.data(),1)==1&&wp[0]==1);
    auto normalize_job=[&]{for(unsigned i=0;i<16;++i){pixaura_decode_handle h{};CHECK(pixaura_working_normalize(&c.c,&image,&wl,&h)==0);pixaura_working_metadata info{};CHECK(pixaura_working_query(&c.c,&h,&info)==0);CHECK(pixaura_decode_release(&c.c,&h)==0);}};
    auto wn1=std::async(std::launch::async,normalize_job),wn2=std::async(std::launch::async,normalize_job);wn1.get();wn2.get();
    auto working_read=[&]{for(unsigned i=0;i<100;++i){std::array<float,24> p{};CHECK(pixaura_working_copy(&c.c,&normalized,0,p.data(),24)==0&&p==wp);}};
    auto wr1=std::async(std::launch::async,working_read),wr2=std::async(std::launch::async,working_read);wr1.get();wr2.get();
    CHECK(pixaura_decode_release(&c.c,&evaluated)==0);CHECK(pixaura_decode_release(&c.c,&normalized)==0);CHECK(pixaura_working_query(&c.c,&normalized,&wm)==3);
    for(int fault:{1,2}){decode::execution_fault=fault;auto untouched=image;CHECK(pixaura_decode_image(&c.c,&source,&untouched)==(fault==1?17:19));CHECK(std::memcmp(&untouched,&image,sizeof(image))==0);}decode::execution_fault=0;
    decode::allocation_fail_after=0;auto failed_handle=image;CHECK(pixaura_decode_image(&c.c,&source,&failed_handle)==8&&std::memcmp(&failed_handle,&image,sizeof(image))==0);decode::allocation_fail_after=-1;
    pixaura_decode_handle duplicate_image{};CHECK(pixaura_decode_image(&c.c,&source,&duplicate_image)==0&&duplicate_image.serial!=image.serial);CHECK(pixaura_decode_release(&c.c,&duplicate_image)==0);
    std::array<pixaura_decode_handle,62> handles{};for(auto& h:handles)CHECK(pixaura_decode_image(&c.c,&source,&h)==0);auto exhausted=image;CHECK(pixaura_decode_image(&c.c,&source,&exhausted)==8&&std::memcmp(&exhausted,&image,sizeof(image))==0);for(auto& h:handles)CHECK(pixaura_decode_release(&c.c,&h)==0);
    auto small_limits=l;small_limits.context_bytes=1024;Context bounded('e',small_limits);exhausted=image;CHECK(open(bounded,exhausted)==8&&std::memcmp(&exhausted,&image,sizeof(image))==0);
    std::array<uint8_t,24> out{};CHECK(pixaura_decode_copy_pixels(&c.c,&image,0,out.data(),out.size())==0&&out[0]==255);
    auto sentinel=image;CHECK(pixaura_decode_image(&c.c,&image,&sentinel)==3&&std::memcmp(&sentinel,&image,sizeof(image))==0);
    CHECK(pixaura_decode_release(&c.c,&source)==0);CHECK(pixaura_decode_query(&c.c,&source,&m)==3);CHECK(pixaura_decode_query(&c.c,&image,&m)==0);CHECK(pixaura_decode_release(&c.c,&source)==3);
    const auto jb=jpeg();Reader jr(jb);const auto ja=store.ingest(jr,jb.size());
    auto other=std::async(std::launch::async,[&]{Context x('b');pixaura_decode_handle s{},im{},wk{};const auto path=root.generic_string();CHECK(pixaura_decode_open(&x.c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(ja.digest.data()),64,ja.bytes,&s)==0);CHECK(pixaura_decode_image(&x.c,&s,&im)==0);CHECK(pixaura_working_normalize(&x.c,&im,&wl,&wk)==0);CHECK(pixaura_decode_query(&x.c,&image,&m)==3);});normalize_job();other.get();
    for(const auto& input:{orient(png(),6,true),orient(jpeg(),8,false),inject(png(),chunk("sRGB",{0})),inject(png(),chunk("gAMA",{0,0,177,143})),inject(inject(png(),chunk("sRGB",{0})),chunk("gAMA",{0,0,177,143})),inject(inject(png(),chunk("sRGB",{0})),chunk("gAMA",{0,0,195,80})),prof_source.encoded()}){
        Reader ir(input);const auto ia=store.ingest(ir,input.size());const auto path=root.generic_string();pixaura_decode_handle ih{},im{},wk{};
        CHECK(pixaura_decode_open(&c.c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(ia.digest.data()),64,ia.bytes,&ih)==0);CHECK(pixaura_decode_image(&c.c,&ih,&im)==0);
        const auto info=decode::admit(input.data(),input.size(),l);const bool supported=info.value.profile_type!=1&&info.srgb_compatible;
        CHECK(pixaura_working_normalize(&c.c,&im,&wl,&wk)==(supported?0:16));
        if(supported){CHECK(pixaura_working_query(&c.c,&wk,&wm)==0&&wm.width==info.value.display_width);CHECK(pixaura_decode_release(&c.c,&wk)==0);}else CHECK(wk.serial==0);
        CHECK(pixaura_decode_release(&c.c,&ih)==0);CHECK(pixaura_decode_release(&c.c,&im)==0);
    }
    auto query=[&]{for(unsigned i=0;i<100;++i){pixaura_decode_metadata info{};CHECK(pixaura_decode_query(&c.c,&image,&info)==0);}};auto a=std::async(std::launch::async,query),d=std::async(std::launch::async,query);a.get();d.get();
    pixaura_decode_handle racing{};CHECK(open(c,racing)==0);auto reader_job=std::async(std::launch::async,[&]{for(unsigned i=0;i<100;++i){pixaura_decode_metadata info{};const auto status=pixaura_decode_query(&c.c,&racing,&info);CHECK(status==0||status==3);}});CHECK(pixaura_decode_release(&c.c,&racing)==0);reader_job.get();
    // The owner joins every operation before context destruction (ADR 0009).
    CHECK(pixaura_working_normalize(&c.c,&image,&wl,&normalized)==0);CHECK(pixaura_decode_release(&c.c,&image)==0);CHECK(pixaura_decode_copy_pixels(&c.c,&image,0,out.data(),1)==3);
    CHECK(pixaura_working_copy(&c.c,&normalized,0,wp.data(),24)==0);CHECK(pixaura_decode_query(&c.c,&normalized,&m)==0&&m.width==2);CHECK(pixaura_decode_release(&c.c,&normalized)==0);store.verify(asset);
    std::cout<<"decode checks "<<checks<<" passed; fixed corpus 4096 mutations + 1024 random/property cases; failures 0\n";
}
