#include "pixaura/editor_v2.h"
#include "../core/src/decode_codec.h"
#include "../core/src/assist_experience.hpp"
#include <filesystem>
#include <fstream>
#include <vector>
#include <array>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstdio>
#include <cstring>
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
#define CHECK(x) do{if(!(x))throw std::runtime_error("runtime pressure line "+std::to_string(__LINE__));}while(false)
#define OK(x) do{const auto code=(x);if(code)throw std::runtime_error("runtime pressure status "+std::to_string(code)+" line "+std::to_string(__LINE__));}while(false)
extern "C" int32_t pixaura_test_editor_v2_control(uint64_t,uint32_t,uint32_t,uint32_t);
namespace fs=std::filesystem;
int main(){fs::path root;uint64_t runtime=0,editor=0;std::vector<pixaura_v2_preview> held;try{
    root=fs::current_path()/("runtime-pressure-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    CHECK(fs::create_directory(root));const auto project=root/"project";CHECK(fs::create_directory(project));
    std::vector<uint8_t> pixels(1024*512*4,128);for(size_t i=3;i<pixels.size();i+=4)pixels[i]=255;
    std::vector<uint8_t> png(pixels.size()+65536);size_t written=0;CHECK(!pixaura_codec_encode_png(pixels.data(),1024,512,png.data(),png.size(),&written,32u<<20));
    const auto input_path=root/"synthetic.png";{std::ofstream file(input_path,std::ios::binary);file.write(reinterpret_cast<const char*>(png.data()),static_cast<std::streamsize>(written));CHECK(file.good());}
    pixaura_capability_profile profile{};OK(pixaura_capability_baseline(1,&profile));
    profile.hardware.platform=PIXAURA_PLATFORM_APPLE;profile.hardware.architecture=PIXAURA_ARCH_ARM64;
    profile.hardware.logical_cpu_count=8;profile.hardware.physical_memory_bytes=UINT64_C(8589934592);
    profile.hardware.runtime_version=18;profile.hardware.provenance=PIXAURA_PROVENANCE_SYNTHETIC;
    profile.hardware.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;
    profile.runtime.thermal=PIXAURA_THERMAL_NORMAL;profile.runtime.memory_pressure=PIXAURA_PRESSURE_NORMAL;
    profile.runtime.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;profile.runtime.process_budget_bytes=UINT64_C(536870912);profile.runtime.sequence=1;
    pixaura_policy_observation observation{};observation.version=1;observation.struct_size=sizeof(observation);
    observation.thermal=PIXAURA_POLICY_NORMAL;observation.memory_pressure=PIXAURA_POLICY_NORMAL;
    observation.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;observation.process_budget_bytes=UINT64_C(536870912);observation.sequence=1;
    std::array<uint8_t,32> synthetic{};for(unsigned i=0;i<32;++i)synthetic[i]=static_cast<uint8_t>(i+1);
    const auto path=root.string(),source=project.string();pixaura_execution_strategy strategy{};
    OK(pixaura_v2_runtime_create_policy(2,path.c_str(),path.size(),synthetic.data(),32,&profile,&observation,&strategy,&runtime));
    const auto input=fd_open(input_path.string().c_str(),O_RDONLY|O_BINARY);CHECK(input>=0);
    const auto opened=pixaura_v2_editor_open(2,runtime,source.c_str(),source.size(),input,written,1,&editor);fd_close(input);CHECK(!opened);
    // Metadata observation must not turn an unchanged worker-side frame copy
    // into a stale-result failure. UI current checks remain nonblocking.
    std::atomic<bool> observing{true};std::atomic<int32_t> observer_error{0};
    std::thread observer([&]{for(unsigned read=0;read<4096&&observing;++read){pixaura_v2_state observed{};const auto code=pixaura_v2_editor_state(2,editor,&observed);if(code){observer_error=code;break;}}});
    int32_t copy_error=0;
    for(unsigned i=0;i<40&&!copy_error;++i){pixaura_v2_preview frame{};copy_error=pixaura_v2_editor_preview(2,editor,64,32,0,&frame);if(copy_error)break;
        std::vector<uint8_t> bytes(static_cast<size_t>(frame.bytes));copy_error=pixaura_v2_frame_copy(2,editor,frame.frame,bytes.data(),bytes.size());const auto released=pixaura_v2_frame_release(2,frame.frame);if(!copy_error)copy_error=released;}
    observing=false;observer.join();CHECK(observer_error==0);OK(copy_error);
    // DEBUG hooks pause the real provider/validator. Cancellation and malformed
    // envelopes must not publish revisions or consume a usable AI delivery.
    pixaura_v2_commercial_state commercial{};OK(pixaura_v2_runtime_commercial_state(2,runtime,&commercial));const auto ai_before=commercial.ai;
    OK(pixaura_test_editor_v2_control(editor,2,0,1));uint64_t paused_ticket=0;OK(pixaura_v2_advisory_begin(2,editor,2,&paused_ticket));
    int32_t paused_result=0;std::thread paused([&]{pixaura_v2_advisory advice{};paused_result=pixaura_v2_advisory_run(2,editor,paused_ticket,&advice);});
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(!pixaura_test_editor_v2_control(editor,2,1,0)&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
    const auto reached=pixaura_test_editor_v2_control(editor,2,1,0);
    pixaura_v2_state before_duplicate{},after_duplicate{};const auto observed_before=pixaura_v2_editor_state(2,editor,&before_duplicate);
    uint64_t duplicate=123;const auto duplicate_status=pixaura_v2_advisory_begin(2,editor,2,&duplicate);
    const auto observed_after=pixaura_v2_editor_state(2,editor,&after_duplicate);
    const auto cancelled=pixaura_v2_advisory_request_cancel(2,editor,paused_ticket);
    const auto resumed=pixaura_test_editor_v2_control(editor,2,2,0);paused.join();
    CHECK(duplicate_status==8&&duplicate==123&&observed_before==0&&observed_after==0&&before_duplicate.generation==after_duplicate.generation);
    CHECK(reached==1&&cancelled==0&&resumed==0&&paused_result==13);
    OK(pixaura_test_editor_v2_control(editor,2,0,1));paused_ticket=0;OK(pixaura_v2_advisory_begin(2,editor,2,&paused_ticket));
    paused_result=0;std::thread changing([&]{pixaura_v2_advisory advice{};paused_result=pixaura_v2_advisory_run(2,editor,paused_ticket,&advice);});
    struct ResumeJoin {uint64_t editor;std::thread& worker;uint32_t family=2;~ResumeJoin(){(void)pixaura_test_editor_v2_control(editor,family,2,0);if(worker.joinable())worker.join();}} resumed_on_error{editor,changing};
    const auto change_deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(!pixaura_test_editor_v2_control(editor,2,1,0)&&std::chrono::steady_clock::now()<change_deadline)std::this_thread::yield();
    CHECK(pixaura_test_editor_v2_control(editor,2,1,0)==1);
    OK(pixaura_v2_editor_select(2,editor,4));int32_t manual_value=100;OK(pixaura_v2_editor_update(2,editor,&manual_value,1));
    pixaura_v2_preview manual_frame{};OK(pixaura_v2_editor_preview(2,editor,64,32,0,&manual_frame));
    std::vector<uint8_t> manual_pixels(static_cast<size_t>(manual_frame.bytes));OK(pixaura_v2_frame_copy(2,editor,manual_frame.frame,manual_pixels.data(),manual_pixels.size()));OK(pixaura_v2_frame_release(2,manual_frame.frame));
    OK(pixaura_v2_editor_cancel(2,editor));OK(pixaura_test_editor_v2_control(editor,2,2,0));changing.join();CHECK(paused_result==9);
    OK(pixaura_v2_runtime_commercial_state(2,runtime,&commercial));CHECK(commercial.ai==ai_before&&commercial.pending==0);
    OK(pixaura_test_editor_v2_control(editor,2,0,UINT32_MAX));
    for(const auto fault: {2u,3u,4u,6u,7u,8u,9u,10u,12u}){
        OK(pixaura_test_editor_v2_control(editor,2,3,fault));pixaura_v2_advisory result{};
        CHECK(pixaura_v2_advisory_request(2,editor,2,&result)!=0);
        OK(pixaura_v2_runtime_commercial_state(2,runtime,&commercial));CHECK(commercial.ai==ai_before&&commercial.pending==0);
        pixaura_v2_state clean{};OK(pixaura_v2_editor_state(2,editor,&clean));CHECK(clean.revisions==1);
    }
    OK(pixaura_test_editor_v2_control(editor,2,3,0));
    OK(pixaura_v2_editor_select(2,editor,4));int32_t value=1234;OK(pixaura_v2_editor_update(2,editor,&value,1));
    pixaura_v2_preview reference{};OK(pixaura_v2_editor_preview(2,editor,360,180,0,&reference));
    std::vector<uint8_t> expected(static_cast<size_t>(reference.bytes));OK(pixaura_v2_frame_copy(2,editor,reference.frame,expected.data(),expected.size()));OK(pixaura_v2_frame_release(2,reference.frame));
    observation.memory_pressure=PIXAURA_POLICY_CRITICAL;++observation.sequence;OK(pixaura_v2_runtime_update(2,runtime,&observation,&strategy));
    CHECK(strategy.runtime_cap_bytes==UINT64_C(33554432)&&strategy.preview_edge==360);
    pixaura_v2_state state{};OK(pixaura_v2_editor_state(2,editor,&state));CHECK(state.selected==4&&state.values[0]==1234&&state.revisions==1);
    pixaura_v2_preview reduced{};OK(pixaura_v2_editor_preview(2,editor,360,180,0,&reduced));
    std::vector<uint8_t> actual(static_cast<size_t>(reduced.bytes));OK(pixaura_v2_frame_copy(2,editor,reduced.frame,actual.data(),actual.size()));CHECK(actual==expected);OK(pixaura_v2_frame_release(2,reduced.frame));
    uint32_t changed=0;OK(pixaura_v2_editor_apply(2,editor,&changed));CHECK(changed==1);OK(pixaura_v2_editor_state(2,editor,&state));CHECK(state.revisions==2);
    // Every frozen registry unit and raw parameter ordering must fit the NEW
    // uncommitted v2 inspection transport, including long highlight/shadow units.
    for(uint32_t choice=0;choice<11;++choice){
        pixaura_v2_advisory result{};OK(pixaura_v2_advisory_request(2,editor,choice,&result));
        pixaura_v2_review_inspection inspected{};OK(pixaura_v2_advisory_inspect(2,editor,result.id,32,&inspected));CHECK(inspected.action_count==1&&inspected.struct_size==4640);
        const auto& choice_expected=pixaura::assist_experience::choice(choice);const auto& descriptor=pixaura::manual::registry()[choice_expected.tool_index];const auto& action=inspected.actions[0];
        CHECK(action.tool_index==choice_expected.tool_index&&action.parameter_count==descriptor.parameter_count);
        for(uint32_t p=0;p<action.parameter_count;++p){CHECK(std::string(action.parameters[p].name)==descriptor.parameters[p].name);CHECK(std::string(action.parameters[p].unit)==descriptor.parameters[p].unit);CHECK(action.parameters[p].raw==choice_expected.values[p]);}
        OK(pixaura_v2_advisory_cancel(2,editor,result.id,32));
    }
    observation.memory_pressure=PIXAURA_POLICY_NORMAL;
    for(unsigned i=0;i<PIXAURA_POLICY_RECOVERY_OBSERVATIONS;++i){++observation.sequence;OK(pixaura_v2_runtime_update(2,runtime,&observation,&strategy));}
    CHECK(strategy.runtime_cap_bytes==UINT64_C(134217728));
    for(unsigned i=0;i<12;++i){pixaura_v2_preview frame{};OK(pixaura_v2_editor_preview(2,editor,512,256,0,&frame));held.push_back(frame);}
    pixaura_v2_advisory advice{};OK(pixaura_v2_advisory_request(2,editor,2,&advice));char approval[33]{};OK(pixaura_v2_advisory_approve(2,editor,advice.id,32,approval,33));
    observation.memory_pressure=PIXAURA_POLICY_CRITICAL;++observation.sequence;OK(pixaura_v2_runtime_update(2,runtime,&observation,&strategy));
    changed=777;CHECK(pixaura_v2_advisory_apply(2,editor,advice.id,32,approval,32,&changed)==8&&changed==777);
    CHECK(pixaura_v2_advisory_apply(2,editor,advice.id,32,approval,32,&changed)==15);
    OK(pixaura_v2_editor_state(2,editor,&state));CHECK(state.revisions==2);
    for(const auto& frame:held)OK(pixaura_v2_frame_release(2,frame.frame));
    held.clear();
    OK(pixaura_v2_advisory_approve(2,editor,advice.id,32,approval,33));OK(pixaura_v2_advisory_apply(2,editor,advice.id,32,approval,32,&changed));CHECK(changed==1);
    OK(pixaura_v2_editor_state(2,editor,&state));CHECK(state.revisions==3);
    pixaura_v2_advisory comparison{};OK(pixaura_v2_advisory_request(2,editor,2,&comparison));
    uint64_t comparison_ticket=0;OK(pixaura_v2_review_preview_begin(2,editor,comparison.id,32,64,32,&comparison_ticket));
    OK(pixaura_test_editor_v2_control(editor,4,0,0));pixaura_v2_preview left{},right{};left.frame=1234;right.frame=5678;int32_t comparison_status=0;
    std::thread preview_worker([&]{comparison_status=pixaura_v2_review_preview_render(2,editor,comparison_ticket,&left,&right);});
    ResumeJoin preview_cleanup{editor,preview_worker,4};
    const auto preview_deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(!pixaura_test_editor_v2_control(editor,4,1,0)&&std::chrono::steady_clock::now()<preview_deadline)std::this_thread::yield();
    CHECK(pixaura_test_editor_v2_control(editor,4,1,0)==1);OK(pixaura_v2_editor_state(2,editor,&state));CHECK(state.revisions==3);
    OK(pixaura_v2_review_preview_cancel(2,editor,comparison_ticket));OK(pixaura_test_editor_v2_control(editor,4,2,0));preview_worker.join();
    CHECK(comparison_status==13&&left.frame==1234&&right.frame==5678);
    pixaura_v2_review_inspection intact{};OK(pixaura_v2_advisory_inspect(2,editor,comparison.id,32,&intact));CHECK(intact.action_count==1);
    OK(pixaura_v2_review_preview_begin(2,editor,comparison.id,32,64,32,&comparison_ticket));OK(pixaura_test_editor_v2_control(editor,4,0,0));
    comparison_status=0;std::thread retiring_worker([&]{comparison_status=pixaura_v2_review_preview_render(2,editor,comparison_ticket,&left,&right);});
    ResumeJoin retirement_cleanup{editor,retiring_worker,4};
    const auto retirement_deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(!pixaura_test_editor_v2_control(editor,4,1,0)&&std::chrono::steady_clock::now()<retirement_deadline)std::this_thread::yield();
    CHECK(pixaura_test_editor_v2_control(editor,4,1,0)==1);OK(pixaura_v2_editor_close(2,editor));editor=0;retiring_worker.join();
    CHECK(comparison_status==13&&left.frame==1234&&right.frame==5678);
    OK(pixaura_v2_runtime_close(2,runtime));runtime=0;
    CHECK(fs::remove_all(root)>0);std::puts("native app pressure: gesture/bitwise preview/Apply/retained-memory denial/single-use approval/recovery PASS");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());for(const auto& frame:held)(void)pixaura_v2_frame_release(2,frame.frame);if(editor)(void)pixaura_v2_editor_close(2,editor);if(runtime)(void)pixaura_v2_runtime_close(2,runtime);return 1;}catch(...){std::fputs("unexpected native pressure failure\n",stderr);return 2;}}
