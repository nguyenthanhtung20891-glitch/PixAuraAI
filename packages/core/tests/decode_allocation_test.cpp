#include "pixaura/decode.h"
#include "pixaura/working.h"
#include "pixaura/evaluation.h"
#include "pixaura/geometry.h"
#include "pixaura/preview.h"
#include "../src/storage.hpp"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <filesystem>
static thread_local int fail_after=-1;
void* operator new(std::size_t n){if(fail_after==0)throw std::bad_alloc();if(fail_after>0)--fail_after;if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}
void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"decode allocation check %d\n",__LINE__);std::_Exit(1);}}while(false)
struct Reader:pixaura::storage::Reader{std::size_t at=0;std::size_t read(uint8_t* p,std::size_t n)override{const auto bytes=std::min(n,sizeof(decode_png)-at);std::memcpy(p,decode_png+at,bytes);at+=bytes;return bytes;}};
int main(int argc,char** argv){
    CHECK(argc==2);const auto root=std::filesystem::absolute(std::filesystem::path(argv[1])/"decode-allocation-root");std::filesystem::create_directories(root);
    const auto path=root.generic_string();pixaura::storage::AssetStore store(path);Reader reader;const auto asset=store.ingest(reader,sizeof(decode_png));
    pixaura_decode_limits l{};CHECK(pixaura_decode_default_limits(1,&l)==0);const uint8_t id[]="dddddddddddddddddddddddddddddddd";unsigned failures=0;
    const uint8_t request[]="{\"operations\":[{\"id\":\"00000000000000000000000000000001\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}]}";
    const uint8_t geometry_request[]="{\"operations\":[{\"id\":\"00000000000000000000000000000002\",\"type\":\"pixaura.rotate\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":1}},{\"id\":\"00000000000000000000000000000003\",\"type\":\"pixaura.crop\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"x_ppm\":0,\"y_ppm\":0,\"width_ppm\":500000,\"height_ppm\":1000000}}]}";
    const uint8_t tone_request[]="{\"operations\":[{\"id\":\"00000000000000000000000000000004\",\"type\":\"pixaura.brightness\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_linear\":500}},{\"id\":\"00000000000000000000000000000005\",\"type\":\"pixaura.contrast\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_stops\":500}},{\"id\":\"00000000000000000000000000000006\",\"type\":\"pixaura.temperature\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"kelvin\":4000}}]}";
    for(int phase=0;phase<19;++phase){bool success=false;
        for(int n=0;n<512;++n){pixaura_decode_context c{};pixaura_decode_handle source{},image{},working{},out{},sentinel;std::memset(&out,0x5a,sizeof(out));sentinel=out;
            if(phase>0)CHECK(pixaura_decode_context_init(1,&c,sizeof(c),id,32,&l)==0);
            if(phase>1)CHECK(pixaura_decode_open(&c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&source)==0);
            pixaura_working_limits wl{};CHECK(pixaura_working_default_limits(1,&wl)==0);
            if(phase>2)CHECK(pixaura_decode_image(&c,&source,&image)==0);
            if(phase>3)CHECK(pixaura_working_normalize(&c,&image,&wl,&working)==0);
            pixaura_decode_handle cancellation{};if(phase==9||phase==11||phase==13||phase==15||phase==18)CHECK(pixaura_cancel_create(&c,1,&cancellation)==0);
            pixaura_geometry_plan plan{},plan_sentinel;std::memset(&plan,0x5a,sizeof(plan));plan_sentinel=plan;
            pixaura_preview_ticket interactive{};if(phase==16)CHECK(pixaura_preview_begin(&c,1,&interactive)==0);
            const pixaura_preview_request fit{1,sizeof(pixaura_preview_request),1,1,2,0};
            fail_after=n;int32_t status;
            if(phase==16)status=pixaura_preview_render_interactive(&c,&working,&interactive,&fit,&out);
            else if(phase==17)status=pixaura_working_evaluate(&c,&working,1,tone_request,sizeof(tone_request)-1,&wl,&out);
            else if(phase==18)status=pixaura_working_evaluate_cancel(&c,&working,1,tone_request,sizeof(tone_request)-1,&wl,&cancellation,&out);
            else if(phase==0)status=pixaura_decode_context_init(1,&c,sizeof(c),id,32,&l);
            else if(phase==1)status=pixaura_decode_open(&c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&out);
            else if(phase==2)status=pixaura_decode_image(&c,&source,&out);
            else if(phase==3)status=pixaura_working_normalize(&c,&image,&wl,&out);
            else if(phase==4)status=pixaura_working_identity(&c,&working,&wl,&out);
            else if(phase==5)status=pixaura_evaluation_validate(1,request,sizeof(request)-1);
            else if(phase==6)status=pixaura_working_evaluate(&c,&working,1,request,sizeof(request)-1,&wl,&out);
            else if(phase==7)status=pixaura_geometry_preflight(1,2,3,request,sizeof(request)-1,&wl,&plan);
            else if(phase==8)status=pixaura_cancel_create(&c,1,&out);
            else if(phase==9)status=pixaura_working_evaluate_cancel(&c,&working,1,request,sizeof(request)-1,&wl,&cancellation,&out);
            else if(phase==10)status=pixaura_working_evaluate(&c,&working,1,geometry_request,sizeof(geometry_request)-1,&wl,&out);
            else if(phase==14||phase==15)status=pixaura_preview_render(&c,&working,&fit,phase==15?&cancellation:nullptr,&out);
            else if(phase==12||phase==13)status=pixaura_preview_create(&c,&working,1,phase==13?&cancellation:nullptr,&out);
            else status=pixaura_working_evaluate_cancel(&c,&working,1,geometry_request,sizeof(geometry_request)-1,&wl,&cancellation,&out);
            fail_after=-1;
            if(status&&phase==7)CHECK(std::memcmp(&plan,&plan_sentinel,sizeof(plan))==0);
            if(status==0)success=true;else{CHECK(status==8);++failures;if(phase==0){const pixaura_decode_context zero{};CHECK(std::memcmp(&c,&zero,sizeof(c))==0);}else CHECK(std::memcmp(&out,&sentinel,sizeof(out))==0);}
            if(phase>0||status==0){if(status!=0&&phase>0){pixaura_decode_handle valid{};if(phase==17)CHECK(pixaura_working_evaluate(&c,&working,1,tone_request,sizeof(tone_request)-1,&wl,&valid)==0);else if(phase==18)CHECK(pixaura_working_evaluate_cancel(&c,&working,1,tone_request,sizeof(tone_request)-1,&wl,&cancellation,&valid)==0);else if(phase==1)CHECK(pixaura_decode_open(&c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&valid)==0);else if(phase==2)CHECK(pixaura_decode_image(&c,&source,&valid)==0);else if(phase==3)CHECK(pixaura_working_normalize(&c,&image,&wl,&valid)==0);else if(phase==16){CHECK(pixaura_preview_begin(&c,1,&interactive)==0);CHECK(pixaura_preview_render_interactive(&c,&working,&interactive,&fit,&valid)==0);}else if(phase==14||phase==15)CHECK(pixaura_preview_render(&c,&working,&fit,phase==15?&cancellation:nullptr,&valid)==0);else if(phase==12||phase==13)CHECK(pixaura_preview_create(&c,&working,1,phase==13?&cancellation:nullptr,&valid)==0);else if(phase==4)CHECK(pixaura_working_identity(&c,&working,&wl,&valid)==0);else if(phase==5)CHECK(pixaura_evaluation_validate(1,request,sizeof(request)-1)==0);else if(phase==7)CHECK(pixaura_geometry_preflight(1,2,3,request,sizeof(request)-1,&wl,&plan)==0);else if(phase==8){CHECK(pixaura_cancel_create(&c,1,&valid)==0);CHECK(pixaura_cancel_release(&c,&valid)==0);}else if(phase==9||phase==11)CHECK(pixaura_working_evaluate_cancel(&c,&working,1,request,sizeof(request)-1,&wl,&cancellation,&valid)==0);else if(phase==10)CHECK(pixaura_working_evaluate(&c,&working,1,geometry_request,sizeof(geometry_request)-1,&wl,&valid)==0);else if(phase==11)CHECK(pixaura_working_evaluate_cancel(&c,&working,1,geometry_request,sizeof(geometry_request)-1,&wl,&cancellation,&valid)==0);else CHECK(pixaura_working_evaluate(&c,&working,1,request,sizeof(request)-1,&wl,&valid)==0);}CHECK(pixaura_decode_context_destroy(&c)==0);}
            if(success)break;
        }CHECK(success);
    }
    // New lifecycle control is fixed metadata: even fail-at-zero cannot break it.
    pixaura_decode_context fixed{};CHECK(pixaura_decode_context_init(1,&fixed,sizeof(fixed),id,32,&l)==0);pixaura_preview_ticket ticket{};
    fail_after=0;const auto begin=pixaura_preview_begin(&fixed,1,&ticket);const auto cancel=pixaura_preview_cancel(&fixed,&ticket);const auto stop=pixaura_preview_stop(&fixed);fail_after=-1;
    CHECK(begin==0&&cancel==0&&stop==0);CHECK(pixaura_decode_context_destroy(&fixed)==0);
    CHECK(failures>0);std::printf("decode C++ allocation sweep failures recovered=%u; phases=17\n",failures);
}
