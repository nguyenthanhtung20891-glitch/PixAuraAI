#include "pixaura/store_ingress_v3.h"
#include "pixaura/editor_v2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <process.h>
#include <share.h>
#include <sys/stat.h>
static int file_open(const char* path,int flags,int mode){int fd=-1;return _sopen_s(&fd,path,flags,_SH_DENYNO,mode)==0?fd:-1;}
#define make_dir(p) _mkdir(p)
#define process_id _getpid
#define file_close _close
#ifndef O_BINARY
#define O_BINARY _O_BINARY
#endif
#else
#include <unistd.h>
#define make_dir(p) mkdir(p,0700)
#define process_id getpid
#define file_open open
#define file_close close
#define O_BINARY 0
#endif
typedef char pixaura_abi_check_2808[(sizeof(pixaura_v2_state)==112)?1:-1];
typedef char pixaura_abi_check_9562[(sizeof(pixaura_v2_preview)==40)?1:-1];
typedef char pixaura_abi_check_2755[(sizeof(pixaura_v2_advisory)==128)?1:-1];
typedef char pixaura_abi_check_6887[(sizeof(pixaura_v2_commit_result)==336)?1:-1];
#define CHECK(x) do{if(!(x)){fprintf(stderr,"C v2 consumer line %d\n",__LINE__);return 1;}}while(0)
int main(int argc,char** argv){
    CHECK(argc==2);char root[4096],project[4096],path[4096];
    CHECK(snprintf(root,sizeof(root),"editor-c-v2-%ld",(long)process_id())>0);CHECK(make_dir(root)==0);
    CHECK(snprintf(project,sizeof(project),"%s/project",root)>0);CHECK(make_dir(project)==0);
    /* Files requires canonical app-private roots. Resolve in this C consumer. */
#ifdef _WIN32
    char* absolute_root=_fullpath(NULL,root,0);char* absolute_project=_fullpath(NULL,project,0);
#else
    char* absolute_root=realpath(root,NULL);char* absolute_project=realpath(project,NULL);
#endif
    CHECK(absolute_root&&absolute_project);
    uint8_t synthetic_key[32];for(unsigned i=0;i<32;++i)synthetic_key[i]=(uint8_t)(i+1);
    pixaura_v2_handle runtime=0,editor=0,sentinel=123;
    CHECK(pixaura_v2_runtime_create(1,absolute_root,strlen(absolute_root),synthetic_key,32,&sentinel)==2&&sentinel==123);
    CHECK(pixaura_v2_runtime_create(2,absolute_root,strlen(absolute_root),synthetic_key,32,&runtime)==0&&runtime);
    const int input=file_open(argv[1],O_RDONLY|O_BINARY,0);CHECK(input>=0);
    struct stat metadata;CHECK(stat(argv[1],&metadata)==0);
    const int32_t opened=pixaura_v2_editor_open(2,runtime,absolute_project,strlen(absolute_project),input,(uint64_t)metadata.st_size,1,&editor);
    if(opened)fprintf(stderr,"v2 import bytes=%llu native-status=%d\n",(unsigned long long)metadata.st_size,opened);
    CHECK(opened==0);file_close(input);
    CHECK(pixaura_v2_editor_open(2,runtime,absolute_project,strlen(absolute_project),-1,0,0,&sentinel)==15&&sentinel==123);
    pixaura_v2_commercial_state commerce;CHECK(pixaura_v2_runtime_commercial_state(2,runtime,&commerce)==0&&commerce.photos==1&&commerce.access==0&&commerce.trial_start==-1);
    pixaura_v2_state state;CHECK(pixaura_v2_editor_state(2,editor,&state)==0&&state.revisions==1&&!state.recovery_only);
    pixaura_v2_preview before;CHECK(pixaura_v2_editor_preview(2,editor,64,48,1,&before)==0&&before.source_width==4000&&before.source_height==3000);
    uint32_t currency=123;CHECK(pixaura_v2_frame_probe(2,editor,before.frame,0,&currency)==0&&currency==0);
    currency=123;CHECK(pixaura_v2_frame_probe(2,editor,editor,0,&currency)==9&&currency==123);
    currency=123;CHECK(pixaura_v2_frame_probe(2,editor,before.frame,2,&currency)==1&&currency==123);
    CHECK(pixaura_v2_frame_probe(2,editor,before.frame,1,&currency)==0&&currency==0);
    uint8_t pixels[64*48*4];CHECK(pixaura_v2_frame_copy(2,editor,before.frame,pixels,sizeof(pixels))==0);
    CHECK(pixaura_v2_frame_copy(2,editor,editor,pixels,sizeof(pixels))==9);
    uint64_t cancelled_ticket=0;pixaura_v2_advisory cancelled_advice;
    CHECK(pixaura_v2_advisory_begin(2,editor,2,&cancelled_ticket)==0&&cancelled_ticket!=0);
    CHECK(pixaura_v2_advisory_request_cancel(2,editor,cancelled_ticket)==0);
    CHECK(pixaura_v2_advisory_run(2,editor,cancelled_ticket,&cancelled_advice)==13);
    CHECK(pixaura_v2_advisory_request_cancel(2,editor,cancelled_ticket)==9);
    CHECK(pixaura_v2_editor_select(2,editor,4)==0);int32_t amount=1000;
    /* Registry tool four is exposure, as certified by the unchanged registry. */
    CHECK(pixaura_v2_editor_update(2,editor,&amount,1)==0);
    CHECK(pixaura_v2_frame_copy(2,editor,before.frame,pixels,sizeof(pixels))==9);
    currency=123;CHECK(pixaura_v2_frame_probe(2,editor,before.frame,0,&currency)==0&&currency==2);
    uint32_t changed=0;CHECK(pixaura_v2_editor_apply(2,editor,&changed)==0&&changed==1);
    CHECK(pixaura_v2_editor_state(2,editor,&state)==0&&state.revisions==2&&state.can_undo);
    CHECK(pixaura_v2_runtime_commercial_state(2,runtime,&commerce)==0&&commerce.access==1&&commerce.trial_end-commerce.trial_start==14*86400);
    CHECK(pixaura_v2_editor_history(2,editor,0)==0);CHECK(pixaura_v2_editor_history(2,editor,1)==0);
    uint8_t binding[8192],candidate[8192],canonical[8192];uint64_t binding_bytes=0,canonical_bytes=0;
    CHECK(pixaura_v2_editor_agent_bytes(2,editor,1,NULL,0,NULL,0,&binding_bytes)==11&&binding_bytes>0&&binding_bytes<sizeof(binding));
    CHECK(pixaura_v2_editor_agent_bytes(2,editor,1,NULL,0,binding,sizeof(binding),&binding_bytes)==0);binding[binding_bytes]=0;
    const int plan_size=snprintf((char*)candidate,sizeof(candidate),"{\"actions\":[{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":101},\"tool_id\":\"pixaura.exposure\"}],\"binding\":%s,\"plan_version\":1}",binding);
    CHECK(plan_size>0&&(size_t)plan_size<sizeof(candidate));
    CHECK(pixaura_v2_editor_agent_bytes(2,editor,2,candidate,(uint64_t)plan_size,canonical,sizeof(canonical),&canonical_bytes)==0);
    uint64_t plan_ticket=123;CHECK(pixaura_v2_advisory_plan_begin(2,editor,(const uint8_t*)"{",1,&plan_ticket)!=0&&plan_ticket==123);
    CHECK(pixaura_v2_advisory_plan_begin(2,editor,canonical,canonical_bytes,&plan_ticket)==0&&plan_ticket!=123);
    CHECK(pixaura_v2_advisory_request_cancel(2,editor,plan_ticket)==0);
    CHECK(pixaura_v2_advisory_run(2,editor,plan_ticket,&cancelled_advice)==13);
    pixaura_v2_advisory advice;CHECK(pixaura_v2_advisory_request(2,editor,2,&advice)==0);
    uint64_t required=0;CHECK(pixaura_v2_advisory_content(2,editor,advice.id,32,1,NULL,0,&required)==11&&required>0&&required<=8192);
    uint8_t content[8192];CHECK(pixaura_v2_advisory_content(2,editor,advice.id,32,1,content,sizeof(content),&required)==0);
    pixaura_v2_review_inspection inspection;
    CHECK(sizeof(pixaura_v2_review_parameter)==112&&sizeof(pixaura_v2_review_action)==568&&sizeof(inspection)==4640);
    CHECK(pixaura_v2_advisory_inspect(2,editor,advice.id,32,&inspection)==0&&inspection.action_count==1&&inspection.version==2&&inspection.struct_size==4640);
    CHECK(inspection.actions[0].tool_index==advice.tool&&strcmp(inspection.provider_id,"reference.assistance.v1")==0);
    pixaura_v2_advisory edited;
    CHECK(pixaura_v2_advisory_edit_command(2,editor,advice.id,32,1,0,0,500,0,&edited)==0);
    CHECK(pixaura_v2_advisory_inspect(2,editor,advice.id,32,&inspection)==9);advice=edited;
    CHECK(pixaura_v2_advisory_inspect(2,editor,advice.id,32,&inspection)==0&&inspection.actions[0].parameters[0].raw==500);
    CHECK(pixaura_v2_advisory_edit_command(2,editor,advice.id,32,4,0,0,0,0,&edited)==0);advice=edited;
    uint64_t comparison_ticket=0;pixaura_v2_preview comparison_before,comparison_after;
    CHECK(pixaura_v2_review_preview_begin(2,editor,advice.id,32,64,48,&comparison_ticket)==0);
    CHECK(pixaura_v2_review_preview_render(2,editor,comparison_ticket,&comparison_before,&comparison_before)==1);
    CHECK(pixaura_v2_review_preview_render(2,editor,comparison_ticket,&comparison_before,&comparison_after)==0);
    CHECK(pixaura_v2_frame_current(2,editor,comparison_before.frame)==0&&pixaura_v2_frame_current(2,editor,comparison_after.frame)==0);
    CHECK(pixaura_v2_review_preview_cancel(2,editor,comparison_ticket)==0);
    CHECK(pixaura_v2_frame_current(2,editor,comparison_before.frame)==9&&pixaura_v2_frame_current(2,editor,comparison_after.frame)==9);
    CHECK(pixaura_v2_frame_release(2,comparison_before.frame)==0&&pixaura_v2_frame_release(2,comparison_after.frame)==0);
    CHECK(pixaura_v2_review_preview_release(2,editor,comparison_ticket)==0);
    char approval[33];memset(approval,'0',32);approval[32]=0;
    CHECK(pixaura_v2_advisory_apply(2,editor,advice.id,32,approval,32,&changed)==15);
    CHECK(pixaura_v2_advisory_approve(2,editor,advice.id,32,approval,sizeof(approval))==0);
    pixaura_v2_commit_result receipt;CHECK(pixaura_v2_advisory_apply_receipt(2,editor,advice.id,32,approval,32,&receipt)==0&&receipt.kind==2&&receipt.epoch>0&&!receipt.utc_authenticated);
    CHECK(pixaura_v2_editor_state(2,editor,&state)==0&&strcmp(receipt.revision,state.revision)==0&&strlen(receipt.event)==32&&strlen(receipt.revision_digest)==64);
    CHECK(pixaura_v2_advisory_apply(2,editor,advice.id,32,approval,32,&changed)==9);
    for(uint32_t format=1;format<=2;++format){
        CHECK(snprintf(path,sizeof(path),"%s/full.%s",root,format==1?"jpg":"png")>0);
        int output=file_open(path,O_RDWR|O_CREAT|O_EXCL|O_BINARY,0600);CHECK(output>=0);pixaura_v2_export_result exported;
        CHECK(sizeof(exported)==176);
        CHECK(pixaura_v2_editor_export_observed(2,editor,format,93,output,&exported)==0&&exported.bytes>0);
        CHECK(exported.version==2&&exported.struct_size==176&&exported.format==format&&exported.width==4000&&exported.height==3000);
        CHECK(strcmp(exported.revision,receipt.revision)==0);file_close(output);
    }
    CHECK(pixaura_v2_editor_close(2,editor)==0);CHECK(pixaura_v2_editor_state(2,editor,&state)==9);
    CHECK(pixaura_v2_runtime_close(2,runtime)==15);CHECK(pixaura_v2_frame_release(2,before.frame)==0);
    CHECK(pixaura_v2_editor_open(2,runtime,absolute_project,strlen(absolute_project),-1,0,0,&editor)==0);
    CHECK(pixaura_v2_editor_state(2,editor,&state)==0&&state.revisions==3);
    CHECK(pixaura_v2_editor_close(2,editor)==0);CHECK(pixaura_v2_runtime_close(2,runtime)==0);
    /* No missing-key replacement and no new commercial state are necessary to
     * inspect/export a previously approved project. Recovery confers no editing. */
    CHECK(pixaura_v2_runtime_recovery_create(2,absolute_root,strlen(absolute_root),NULL,NULL,NULL,&runtime)==0);
    CHECK(pixaura_v2_runtime_commercial_state(2,runtime,&commerce)==3003);
    CHECK(pixaura_v2_editor_open(2,runtime,absolute_project,strlen(absolute_project),-1,0,0,&editor)==0);
    CHECK(pixaura_v2_editor_state(2,editor,&state)==0&&state.revisions==3&&state.recovery_only&&state.commerce_unavailable);
    CHECK(pixaura_v2_editor_select(2,editor,4)==3003);
    CHECK(pixaura_v2_advisory_request(2,editor,2,&advice)==3003);
    pixaura_v2_preview recovery_frame;CHECK(pixaura_v2_editor_preview(2,editor,64,48,0,&recovery_frame)==0);
    CHECK(pixaura_v2_frame_copy(2,editor,recovery_frame.frame,pixels,sizeof(pixels))==0);CHECK(pixaura_v2_frame_release(2,recovery_frame.frame)==0);
    CHECK(snprintf(path,sizeof(path),"%s/recovery.png",root)>0);
    int recovery_output=file_open(path,O_RDWR|O_CREAT|O_EXCL|O_BINARY,0600);CHECK(recovery_output>=0);uint64_t recovery_bytes=0;
    CHECK(pixaura_v2_editor_export(2,editor,2,0,recovery_output,&recovery_bytes)==0&&recovery_bytes>0);file_close(recovery_output);
    CHECK(pixaura_v2_editor_close(2,editor)==0);CHECK(pixaura_v2_runtime_close(2,runtime)==0);
    /* Synthetic capability metadata exercises resource policy, NOT hardware
     * performance/admission certification. No GPU backend may be selected. */
    pixaura_capability_profile profile;CHECK(pixaura_capability_baseline(1,&profile)==0);
    profile.hardware.platform=PIXAURA_PLATFORM_APPLE;profile.hardware.architecture=PIXAURA_ARCH_ARM64;
    profile.hardware.logical_cpu_count=8;profile.hardware.physical_memory_bytes=UINT64_C(8589934592);
    profile.hardware.runtime_version=18;profile.hardware.provenance=PIXAURA_PROVENANCE_SYNTHETIC;
    profile.hardware.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;
    profile.runtime.thermal=PIXAURA_THERMAL_NORMAL;profile.runtime.memory_pressure=PIXAURA_PRESSURE_NORMAL;
    profile.runtime.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;profile.runtime.process_budget_bytes=UINT64_C(536870912);profile.runtime.sequence=1;
    pixaura_policy_observation observation={0};observation.version=1;observation.struct_size=sizeof(observation);
    observation.thermal=PIXAURA_POLICY_NORMAL;observation.memory_pressure=PIXAURA_POLICY_NORMAL;
    observation.gpu_availability=PIXAURA_CAPABILITY_UNAVAILABLE;observation.process_budget_bytes=UINT64_C(536870912);observation.sequence=1;
    pixaura_execution_strategy strategy;
    CHECK(pixaura_v2_runtime_create_policy(2,absolute_root,strlen(absolute_root),synthetic_key,32,&profile,&observation,&strategy,&runtime)==0);
    CHECK(strategy.forced_cpu==1&&strategy.backend==0&&strategy.runtime_cap_bytes<=UINT64_C(134217728));
    CHECK(pixaura_v2_editor_open(2,runtime,absolute_project,strlen(absolute_project),-1,0,0,&editor)==0);
    pixaura_v2_preview policy_frame;CHECK(pixaura_v2_editor_preview(2,editor,720,720,0,&policy_frame)==0);
    observation.memory_pressure=PIXAURA_POLICY_CRITICAL;observation.sequence=2;
    CHECK(pixaura_v2_runtime_update(2,runtime,&observation,&strategy)==0&&strategy.runtime_cap_bytes<=UINT64_C(33554432));
    CHECK(policy_frame.width>strategy.preview_edge||policy_frame.height>strategy.preview_edge);
    currency=123;CHECK(pixaura_v2_frame_probe(2,editor,policy_frame.frame,0,&currency)==8&&currency==123);
    CHECK(pixaura_v2_frame_release(2,policy_frame.frame)==0);
    /* Pressure preserves ordinary state and Cancel; only new allocations are
     * denied. The separate runtime pressure test proves small-image Apply. */
    CHECK(pixaura_v2_editor_state(2,editor,&state)==0&&state.revisions==3);
    CHECK(pixaura_v2_editor_cancel(2,editor)==0);
    CHECK(pixaura_v2_editor_open(2,runtime,absolute_project,strlen(absolute_project),-1,0,0,&sentinel)==15&&sentinel==123);
    CHECK(pixaura_v2_editor_close(2,editor)==0);CHECK(pixaura_v2_runtime_close(2,runtime)==0);
    CHECK(pixaura_v2_runtime_create_policy(2,absolute_root,strlen(absolute_root),synthetic_key,32,&profile,&observation,&strategy,&runtime)==0);
    /* Versioned managed reader may now use anonymous bounded rows. This
     * compressed source fits critical32; high-entropy rejection is exercised
     * separately by low_tier_application_v2_test without raising the cap. */
    CHECK(pixaura_v2_editor_open(2,runtime,absolute_project,strlen(absolute_project),-1,0,0,&editor)==0);
    CHECK(pixaura_v2_editor_state(2,editor,&state)==0&&state.revisions==3);
    CHECK(pixaura_v2_editor_close(2,editor)==0);
    CHECK(pixaura_v2_runtime_close(2,runtime)==0);
    free(absolute_root);free(absolute_project);
    puts("Shipping C ABI v2: real12MP native import/manual/AI/HUMANApply/history/reopen/JPEG/PNG; handle lifetime and v1 version isolation PASS");return 0;
}
