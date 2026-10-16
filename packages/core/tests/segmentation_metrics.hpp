#ifndef PIXAURA_SEGMENTATION_METRICS_HPP
#define PIXAURA_SEGMENTATION_METRICS_HPP
// Engineering-only v1; never compiled into production runtime.
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>
#include <stdexcept>
namespace pixaura::segmentation {
enum class Status { ok, version, shape, probability, mask, threshold, overflow, identity };
struct Error { Status status; };
inline void check(bool value, Status status){if(!value)throw Error{status};}
constexpr uint64_t max_pixels=65536;
struct Counts {uint64_t tp=0,fp=0,fn=0,tn=0;};
struct Ratio {uint64_t numerator,denominator;};
struct Metrics {Counts counts;Ratio iou,dice,accuracy,precision,recall;};
inline uint64_t add(uint64_t a,uint64_t b){check(a<=UINT64_MAX-b,Status::overflow);return a+b;}
inline Metrics scores(Counts c){
    const auto predicted=add(c.tp,c.fp),expected=add(c.tp,c.fn);
    const auto intersection_union=add(predicted,c.fn),total=add(intersection_union,c.tn);
    const auto twice=add(c.tp,c.tp),dice_den=add(predicted,expected);
    check(total&&total<=max_pixels,Status::shape);
    const Ratio perfect{1,1},zero{0,1};
    return {c,intersection_union?Ratio{c.tp,intersection_union}:perfect,
        dice_den?Ratio{twice,dice_den}:perfect,{add(c.tp,c.tn),total},
        predicted?Ratio{c.tp,predicted}:(expected?zero:perfect),
        expected?Ratio{c.tp,expected}:(predicted?zero:perfect)};
}
inline Metrics evaluate(uint32_t version,uint32_t width,uint32_t height,
    const uint8_t* expected,uint64_t expected_count,const float* probability,uint64_t count,float threshold=0.5f){
    check(version==1,Status::version);check(threshold==0.5f,Status::threshold);
    check(width&&height&&width<=256&&height<=256,Status::shape);
    const uint64_t pixels=uint64_t(width)*height;
    check(expected&&probability&&expected_count==pixels&&count==pixels&&pixels<=max_pixels,Status::shape);
    Counts c;
    for(uint64_t i=0;i<pixels;++i){
        check(expected[i]<=1,Status::mask);uint32_t bits=0;std::memcpy(&bits,probability+i,4);
        // Reject nonfinite and negative values by bits; negative zero is legal.
        check((bits&0x7fffffffu)<=0x3f800000u&&(!(bits>>31)||!(bits&0x7fffffffu)),Status::probability);
        const bool predicted=(bits&0x7fffffffu)>=0x3f000000u;
        if(expected[i]){if(predicted)++c.tp;else ++c.fn;}else{if(predicted)++c.fp;else ++c.tn;}
    }
    return scores(c);
}
constexpr std::array<const char*,12> scenes{{"rectangle","ellipse","thin","disconnected","border",
    "high-contrast","low-contrast","gradient","small","large","hole","alias-edge"}};
struct Scene {std::vector<uint8_t> expected;std::vector<float> rgb;};
inline Scene scene(uint32_t id,uint32_t size){
    check(id<scenes.size()&&(size==8||size==256),Status::shape);
    const size_t n=size_t(size)*size;Scene value{std::vector<uint8_t>(n),std::vector<float>(3*n)};
    for(uint32_t y=0;y<size;++y)for(uint32_t x=0;x<size;++x){
        const int64_t dx=2*int64_t(x)+1-size,dy=2*int64_t(y)+1-size;
        const bool box=x>=size/4&&x<3*size/4&&y>=size/4&&y<3*size/4;
        bool foreground=false;
        switch(id){
        case 0:case 5:case 6:case 7:foreground=box;break;
        case 1:foreground=dx*dx+dy*dy<=int64_t(size)*size/4;break;
        case 2:foreground=x==size/2||y==size/2;break;
        case 3:foreground=(x<size/4&&y<size/4)||(x>=3*size/4&&y>=3*size/4);break;
        case 4:foreground=x<size/2&&y<size/2;break;
        case 8:foreground=x==size/2&&y==size/2;break;
        case 9:foreground=x>0&&y>0&&x+1<size&&y+1<size;break;
        case 10:foreground=box&&!(x>=3*size/8&&x<5*size/8&&y>=3*size/8&&y<5*size/8);break;
        case 11:foreground=((x+y)%4)<2;break;
        }
        const size_t i=size_t(y)*size+x;value.expected[i]=foreground?1:0;
        // Deliberately separable colors except low contrast, gradient and small
        // subject. Ground truth above uses geometry, never color thresholding.
        float v=foreground?0.75f:0.25f;
        if(id==5)v=foreground?1.0f:0.0f;
        if(id==6)v=foreground?0.4375f:0.375f;
        if(id==7)v=foreground?0.75f:static_cast<float>(x)/static_cast<float>(size);
        if(id==8)v=0.25f;
        for(size_t channel=0;channel<3;++channel)value.rgb[channel*n+i]=v;
    }
    return value;
}
inline int metric_goldens(){
    int failed=0;
    const std::array<std::array<uint8_t,4>,6> truth{{{{1,0,1,0}},{{1,1,0,0}},{{1,1,0,0}},{{0,0,0,0}},{{1,1,0,0}},{{0,0,0,0}}}};
    const std::array<std::array<float,4>,6> predicted{{{{1,0,1,0}},{{0,0,1,1}},{{1,0,1,0}},{{1,0,0,0}},{{0,0,0,0}},{{0,0,0,0}}}};
    // Independently hand-counted TP/FP/FN/TN, then exact expected fractions.
    const std::array<Counts,6> counts{{{2,0,0,2},{0,2,2,0},{1,1,1,1},{0,1,0,3},{0,0,2,2},{0,0,0,4}}};
    const std::array<std::array<Ratio,5>,6> expected{{
        {{{1,1},{1,1},{1,1},{1,1},{1,1}}},{{{0,1},{0,1},{0,1},{0,1},{0,1}}},
        {{{1,3},{1,2},{1,2},{1,2},{1,2}}},{{{0,1},{0,1},{3,4},{0,1},{0,1}}},
        {{{0,1},{0,1},{1,2},{0,1},{0,1}}},{{{1,1},{1,1},{1,1},{1,1},{1,1}}}}};
    for(size_t i=0;i<truth.size();++i){const auto m=evaluate(1,2,2,truth[i].data(),4,predicted[i].data(),4);
        if(m.counts.tp!=counts[i].tp||m.counts.fp!=counts[i].fp||m.counts.fn!=counts[i].fn||m.counts.tn!=counts[i].tn)++failed;
        const std::array<Ratio,5> got{{m.iou,m.dice,m.accuracy,m.precision,m.recall}};
        for(size_t j=0;j<5;++j)if(got[j].numerator*expected[i][j].denominator!=expected[i][j].numerator*got[j].denominator)++failed;
    }
    auto rejects=[&](auto f,Status status){try{f();++failed;}catch(const Error& e){if(e.status!=status)++failed;}};
    const uint8_t mask[]={0};float v=0;
    rejects([&]{evaluate(2,1,1,mask,1,&v,1);},Status::version);
    rejects([&]{evaluate(1,1,1,mask,1,&v,1,0.4f);},Status::threshold);
    rejects([&]{evaluate(1,0,1,mask,1,&v,1);},Status::shape);
    rejects([&]{evaluate(1,UINT32_MAX,UINT32_MAX,mask,1,&v,1);},Status::shape);
    rejects([&]{evaluate(1,1,1,mask,0,&v,1);},Status::shape);
    rejects([&]{evaluate(1,1,1,mask,1,nullptr,0);},Status::shape);
    const uint8_t bad[]={2};rejects([&]{evaluate(1,1,1,bad,1,&v,1);},Status::mask);
    for(auto bits:{0xbf000000u,0x3f800001u,0x7f800000u,0x7fc00000u,0x7f800001u}){
        std::memcpy(&v,&bits,4);rejects([&]{evaluate(1,1,1,mask,1,&v,1);},Status::probability);}
    rejects([&]{scores({UINT64_MAX,1,0,0});},Status::overflow);
    rejects([&]{scores({UINT64_MAX/2+1,0,0,0});},Status::overflow);
    v=0.5f;if(evaluate(1,1,1,mask,1,&v,1).counts.fp!=1)++failed;
    const uint32_t below=0x3effffffu;std::memcpy(&v,&below,4);if(evaluate(1,1,1,mask,1,&v,1).counts.tn!=1)++failed;
    return failed;
}
}
#endif
