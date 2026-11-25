#ifndef PIXAURA_CERTIFICATION_V2_HPP
#define PIXAURA_CERTIFICATION_V2_HPP
#ifdef PIXAURA_CERTIFICATION_TESTING
#include "../../document.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#include <cstdint>
namespace pixaura::editor {
// Debug-only instrumentation of the REAL pipeline. Never supplies a candidate,
// approval, native receipt or successful result. Absent from release builds.
struct CertificationV2 {
    std::atomic<uint32_t> point{UINT32_MAX},reached{0},resume{0},fault{0},active{0};
    int32_t control(uint32_t command,uint32_t value,uint32_t points,uint32_t faults){
        if(command==1)return reached.load()?1:0;
        if(command==2){resume=1;return 0;}
        if(active)return 8;
        if(command==0){if(value>points&&value!=UINT32_MAX)return 1;point=value;reached=0;resume=0;return 0;}
        if(command==3){if(value>faults)return 1;fault=value;return 0;}
        return 1;
    }
    void checkpoint(uint32_t where,unsigned seconds=15){
        if(point!=where)return;
        reached=1;
        const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(seconds);
        while(!resume){if(std::chrono::steady_clock::now()>=deadline)throw document::Failure{14};std::this_thread::yield();}
    }
    struct Run {CertificationV2& owner;explicit Run(CertificationV2& value):owner(value){uint32_t idle=0;if(!owner.active.compare_exchange_strong(idle,1))throw document::Failure{8};}~Run(){owner.active=0;}};
};
}
#endif
#endif
