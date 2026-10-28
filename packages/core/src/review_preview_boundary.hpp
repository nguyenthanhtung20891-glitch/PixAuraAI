// Included after the read-only Auto boundary. Preview image resources only.
#include "review_raster.hpp"
namespace {
static_assert(sizeof(pixaura_review_preview_ticket)==56,"review preview ticket ABI");
static_assert(sizeof(pixaura_review_preview_pair)==176,"review preview pair ABI");
void review_ticket(Service& s,const pixaura_review_preview_ticket& t){
    require(t.api_version==1,2);require(t.struct_size==sizeof(t)&&t.kind==PIXAURA_REVIEW_PREVIEW_KIND&&!t.reserved,3);
    require(t.request_serial&&std::memcmp(&t,&s.review_preview.ticket,sizeof(t))==0,3);
}
void review_bound(Service& s,bool repository=true){
    auto& r=s.review_preview;s.ready();require(!r.cancelled,13);
    auto_current(s,r.proposal);require(s.auto_plan==r.plan,9);
    require(s.loaded.epoch==r.repository_epoch,9);
    if(repository)require(s.repository->read().epoch==r.repository_epoch,9);
    pixaura_execution_strategy strategy{};check(pixaura_policy_query(1,&s.raster,&strategy));
    require(strategy.epoch==r.policy_epoch,9);
}
void review_pair(Service& s,const pixaura_review_preview_pair& p,bool repository=true){
    review_ticket(s,p.ticket);require(s.review_preview.ready,13);
    require(std::memcmp(&p,&s.review_preview.pair,sizeof(p))==0,3);review_bound(s,repository);
}
void review_checkpoint(Service& s,uint32_t point){
#ifdef PIXAURA_CERTIFICATION_TESTING
    auto& r=s.review_preview;if(r.point.load()!=point)return;
    r.reached=1;const auto end=std::chrono::steady_clock::now()+std::chrono::seconds(15);
    while(!r.resume.load()){require(std::chrono::steady_clock::now()<end,14);std::this_thread::yield();}
#else
    (void)s;(void)point;
#endif
}
}
extern "C" int32_t pixaura_review_preview_begin(uint32_t version,pixaura_editor_context* c,
 const pixaura_auto_proposal* proposal,const uint8_t* input,uint64_t bytes,
 uint32_t width,uint32_t height,pixaura_review_preview_ticket* out){return attempt([&]{
    require(version==1,2);require(bytes&&bytes<=8192,8);require(width&&height&&width<=1024&&height<=1024);
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{proposal,sizeof(*proposal),alignof(pixaura_auto_proposal)},
        {input,bytes},{out,sizeof(*out),alignof(pixaura_review_preview_ticket)}});
    auto& s=owner(c);std::unique_lock<std::mutex> lock(s.gate,std::try_to_lock);require(lock.owns_lock(),8);
    s.ready();auto& r=s.review_preview;require(!r.active&&!s.agent_active&&!s.assist_active&&!s.auto_active,8);
    auto_current(s,*proposal);const std::string_view plan(reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes));
    require(plan==s.auto_plan,9);
    // Existing Plan parser and Registry guard, never approval/execution admission.
    const auto canonical=good(document::canonical_agent_schema(plan,2,*s.loaded.snapshot,s.document_identity));require(canonical==plan,6);
    const pixaura_agent_schema_request request{1,sizeof(request),2,1,0,{0,0,0}};uint64_t required=0;
    const auto code=pixaura_agent_tools_bind(1,1,&s.document,&s.live,&request,input,bytes,nullptr,0,&required);require(code==11,code?code:14);
    require(s.repository->read().epoch==s.loaded.epoch,9);require(r.sequence!=UINT64_MAX,8);
    pixaura_execution_strategy strategy{};check(pixaura_policy_query(1,&s.raster,&strategy));
    String prepared(plan);r.invalidate(s.raster);r.plan=std::move(prepared);r.proposal=*proposal;
    r.repository_epoch=s.loaded.epoch;r.policy_epoch=strategy.epoch;
    r.width=std::min(width,strategy.preview_edge);r.height=std::min(height,strategy.preview_edge);
    require(r.width&&r.height,8);
    pixaura_review_preview_ticket t{};t.api_version=1;t.struct_size=sizeof(t);t.kind=PIXAURA_REVIEW_PREVIEW_KIND;
    std::memcpy(t.context_id,s.seed.data(),32);t.request_serial=++r.sequence;
    r.ticket=t;r.pending=true;r.cancelled=false;*out=t;
});}
extern "C" int32_t pixaura_review_preview_render(uint32_t version,pixaura_editor_context* c,
 const pixaura_review_preview_ticket* ticket,pixaura_review_preview_pair* out){return attempt([&]{
    require(version==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},
        {ticket,sizeof(*ticket),alignof(pixaura_review_preview_ticket)},{out,sizeof(*out),alignof(pixaura_review_preview_pair)}});
    auto& s=owner(c);document::Snapshot snapshot;String plan,context;uint32_t edge=0;
    {std::unique_lock<std::mutex> lock(s.gate,std::try_to_lock);require(lock.owns_lock(),8);
     auto& r=s.review_preview;review_ticket(s,*ticket);require(!r.active&&!s.agent_active,8);require(r.pending,13);review_bound(s);
     r.pending=false;snapshot=s.loaded.snapshot;plan=r.plan;context=s.document_identity;edge=std::min(r.width,r.height);
     check(pixaura_cancel_create(&s.raster,1,&r.cancellation));r.active=true;}
    struct CallCleanup{Service& s;~CallCleanup(){std::lock_guard<std::mutex> lock(s.gate);auto& r=s.review_preview;
        if(r.cancellation.serial){(void)pixaura_cancel_release(&s.raster,&r.cancellation);}
        r.cancellation={};r.active=false;}} call{s};
    pixaura_decode_handle before{},after{},working{};
    struct Images{Service& s;pixaura_decode_handle &b,&a,&w;~Images(){if(w.serial)(void)pixaura_decode_release(&s.raster,&w);
        if(b.serial){(void)pixaura_preview_release(&s.raster,&b);}
        if(a.serial){(void)pixaura_preview_release(&s.raster,&a);}}} images{s,before,after,working};
    review_checkpoint(s,0);
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(s.review_preview.fault==1)throw std::bad_alloc();
    if(s.review_preview.fault==2){throw Failure{8};}
    if(s.review_preview.fault==3){throw Failure{14};}
#endif
    auto base=good(document::replay(*snapshot,snapshot->current()));auto stack=base;
    document::Vector<document::EditOperation> added;
    require(good(document::canonical_agent_schema(plan,2,*snapshot,context,&added))==plan,6);
    uint32_t id_serial=1;
    for(auto op:added){for(;;){require(id_serial<=264,8);String id(32,'0');const auto tail=std::to_string(id_serial++);
        id.replace(32-tail.size(),tail.size(),tail);bool used=false;for(const auto& prior:stack)if(prior.id.text()==id)used=true;
        if(!used){op.id=document::Id::parse(id);break;}}stack.push_back(op);}
    pixaura_review_preview_pair result{};
    check(review_raster_pair(&s.raster,&s.original,&s.review_preview.cancellation,base,stack,s.limits,edge,
        s.review_preview.policy_epoch,&before,&after,&result));
    result.ticket=*ticket;review_checkpoint(s,1);review_checkpoint(s,2);
    // Even a same-sized crop/rotation changes pixel correspondence; safe toggle
    // is used rather than claiming an unproved spatial split mapping.
    for(const auto& op:added)if(op.type=="pixaura.crop"||op.type=="pixaura.rotate")result.geometry_changed=1;

    std::lock_guard<std::mutex> lock(s.gate);review_ticket(s,*ticket);review_bound(s);
    auto& r=s.review_preview;r.before=before;r.after=after;before={};after={};r.pair=result;r.ready=true;*out=result;
});}
extern "C" int32_t pixaura_review_preview_current(uint32_t version,pixaura_editor_context* c,const pixaura_review_preview_pair* p){return attempt([&]{
    require(version==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{p,sizeof(*p),alignof(pixaura_review_preview_pair)}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);review_pair(s,*p);
});}
extern "C" int32_t pixaura_review_preview_copy(uint32_t version,pixaura_editor_context* c,const pixaura_review_preview_pair* p,
 uint32_t side,uint64_t offset,uint8_t* output,uint64_t bytes){return attempt([&]{
    require(version==1,2);require(side<=1&&bytes<=4194304);
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{p,sizeof(*p),alignof(pixaura_review_preview_pair)},{output,bytes}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);review_pair(s,*p,false);
    const auto image=side?s.review_preview.after:s.review_preview.before;require(image.serial,3);
    check(pixaura_preview_copy(&s.raster,&image,offset,output,bytes));
});}
extern "C" int32_t pixaura_review_preview_release(uint32_t version,pixaura_editor_context* c,const pixaura_review_preview_pair* p){return attempt([&]{
    require(version==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{p,sizeof(*p),alignof(pixaura_review_preview_pair)}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);review_ticket(s,p->ticket);
    require(std::memcmp(p,&s.review_preview.pair,sizeof(*p))==0,3);s.review_preview.release(s.raster);
});}
extern "C" int32_t pixaura_review_preview_cancel(uint32_t version,pixaura_editor_context* c,const pixaura_review_preview_ticket* t){return attempt([&]{
    require(version==1,2);execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{t,sizeof(*t),alignof(pixaura_review_preview_ticket)}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);review_ticket(s,*t);s.review_preview.invalidate(s.raster);
});}
#ifdef PIXAURA_CERTIFICATION_TESTING
extern "C" PIXAURA_API int32_t pixaura_test_review_preview_control(pixaura_editor_context* c,uint32_t command,uint32_t value){
    try{auto& s=owner(c);auto& r=s.review_preview;
        if(command==1){return static_cast<int32_t>(r.reached.load());}
        if(command==2){r.resume=1;return 0;}
        std::lock_guard<std::mutex> lock(s.gate);
        if(command==5)return static_cast<int32_t>((r.before.serial?r.pair.before.image_bytes:0)+(r.after.serial?r.pair.after.image_bytes:0));
        require(!r.active,8);
        if(command==0){require(value<=2||value==UINT32_MAX);r.point=value;r.reached=0;r.resume=0;}
        else if(command==3){require(value<=3);r.fault=value;}else return 1;
        return 0;
    }catch(const Failure& e){return e.code;}catch(...){return 14;}
}
#endif
