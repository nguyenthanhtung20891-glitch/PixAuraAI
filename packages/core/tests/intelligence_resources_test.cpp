#include "../src/intelligence_resources.hpp"
#include <cstdlib>
#include <cstdio>
#include <thread>
using namespace pixaura::intelligence;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x))std::abort();}while(false)
namespace {
std::mutex gate;std::condition_variable cv;
bool paused=false,entered=false,released=false,fail_load=false,allocation_failure=false;
unsigned loads=0,destroys=0,runs=0;
class Fixture final:public Runtime {
public:
    ~Fixture()override{++destroys;}
    void load()override{++loads;if(fail_load)throw 0;}
    void run(const std::atomic<bool>& cancellation)override {
        if(allocation_failure)throw std::bad_alloc();
        ++runs;std::unique_lock<std::mutex> lock(gate);entered=true;cv.notify_all();
        if(paused)cv.wait(lock,[&]{return released;});
        (void)cancellation.load();
    }
};
std::unique_ptr<Runtime> factory(){return std::make_unique<Fixture>();}
void pause(){std::lock_guard<std::mutex> lock(gate);paused=true;entered=false;released=false;}
void await(){std::unique_lock<std::mutex> lock(gate);cv.wait(lock,[]{return entered;});}
void resume(){std::lock_guard<std::mutex> lock(gate);released=true;paused=false;cv.notify_all();}
}
int main(){
    auto& owner=Owner::shared();CHECK(&owner==&Owner::shared());CHECK(owner.phase()==Phase::unloaded);CHECK(loads==0);
    Costs costs{10,20,3,4,5,6,7,8};Observation facts{200,30,40,1,true,false,true,UINT64_MAX};
    uint64_t total=0;CHECK(sum(costs,total)&&total==63);CHECK(fits(costs,facts));
    auto bad=costs;bad.mapping=UINT64_MAX;CHECK(!sum(bad,total));
    CHECK(!fits(Costs{},facts));
    auto diagnostic=facts;diagnostic.process_rss=0;CHECK(fits(costs,diagnostic)==fits(costs,facts));
    auto unknown=facts;unknown.known=false;CHECK(!fits(costs,unknown));
    unknown=facts;unknown.pressure=true;CHECK(!fits(costs,unknown));
    unknown=facts;unknown.foreground=false;CHECK(!fits(costs,unknown));
    unknown=facts;unknown.manual_reserve=0;CHECK(!fits(costs,unknown));
    unknown=facts;unknown.platform_reserve=0;CHECK(!fits(costs,unknown));
    unknown=facts;unknown.platform_reserve=UINT64_MAX;CHECK(!fits(costs,unknown));
    unknown=facts;unknown.available=100;CHECK(!fits(costs,unknown));
    unknown=facts;unknown.sequence=0;CHECK(!fits(costs,unknown));
    Client editor1,editor2;CHECK(owner.request(editor1,1,costs,unknown,factory)==Status::resource);CHECK(loads==0);
    CHECK(owner.request(editor1,1,costs,facts,factory)==Status::success);CHECK(loads==1&&runs==1);
    CHECK(owner.request(editor2,1,costs,facts,factory)==Status::success);CHECK(loads==1&&runs==2);
    bad=costs;bad.session=1;CHECK(owner.request(editor1,1,bad,facts,factory)==Status::resource);
    pause();Status result=Status::failed;std::thread work([&]{result=owner.request(editor1,1,costs,facts,factory);});await();
    CHECK(owner.phase()==Phase::running);CHECK(owner.request(editor2,1,costs,facts,factory)==Status::busy);
    owner.cancel();resume();work.join();CHECK(result==Status::cancelled);
    CHECK(owner.request(editor2,1,costs,facts,factory)==Status::success); // nonsticky cancellation
    pause();std::thread pressured([&]{result=owner.request(editor2,1,costs,facts,factory);});await();
    auto pressure=facts;pressure.sequence=2;pressure.pressure=true;
    CHECK(owner.observe(pressure)==Status::success);resume();pressured.join();CHECK(result==Status::cancelled);
    CHECK(owner.request(editor2,1,costs,facts,factory)==Status::stale);
    auto conflict=pressure;conflict.pressure=false;CHECK(owner.observe(conflict)==Status::stale);
    facts.sequence=3;CHECK(owner.request(editor2,1,costs,facts,factory)==Status::success);
    pause();std::thread old([&]{result=owner.request(editor1,1,costs,facts,factory);});await();
    editor1.abandon();resume();old.join();CHECK(result==Status::stale); // editor/source/mode departure
    CHECK(owner.request(editor1,1,costs,facts,factory)==Status::stale);
    pause();std::thread stopping([&]{result=owner.request(editor2,1,costs,facts,factory);});await();owner.stop();
    CHECK(owner.request(editor1,2,costs,facts,factory)==Status::unavailable);resume();stopping.join();CHECK(result==Status::cancelled);
    owner.unload();CHECK(destroys==1&&owner.phase()==Phase::unloaded);
    fail_load=true;CHECK(owner.request(editor2,1,costs,facts,factory)==Status::failed);CHECK(destroys==2);fail_load=false;
    for(unsigned i=0;i<3;++i){CHECK(owner.request(editor2,1,costs,facts,factory)==Status::success);owner.unload();}
    CHECK(owner.phase()==Phase::unloaded);CHECK(loads==5&&destroys==5);
    // Unload while a request is active must join it before destroying its session.
    pause();std::thread last([&]{result=owner.request(editor2,1,costs,facts,factory);});await();owner.stop();
    std::thread teardown([&]{owner.unload();});resume();last.join();teardown.join();
    CHECK(result==Status::cancelled&&destroys==6&&owner.phase()==Phase::unloaded);
    allocation_failure=true;CHECK(owner.request(editor2,1,costs,facts,factory)==Status::resource);
    CHECK(destroys==7);allocation_failure=false;owner.unload();
    owner.shutdown();CHECK(owner.request(editor2,1,costs,facts,factory)==Status::unavailable);
    owner.unload();CHECK(owner.phase()==Phase::stopped);
    std::printf("Intelligence resource foundation: %u checks; no model/editor/approval/history capability\n",checks);
}
