// Included after the existing schema boundary; same ownership and cancellation authority.
namespace {
static_assert(sizeof(pixaura_reference_planner_request)==32,"reference request ABI");
int32_t reference_work(bool plan,uint32_t version,pixaura_document_context* context,
    const pixaura_document_handle* base,const pixaura_reference_planner_request* request,
    const uint8_t* input,uint64_t bytes,uint8_t* output,uint64_t capacity,uint64_t* required){
    return boundary(nullptr,[&]{
        agent_version(version);
        require(bytes && bytes<=PIXAURA_STRUCTURED_INTENT_MAX_BYTES,PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        agent_ranges({{context,sizeof(*context),alignof(pixaura_document_context)},
            {base,sizeof(*base),alignof(pixaura_document_handle)},
            {request,sizeof(*request),alignof(pixaura_reference_planner_request)},
            {input,bytes},{output,capacity},{required,sizeof(*required),alignof(uint64_t)}});
        agent_version(request->api_version);agent_version(request->planner_version);agent_version(request->registry_version);
        require(request->struct_size==sizeof(*request) && request->pre_cancel<=1 &&
            request->reserved[0]==0 && request->reserved[1]==0 && request->reserved[2]==0);
        require(!request->pre_cancel,PIXAURA_DOCUMENT_CANCELLED);
        const auto h=live(context);Snapshot snapshot;uint64_t epoch=0;
        const String context_id(reinterpret_cast<const char*>(h.identity),32);
#ifdef PIXAURA_CERTIFICATION_TESTING
        void (*checkpoint)(void*)=nullptr;void* checkpoint_state=nullptr;
#endif
        {
            std::lock_guard<std::mutex> lock(h.registry->mutex);
            snapshot=snapshot_locked(h,base);
            require(!h.registry->schema_busy && h.registry->schema_epoch!=UINT64_MAX,PIXAURA_DOCUMENT_RESOURCE_LIMIT);
            epoch=h.registry->schema_epoch;h.registry->schema_busy=true;
#ifdef PIXAURA_CERTIFICATION_TESTING
            checkpoint=h.registry->schema_checkpoint;checkpoint_state=h.registry->schema_checkpoint_state;
#endif
        }
        SchemaLease lease{*h.registry};
#ifdef PIXAURA_CERTIFICATION_TESTING
        if(checkpoint)checkpoint(checkpoint_state); // in-flight before bounded mapping
#endif
        {
            std::lock_guard<std::mutex> lock(h.registry->mutex);
            require(epoch==h.registry->schema_epoch,PIXAURA_DOCUMENT_CANCELLED);
        }
        auto result=plan_structured_intent({reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes)},*snapshot,context_id);
        if(!result.code)agent_bind_tools(result.value.operations); // EXACT Step 3 binding authority
#ifdef PIXAURA_CERTIFICATION_TESTING
        if(checkpoint)checkpoint(checkpoint_state); // before any successful publication
#endif
        std::lock_guard<std::mutex> lock(h.registry->mutex);
        require(epoch==h.registry->schema_epoch,PIXAURA_DOCUMENT_CANCELLED);
        require(h.registry->snapshots.count(base->serial)!=0,PIXAURA_DOCUMENT_STALE_BASE);
        if(result.code)throw Failure{result.code};
        if(plan)require(result.value.intent==std::string_view(reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes)),PIXAURA_DOCUMENT_INVALID_PROJECT);
        return schema_copy(plan?result.value.plan:result.value.intent,output,capacity,required);
    });
}
}
int32_t pixaura_structured_intent_canonicalize(uint32_t version,pixaura_document_context* context,
    const pixaura_document_handle* base,const pixaura_reference_planner_request* request,
    const uint8_t* input,uint64_t bytes,uint8_t* output,uint64_t capacity,uint64_t* required){
    return reference_work(false,version,context,base,request,input,bytes,output,capacity,required);
}
int32_t pixaura_reference_planner_plan(uint32_t version,pixaura_document_context* context,
    const pixaura_document_handle* base,const pixaura_reference_planner_request* request,
    const uint8_t* input,uint64_t bytes,uint8_t* output,uint64_t capacity,uint64_t* required){
    return reference_work(true,version,context,base,request,input,bytes,output,capacity,required);
}
