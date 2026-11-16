#include "native_owner_v1.hpp"
#include "../core/src/decode_codec.h"
#include <filesystem>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <thread>
using namespace pixaura;
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"native commerce check %d\n",__LINE__);std::exit(1);}}while(false)
namespace pixaura::commerce {
struct NativeCommercialOwnerTestAccess {
    static void hook(void (*callback)(unsigned)){NativeCommercialOwner::checkpoint_=callback;}
    static void synthetic_store_fact(NativeCommercialOwner& owner,const Entitlement& fact){owner.verified_store_fact("synthetic-only-not-a-store-product",fact);}
};
}
static std::string hex(unsigned n){char s[33];std::snprintf(s,sizeof s,"%032x",n);return s;}
static commerce::Clock clock_value(){return {20000LL*86400+100,100,hex(500),false};}
struct TestClock:commerce::NativeClockSourceV1{commerce::Clock value=clock_value();commerce::Clock sample()override{return value;}};
static TestClock* advancing_clock=nullptr;
static void midnight(unsigned point){if(point==2){commerce::NativeCommercialOwnerTestAccess::hook(nullptr);advancing_clock->value.uptime_seconds+=2;advancing_clock->value.utc_seconds+=2;}}
static std::vector<uint8_t> png(unsigned seed=0){
    std::vector<uint8_t> rgba(7*11*4,255),encoded(65536);size_t written=0;
    for(std::size_t i=0;i<rgba.size();i+=4){rgba[i]=uint8_t(i%255);rgba[i+1]=uint8_t((i/4)%255);}
    rgba[0]=uint8_t(seed);
    CHECK(!pixaura_codec_encode_png(rgba.data(),7,11,encoded.data(),encoded.size(),&written,32u<<20));encoded.resize(written);encoded.shrink_to_fit();return encoded;
}
struct Input:storage::Reader{
    const std::vector<uint8_t>& input;std::size_t offset=0;explicit Input(const std::vector<uint8_t>& data):input(data){}
    std::size_t read(uint8_t* out,std::size_t n)override{n=std::min(n,input.size()-offset);std::memcpy(out,input.data()+offset,n);offset+=n;return n;}
};
static void setup(const std::string& root,unsigned seed=0){
    CHECK(std::filesystem::create_directory(root));auto encoded=png(seed);storage::AssetStore store(root);Input input(encoded);const auto asset=store.ingest(input,encoded.size());
    const auto manifest="{\"schema_version\":1,\"document_id\":\""+hex(2)+"\",\"project_id\":\""+hex(1)+"\",\"current_revision_id\":\""+hex(100)+"\",\"source\":{\"sha256\":\""+asset.digest+"\",\"byte_length\":"+std::to_string(encoded.size())+",\"metadata\":{\"width\":7,\"height\":11,\"orientation\":1,\"codec\":\"png\",\"has_alpha\":true,\"icc_sha256\":null}},\"operations\":[],\"revisions\":[{\"id\":\""+hex(100)+"\",\"parent_id\":null,\"actor\":\"import\",\"plan_id\":null,\"stack\":[]}],\"redo\":[]}";
    auto created=document::create_document(manifest,hex(500));CHECK(!created.code);storage::Repository repository(root);repository.create(*created.value);repository.migrate(1,2);repository.migrate(2,3);
}
static void manual_edit(const std::string& root){
    storage::Repository repository(root);const auto loaded=repository.read();const auto& doc=*loaded.snapshot;
    const auto command="{\"command_version\":1,\"kind\":\"commit\",\"expected_revision_id\":\""+doc.current().text()+"\",\"expected_session_id\":\""+doc.session().id.text()+"\",\"expected_generation\":"+std::to_string(doc.session().generation)+",\"revision_id\":\""+hex(101)+"\",\"actor\":\"manual\",\"plan_id\":null,\"operations\":[{\"id\":\""+hex(11)+"\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":100}}],\"stack\":[\""+hex(11)+"\"]}";
    repository.apply(command,loaded.epoch);
}
static unsigned crash_point=0;
static void crash(unsigned point){if(point==crash_point)std::_Exit(0);}
static commerce::NativeCommercialOwner* cancelling_owner=nullptr;
static void cancel_before_delivery(unsigned point){if(point==2){commerce::NativeCommercialOwnerTestAccess::hook(nullptr);CHECK(cancelling_owner->cancel_advisory(hex(201))==commerce::NativeCancelV1::requested);}}
static std::string stale_root;static unsigned blocked_writers=0;
static void stale(unsigned point){if(point==2){commerce::NativeCommercialOwnerTestAccess::hook(nullptr);try{manual_edit(stale_root);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==15);++blocked_writers;}storage::Repository recovery(stale_root);CHECK(recovery.read().snapshot->revisions().size()==1);}}
int main(int argc,char** argv){
    TestClock test_clock;
    if(argc==3&&std::string(argv[1])=="setup"){setup(argv[2]);return 0;}
    if(argc>=4){
        const std::string mode=argv[1],root=argv[2],ledger=argv[3];auto encoded=png();tiled::Budget budget(134217728,encoded.capacity());integration::OriginalReaderV2 reader(encoded.data(),encoded.size(),budget);
        if(mode=="crash"){
            CHECK(argc==5);crash_point=static_cast<unsigned>(std::stoul(argv[4]));commerce::NativeCommercialOwner owner(ledger,root,reader,budget,test_clock,hex(700));commerce::NativeCommercialOwnerTestAccess::hook(crash);
            owner.deliver_advisory(hex(1),"adjust.exposure","{\"milli_ev\":1250}");CHECK(false);
        }else{
            {storage::Repository repository(root);repository.open_session(hex(501));}
            commerce::NativeCommercialOwner owner(ledger,root,reader,budget,test_clock,hex(701));const auto state=owner.state();
            if(mode=="inspect-delivered"){
                CHECK(state.ai==1&&state.photos==1&&state.pending==0);const auto token=std::to_string(clock_value().utc_seconds/86400)+":"+hex(1);
                auto restored=owner.restore_advisory(token);CHECK(restored.plan.find(hex(701))!=std::string::npos&&restored.plan.find(hex(501))!=std::string::npos);CHECK(owner.state().ai==1);
            }else{CHECK(state.ai==0&&state.photos==0&&state.pending==0);}
            CHECK(state.trial_start==-1&&commerce::NativeCommercialOwner::recovery_allowed());return 0;
        }
    }
    const auto name="v18-commerce-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());const auto root=(std::filesystem::current_path()/name).string();setup(root);
    auto encoded=png();tiled::Budget budget(134217728,encoded.capacity());integration::OriginalReaderV2 reader(encoded.data(),encoded.size(),budget);
    {
        commerce::NativeCommercialOwner owner(root+"/commercial.sqlite",root,reader,budget,test_clock,hex(700));
        for(unsigned i=10;i<110;++i)CHECK(owner.admit_editable_source(hex(i)).decision==commerce::Admission::allowed);
        CHECK(owner.state().photos==1&&owner.state().trial_start==-1);
        const auto first=owner.deliver_advisory(hex(1),"adjust.exposure","{\"milli_ev\":1250}");CHECK(first.receipt.source==reader.digest()&&first.receipt.encoded_bytes==encoded.size());
        CHECK(owner.deliver_advisory(hex(1),"adjust.exposure","{\"milli_ev\":1250}").plan==first.plan&&owner.state().ai==1);
        for(const auto& parameters:std::vector<std::string>{"{\"milli_ev\":999999}","{\"milli_ev\":100,\"approved\":true}","{broken}"}){
            try{owner.deliver_advisory(hex(200),"adjust.exposure",parameters);CHECK(false);}catch(const document::Failure&){}
            CHECK(owner.state().ai==1&&owner.state().pending==0);
        }
        CHECK(owner.cancel_advisory(hex(1))==commerce::NativeCancelV1::already_delivered);
        cancelling_owner=&owner;commerce::NativeCommercialOwnerTestAccess::hook(cancel_before_delivery);
        try{owner.deliver_advisory(hex(201),"adjust.exposure","{\"milli_ev\":500}");CHECK(false);}catch(const document::Failure& e){CHECK(e.code==13);}
        CHECK(owner.state().ai==1&&owner.state().pending==0&&owner.cancel_advisory(hex(201))==commerce::NativeCancelV1::not_active);
        owner.dismiss_advisory(first.token);
        try{owner.deliver_advisory(hex(1),"adjust.exposure","{\"milli_ev\":2500}");CHECK(false);}catch(const document::Failure& e){CHECK(e.code==9);}
        CHECK(owner.state().ai==1);
        stale_root=root;commerce::NativeCommercialOwnerTestAccess::hook(stale);
        const auto fenced=owner.deliver_advisory(hex(2),"adjust.exposure","{\"milli_ev\":1000}");
        CHECK(blocked_writers==1&&owner.state().ai==2&&owner.state().pending==0);
        manual_edit(root); // Once the delivery fence is released, ordinary manual editing succeeds.
        try{owner.restore_advisory(fenced.token);CHECK(false);}catch(const document::Failure& e){CHECK(e.code==9);}
        CHECK(owner.state().ai==2);
        const auto third=owner.deliver_advisory(hex(3),"adjust.exposure","{\"milli_ev\":500}");(void)third;
        try{owner.deliver_advisory(hex(5),"adjust.exposure","{\"milli_ev\":500}");CHECK(false);}catch(const commerce::CommercialDenialV1& e){CHECK(e.reason==commerce::Admission::ai_limit);}
        commerce::NativeCommercialOwnerTestAccess::synthetic_store_fact(owner,{commerce::Entitlement::lifetime,true,false,0});
        auto paid=owner.deliver_advisory(hex(6),"adjust.exposure","{\"milli_ev\":500}");CHECK(!paid.plan.empty()&&owner.state().ai==3);
        commerce::NativeCommercialOwnerTestAccess::synthetic_store_fact(owner,{commerce::Entitlement::lifetime,true,true,0});
        CHECK(commerce::NativeCommercialOwner::recovery_allowed()&&owner.state().trial_start==-1);
    }
    {
        commerce::NativeCommercialOwner owner(root+"/concurrent.sqlite",root,reader,budget,test_clock,hex(702));std::atomic<unsigned> success{0},denied{0};std::vector<std::thread> workers;
        for(unsigned i=0;i<12;++i)workers.emplace_back([&,i]{try{owner.deliver_advisory(hex(300+i),"adjust.exposure","{\"milli_ev\":500}");++success;}catch(const commerce::CommercialDenialV1& e){CHECK(e.reason==commerce::Admission::ai_limit);++denied;}});
        for(auto& worker:workers){worker.join();}
        CHECK(success==3&&denied==9&&owner.state().ai==3&&owner.state().photos==1&&owner.state().pending==0);
    }
    const auto shared_ledger=root+"/photos.sqlite";
    for(unsigned i=0;i<6;++i){
        const auto path=root+"/photo-"+std::to_string(i);setup(path,i+1);auto photo=png(i+1);tiled::Budget b(134217728,photo.capacity());
        integration::OriginalReaderV2 decoded(photo.data(),photo.size(),b);commerce::NativeCommercialOwner owner(shared_ledger,path,decoded,b,test_clock,hex(800+i));
        const auto result=owner.admit_editable_source(hex(400+i));CHECK(result.decision==(i<5?commerce::Admission::allowed:commerce::Admission::photo_limit));
        CHECK(owner.state().photos==int(std::min(i+1,5u))&&owner.state().ai==0);
        storage::Repository recovery(path);CHECK(recovery.read().snapshot->revisions().size()==1); // Sixth source still opens for recovery.
    }
    {
        TestClock midnight_clock;midnight_clock.value.utc_seconds=20000LL*86400+86399;
        commerce::NativeCommercialOwner owner(root+"/midnight.sqlite",root,reader,budget,midnight_clock,hex(900));
        advancing_clock=&midnight_clock;commerce::NativeCommercialOwnerTestAccess::hook(midnight);
        auto delivered=owner.deliver_advisory(hex(900),"adjust.exposure","{\"milli_ev\":500}");CHECK(!delivered.plan.empty());
        const auto state=owner.state();CHECK(state.day==20001&&state.ai==1&&state.photos==1&&state.pending==0);
    }
    std::puts("native stored source/planner receipts, source dedup, stale rejection, dismissed replay, concurrent3cap, synthetic paid projection PASS; trial publication/store auth NOT qualified");return 0;
}
