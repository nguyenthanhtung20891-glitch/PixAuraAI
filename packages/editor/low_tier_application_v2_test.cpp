#include "pixaura/editor_v2.h"
#include "../core/src/decode.hpp"
#include "../core/src/sha256.hpp"
#include "../core/src/decode_codec.h"
#include "../core/src/evaluation.hpp"
#include "../core/src/preview.hpp"
#include <filesystem>
#include <fstream>
#include <vector>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#define fd_open _open
#define fd_close _close
#else
#include <unistd.h>
#define fd_open open
#define fd_close close
#define O_BINARY 0
#endif
using namespace pixaura;
namespace fs=std::filesystem;
#define CHECK(x) do{if(!(x))throw std::runtime_error("LOW application line "+std::to_string(__LINE__));}while(false)
#define OK(x) do{const auto code=(x);if(code)throw std::runtime_error("LOW application status "+std::to_string(code)+" line "+std::to_string(__LINE__));}while(false)
static std::vector<uint8_t> read(const fs::path& path){std::ifstream file(path,std::ios::binary|std::ios::ate);CHECK(file);const auto length=file.tellg();CHECK(length>0);std::vector<uint8_t> bytes(static_cast<size_t>(length));file.seekg(0);CHECK(file.read(reinterpret_cast<char*>(bytes.data()),length));return bytes;}
static int verify(const char* original,const char* manifest){
    // Separate reference PROCESS. Its oracle buffers are not product/native
    // admission, device performance, or memory qualification observations.
    std::ifstream receipt(manifest);std::string path;CHECK(std::getline(receipt,path));const auto root=fs::u8path(path);
    const auto encoded=read(original);auto m=decode::admit(encoded.data(),encoded.size(),decode::defaults());
    decode::Source source(std::make_unique<decode::Vector<uint8_t>>(encoded.begin(),encoded.end()),std::move(m));const auto& metadata=source.metadata();
    auto original_pixels=decode::execute(source,decode::defaults());CHECK(metadata.value.width==4000&&metadata.value.height==3000);
    std::vector<uint8_t> expected(48000000);const auto operations=evaluation::parse("{\"operations\":[{\"id\":\"00000000000000000000000000000001\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}]}");
    for(uint32_t y=0;y<3000;y+=128)for(uint32_t x=0;x<4000;x+=128){
        const auto w=std::min(128u,4000-x),h=std::min(128u,3000-y);std::vector<uint8_t> raw(size_t(w)*h*4);
        for(uint32_t row=0;row<h;++row)std::memcpy(raw.data()+size_t(row)*w*4,original_pixels->data()+(size_t(y+row)*4000+x)*4,size_t(w)*4);
        auto local=metadata;local.value.width=local.value.display_width=w;local.value.height=local.value.display_height=h;local.value.row_stride=uint64_t(w)*4;local.value.decoded_bytes=uint64_t(w)*h*4;
        auto normalized=working::normalize(local,raw.data(),raw.size(),working::defaults());auto evaluated=evaluation::evaluate(normalized,operations,working::defaults());
        for(uint32_t row=0;row<h;++row)for(uint32_t col=0;col<w;++col){const auto* p=evaluated.pixels->data()+(size_t(row)*w+col)*4;auto* out=expected.data()+(size_t(y+row)*4000+x+col)*4;const double alpha=p[3];for(unsigned c=0;c<3;++c)out[c]=alpha==0?0:preview::encode(double(p[c])/alpha);out[3]=preview::alpha(alpha);}
    }
    for(unsigned format=1;format<=2;++format){std::vector<uint8_t> bytes(64u<<20);size_t count=0;
        CHECK((format==1?pixaura_codec_encode_jpeg(expected.data(),4000,3000,93,bytes.data(),bytes.size(),&count,32u<<20):pixaura_codec_encode_png(expected.data(),4000,3000,bytes.data(),bytes.size(),&count,32u<<20))==0);
        bytes.resize(count);const auto actual=read(root/(format==1?"full.jpg":"full.png"));CHECK(actual==bytes);
        storage::Sha256 hash;hash.update(actual.data(),actual.size());std::printf("LOW12MP independent frozen codec/kernel oracle format%u bytes%zu SHA256%s bitwise PASS\n",format,actual.size(),hash.finish().c_str());
    }
    return 0;
}
int main(int argc,char** argv){std::setvbuf(stdout,nullptr,_IONBF,0);const auto started=std::chrono::steady_clock::now();const auto stage=[&](const char* name){std::printf("LOW12MP elapsed_ms%lld %s\n",static_cast<long long>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-started).count()),name);};uint64_t runtime=0,editor=0;try{
    CHECK(argc==4);if(std::strcmp(argv[1],"--verify")==0)return verify(argv[2],argv[3]);CHECK(std::strcmp(argv[1],"--exercise")==0);
    const auto root=fs::current_path()/("low12mp-v2-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(fs::create_directory(root));fs::permissions(root,fs::perms::owner_all,fs::perm_options::replace);
    const auto project=root/"project";CHECK(fs::create_directory(project));fs::permissions(project,fs::perms::owner_all,fs::perm_options::replace);
    pixaura_capability_profile profile{};OK(pixaura_capability_baseline(1,&profile));profile.hardware.platform=PIXAURA_PLATFORM_APPLE;profile.hardware.architecture=PIXAURA_ARCH_ARM64;
    profile.hardware.logical_cpu_count=2;profile.hardware.physical_memory_bytes=UINT64_C(1073741824);profile.hardware.runtime_version=18;
    profile.hardware.provenance=PIXAURA_PROVENANCE_SYNTHETIC;profile.hardware.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;
    profile.runtime.thermal=PIXAURA_THERMAL_NORMAL;profile.runtime.memory_pressure=PIXAURA_PRESSURE_NORMAL;profile.runtime.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;profile.runtime.process_budget_bytes=UINT64_C(536870912);profile.runtime.sequence=1;
    pixaura_policy_observation observation{};observation.version=1;observation.struct_size=sizeof(observation);observation.thermal=PIXAURA_POLICY_NORMAL;observation.memory_pressure=PIXAURA_POLICY_NORMAL;observation.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;observation.process_budget_bytes=UINT64_C(536870912);observation.sequence=1;
    uint8_t synthetic[32];for(unsigned i=0;i<32;++i)synthetic[i]=static_cast<uint8_t>(i+1);pixaura_execution_strategy strategy{};const auto path=root.u8string(),document=project.u8string();
    OK(pixaura_v2_runtime_create_policy(2,path.c_str(),path.size(),synthetic,32,&profile,&observation,&strategy,&runtime));CHECK(strategy.runtime_cap_bytes==UINT64_C(67108864)&&strategy.forced_cpu&&strategy.backend==0);
    const auto input=fd_open(argv[2],O_RDONLY|O_BINARY);CHECK(input>=0);const auto imported=pixaura_v2_editor_open(2,runtime,document.c_str(),document.size(),input,fs::file_size(argv[2]),1,&editor);fd_close(input);CHECK(imported==0);stage("import complete");
    pixaura_v2_state state{};OK(pixaura_v2_editor_state(2,editor,&state));CHECK(state.revisions==1);const std::string original_revision=state.revision;
    pixaura_v2_preview before{};OK(pixaura_v2_editor_preview(2,editor,64,48,1,&before));CHECK(before.source_width==4000&&before.source_height==3000);OK(pixaura_v2_frame_release(2,before.frame));stage("original preview complete");
    OK(pixaura_v2_editor_select(2,editor,4));int32_t exposure=1000;OK(pixaura_v2_editor_update(2,editor,&exposure,1));
    uint32_t changed=0;OK(pixaura_v2_editor_apply(2,editor,&changed));CHECK(changed==1);OK(pixaura_v2_editor_state(2,editor,&state));CHECK(state.revisions==2);const std::string committed=state.revision;stage("HumanApply complete");
    pixaura_v2_commercial_state commercial{};OK(pixaura_v2_runtime_commercial_state(2,runtime,&commercial));CHECK(commercial.access==1&&commercial.trial_start>=0);
    pixaura_v2_preview after{};OK(pixaura_v2_editor_preview(2,editor,64,48,0,&after));CHECK(after.source_width==4000&&after.source_height==3000);OK(pixaura_v2_frame_release(2,after.frame));stage("edited preview complete");
    OK(pixaura_v2_editor_history(2,editor,0));OK(pixaura_v2_editor_state(2,editor,&state));CHECK(original_revision==state.revision);
    OK(pixaura_v2_editor_history(2,editor,1));OK(pixaura_v2_editor_state(2,editor,&state));CHECK(committed==state.revision);
    OK(pixaura_v2_editor_select(2,editor,4));exposure=-1000;OK(pixaura_v2_editor_update(2,editor,&exposure,1));OK(pixaura_v2_editor_cancel(2,editor));
    OK(pixaura_v2_editor_close(2,editor));editor=0;OK(pixaura_v2_editor_open(2,runtime,document.c_str(),document.size(),-1,0,0,&editor));
    OK(pixaura_v2_editor_state(2,editor,&state));CHECK(state.revisions==2&&committed==state.revision);stage("history/reopen complete");
    for(unsigned format=1;format<=2;++format){const auto output_path=(root/(format==1?"full.jpg":"full.png")).u8string();const auto output=fd_open(output_path.c_str(),O_RDWR|O_CREAT|O_EXCL|O_BINARY,0600);CHECK(output>=0);
        pixaura_v2_export_result exported{};const auto code=pixaura_v2_editor_export_observed(2,editor,format,93,output,&exported);fd_close(output);CHECK(code==0&&exported.width==4000&&exported.height==3000&&exported.bytes>0);CHECK(committed==exported.revision);
        stage(format==1?"JPEG complete":"PNG complete");std::printf("LOW12MP native full export format%u bytes%llu dimensions%ux%u cap%llu forcedCPU1 PASS\n",format,static_cast<unsigned long long>(exported.bytes),exported.width,exported.height,static_cast<unsigned long long>(strategy.runtime_cap_bytes));
    }
    OK(pixaura_v2_editor_close(2,editor));editor=0;OK(pixaura_v2_runtime_close(2,runtime));runtime=0;
    // Frozen critical policy stays32MiB: encoded high-entropy source alone
    // exceeds capacity. Denial must not create a document, receipt, or charge.
    observation.memory_pressure=PIXAURA_POLICY_CRITICAL;observation.sequence=2;
    OK(pixaura_v2_runtime_create_policy(2,path.c_str(),path.size(),synthetic,32,&profile,&observation,&strategy,&runtime));
    CHECK(strategy.runtime_cap_bytes==UINT64_C(33554432));
    pixaura_v2_commercial_state prior{},denied{};OK(pixaura_v2_runtime_commercial_state(2,runtime,&prior));
    const auto rejected_root=root/"resource-denied";CHECK(fs::create_directory(rejected_root));const auto rejected=rejected_root.u8string();
    const auto large_input=fd_open(argv[2],O_RDONLY|O_BINARY);CHECK(large_input>=0);uint64_t unchanged=123;
    const auto rejection=pixaura_v2_editor_open(2,runtime,rejected.c_str(),rejected.size(),large_input,fs::file_size(argv[2]),1,&unchanged);fd_close(large_input);
    CHECK(rejection==8&&unchanged==123);OK(pixaura_v2_runtime_commercial_state(2,runtime,&denied));
    CHECK(prior.photos==denied.photos&&prior.ai==denied.ai&&prior.trial_start==denied.trial_start);
    OK(pixaura_v2_runtime_close(2,runtime));runtime=0;
    std::puts("Critical32MiB high-entropy denial8: output sentinel and commercial usage/trial unchanged PASS");
    for(const auto& file:fs::directory_iterator(project))CHECK(file.path().filename().u8string().find(".pixaura-row-v2-")!=0);
    std::ofstream manifest(argv[3],std::ios::binary|std::ios::trunc);manifest<<root.u8string()<<'\n';CHECK(manifest.good());
    std::puts("LOW64MiB real high-entropy12MP shipping C bridge: import/BeforeAfter/manual HUMANApply/trial/Cancel/history/reopen/JPEG/PNG PASS; HOST SYNTHETIC, not physical qualification");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());if(editor)(void)pixaura_v2_editor_close(2,editor);if(runtime)(void)pixaura_v2_runtime_close(2,runtime);return 1;}catch(const document::Failure& e){std::fprintf(stderr,"LOW application native%d\n",e.code);return 1;}}
