// Shared document ownership and Step 1 ABI defenses, no execution or approval.
namespace {
static_assert(sizeof(pixaura_agent_schema_request)==32,"schema request ABI");
struct SchemaLease {
    Registry& registry;
    ~SchemaLease(){std::lock_guard<std::mutex> lock(registry.mutex);registry.schema_busy=false;}
};
int32_t schema_copy(const String& value,uint8_t* output,uint64_t capacity,uint64_t* required){
    if(!output || capacity<value.size()){*required=value.size();return PIXAURA_DOCUMENT_BUFFER_TOO_SMALL;}
    std::memcpy(output,value.data(),value.size());*required=value.size();return PIXAURA_OK;
}
}
int32_t pixaura_agent_schema_binding(uint32_t version,pixaura_document_context* context,
    const pixaura_document_handle* base,uint8_t* output,uint64_t capacity,uint64_t* required){
    return boundary(nullptr,[&]{
        agent_version(version);
        agent_ranges({{context,sizeof(*context),alignof(pixaura_document_context)},
            {base,sizeof(*base),alignof(pixaura_document_handle)},{output,capacity},{required,sizeof(*required),alignof(uint64_t)}});
        const auto h=live(context);std::lock_guard<std::mutex> lock(h.registry->mutex);
        const auto snapshot=snapshot_locked(h,base);
        auto binding=pixaura::document::agent_schema_binding(*snapshot,{reinterpret_cast<const char*>(h.identity),32});
        if(binding.code)throw Failure{binding.code};
        return schema_copy(binding.value,output,capacity,required);
    });
}
int32_t pixaura_agent_schema_canonicalize(uint32_t version,pixaura_document_context* context,
    const pixaura_document_handle* base,const pixaura_agent_schema_request* request,
    const uint8_t* input,uint64_t bytes,uint8_t* output,uint64_t capacity,uint64_t* required){
    return boundary(nullptr,[&]{
        agent_version(version);
        require(bytes && bytes<=PIXAURA_AGENT_MAX_BYTES,PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        agent_ranges({{context,sizeof(*context),alignof(pixaura_document_context)},
            {base,sizeof(*base),alignof(pixaura_document_handle)},
            {request,sizeof(*request),alignof(pixaura_agent_schema_request)},
            {input,bytes},{output,capacity},{required,sizeof(*required),alignof(uint64_t)}});
        require(request->api_version==1,PIXAURA_UNSUPPORTED_ABI);
        require(request->struct_size==sizeof(*request) && request->reserved[0]==0 && request->reserved[1]==0 && request->reserved[2]==0);
        require((request->kind==1 || request->kind==2) && request->require_canonical<=1 && request->pre_cancel<=1);
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
        auto canonical=pixaura::document::canonical_agent_schema(
            {reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes)},request->kind,*snapshot,context_id);
#ifdef PIXAURA_CERTIFICATION_TESTING
        if(checkpoint)checkpoint(checkpoint_state);
#endif
        // Cancellation and live token liveness are checked before ANY publication.
        std::lock_guard<std::mutex> lock(h.registry->mutex);
        require(epoch==h.registry->schema_epoch,PIXAURA_DOCUMENT_CANCELLED);
        require(h.registry->snapshots.count(base->serial)!=0,PIXAURA_DOCUMENT_STALE_BASE);
        if(canonical.code)throw Failure{canonical.code};
        if(request->require_canonical)require(canonical.value==std::string_view(reinterpret_cast<const char*>(input),static_cast<std::size_t>(bytes)),PIXAURA_DOCUMENT_INVALID_PROJECT);
        return schema_copy(canonical.value,output,capacity,required);
    });
}
#ifdef PIXAURA_CERTIFICATION_TESTING
int32_t pixaura_agent_schema_test_checkpoint(pixaura_document_context* context,void (*checkpoint)(void*),void* state){
    return boundary(nullptr,[&]{
        const auto h=live(context);std::lock_guard<std::mutex> lock(h.registry->mutex);
        require(!h.registry->schema_busy,PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        h.registry->schema_checkpoint=checkpoint;h.registry->schema_checkpoint_state=state;return PIXAURA_OK;
    });
}
#endif
int32_t pixaura_agent_schema_cancel(uint32_t version,pixaura_document_context* context){
    return boundary(nullptr,[&]{
        agent_version(version);agent_ranges({{context,sizeof(*context),alignof(pixaura_document_context)}});
        const auto h=live(context);std::lock_guard<std::mutex> lock(h.registry->mutex);
        require(h.registry->schema_epoch!=UINT64_MAX,PIXAURA_DOCUMENT_RESOURCE_LIMIT);
        ++h.registry->schema_epoch;return PIXAURA_OK;
    });
}
