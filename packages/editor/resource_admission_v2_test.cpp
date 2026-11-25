#include "../core/src/v2/editor/resource_admission_v2.hpp"
#include <stdexcept>
#include <thread>
#include <cstdio>
using namespace pixaura;
#define CHECK(x) do{if(!(x))throw std::runtime_error("resource admission line "+std::to_string(__LINE__));}while(false)
template<class F>void fails(int code,F call){bool failed=false;try{call();}catch(const document::Failure& e){CHECK(e.code==code);failed=true;}CHECK(failed);}
int main(){try{
    // Byte-sized synthetic accounting, NOT physical memory or a new product cap.
    auto budget=std::make_shared<tiled::Budget>(128,0);
    auto control=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget->reserve(1)));
    auto source=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget->reserve(16)));
    auto owner=std::make_shared<editor::ResourceAdmissionV2>(budget,128,128);
    auto existing=owner->work();existing->admit();existing->admit();CHECK(owner->observation().active_jobs==1);
    owner->update(128);CHECK(!owner->observation().pending);
    auto concurrent=owner->work();concurrent->admit();CHECK(owner->observation().active_jobs==2);concurrent.reset();
    auto scratch=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget->reserve(8)));
    std::thread pressure([&]{owner->update(32);});pressure.join();CHECK(owner->observation().pending);
    auto denied=owner->work();fails(8,[&]{denied->admit();});fails(15,[&]{owner->close();});
    scratch.reset();existing.reset();auto facts=owner->observation();
    CHECK(!facts.pending&&facts.active_jobs==0&&facts.allocated_reservation==17&&facts.withheld_capacity==96);
    auto allowed=owner->work();allowed->admit();auto frame=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget->reserve(15)));
    fails(8,[&]{auto excess=budget->reserve(1);(void)excess;});
    CHECK(owner->observation().allocated_reservation==32&&budget->peak()==128);
    allowed.reset();frame.reset();source.reset();owner->update(128);CHECK(owner->observation().withheld_capacity==0);
    auto large=std::unique_ptr<tiled::Budget::Lease>(new tiled::Budget::Lease(budget->reserve(64)));
    owner->update(32);facts=owner->observation();CHECK(facts.allocated_reservation==65&&facts.withheld_capacity==63);
    auto blocked=owner->work();fails(8,[&]{blocked->admit();});large.reset();
    auto recovered=owner->work();recovered->admit();facts=owner->observation();CHECK(facts.allocated_reservation==1&&facts.withheld_capacity==96);
    recovered.reset();owner->close();fails(9,[&]{auto stopped=owner->work();stopped->admit();});
    owner.reset();control.reset();CHECK(budget->inflight()==0);
    std::puts("versioned resource admission: pressure/quiescence/exact lower cap/retained owners/recovery/close PASS");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}catch(...){std::fputs("unexpected native failure\n",stderr);return 2;}}
