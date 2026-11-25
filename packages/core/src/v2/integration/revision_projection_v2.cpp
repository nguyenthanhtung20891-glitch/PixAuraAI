#include "revision_projection_v2.hpp"
namespace pixaura::integration {
namespace {
// Count actual retained native container payload capacities. Inline strings are
// conservatively counted twice; allocator/control overhead is not process RSS.
uint64_t snapshot_payload(const document::Snapshot& snapshot){
    if(!snapshot)throw document::Failure{1};
    uint64_t bytes=sizeof(document::ImageDocument);
    const auto add=[&](uint64_t n){if(n>UINT64_MAX-bytes)throw document::Failure{8};bytes+=n;};
    const auto id=[&](const auto& value){add(value.text().capacity()+1);};
    const auto& d=*snapshot;id(d.identity().project);id(d.identity().document);id(d.source().sha256);id(d.current());id(d.session().id);
    add(d.source().metadata.codec.capacity()+1);if(d.source().metadata.icc)id(*d.source().metadata.icc);
    add(d.operations().capacity()*sizeof(document::EditOperation));
    for(const auto& op:d.operations()){id(op.id);add(op.type.capacity()+1);}
    add(d.revisions().capacity()*sizeof(document::Revision));
    for(const auto& r:d.revisions()){
        id(r.id);if(r.parent)id(*r.parent);if(r.plan)id(*r.plan);add(r.actor.capacity()+1);add(r.stack.capacity()*sizeof(document::Id));
        for(const auto& entry:r.stack)id(entry);
    }
    add(d.redo().capacity()*sizeof(document::Id));for(const auto& entry:d.redo())id(entry);
    return bytes;
}
}
evaluation::Stack RevisionProjectionV2::bound(const document::Snapshot& snapshot,const OriginalReaderV2& reader){
    if(!snapshot)throw document::Failure{1};
    const auto& source=snapshot->source();const auto& m=reader.metadata().value;
    if(source.sha256.text()!=reader.digest()||source.byte_length!=reader.encoded_bytes()||
       source.metadata.width!=m.width||source.metadata.height!=m.height||source.metadata.orientation!=m.orientation||
       source.metadata.codec!=(m.format==1?"jpeg":"png")||source.metadata.has_alpha!=bool(m.has_alpha)||source.metadata.icc)throw document::Failure{19};
    auto replayed=document::replay(*snapshot,snapshot->current());
    if(replayed.code)throw document::Failure{replayed.code};
    return std::move(replayed.value);
}
RevisionProjectionV2::RevisionProjectionV2(document::Snapshot snapshot,const OriginalReaderV2& reader,tiled::Budget& budget):snapshot_lease_(new tiled::Budget::Lease(budget.reserve(snapshot_payload(snapshot)))),snapshot_(std::move(snapshot)),plan_(reader,bound(snapshot_,reader),budget){}
}
