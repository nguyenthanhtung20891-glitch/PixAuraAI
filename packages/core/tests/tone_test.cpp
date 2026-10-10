#include "../src/evaluation.hpp"
#include "tone_fixtures.hpp"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <string>
#define CHECK(x) do{if(!(x))throw std::runtime_error("line "+std::to_string(__LINE__)+": " #x);}while(false)
using namespace pixaura;
float number(unsigned bits){float value;std::memcpy(&value,&bits,4);return value;}
unsigned bits(float value){unsigned result;std::memcpy(&result,&value,4);return result;}
document::String request(const char* tool,const char* parameter,int value,int version=1) {
    const std::string text=std::string("{\"operations\":[{\"id\":\"00000000000000000000000000000011\",\"type\":\"pixaura.")+tool+"\",\"operation_version\":"+std::to_string(version)+",\"parameter_version\":1,\"parameters\":{\""+parameter+"\":"+std::to_string(value)+"}}]}";
    return document::String(text);
}
const char* parameter(const char* tool){return std::strcmp(tool,"brightness")==0?"milli_linear":std::strcmp(tool,"contrast")==0?"milli_stops":std::strcmp(tool,"saturation")==0?"milli_ratio":std::strcmp(tool,"temperature")==0?"kelvin":"milli_ev";}
working::Image source(const unsigned* input){working::Image image;image.metadata={1,sizeof(pixaura_working_metadata),1,1,1,1,1,0,16,16};image.pixels=std::make_unique<decode::Vector<float>>();for(unsigned i=0;i<4;++i)image.pixels->push_back(number(input[i]));return image;}
int main(){try{
    const auto limits=working::defaults();
    unsigned count=0;
    for(const auto& fixture:tone_fixtures){const auto stack=evaluation::parse(request(fixture.tool,parameter(fixture.tool),fixture.parameter));auto original=source(fixture.input);auto result=evaluation::evaluate(original,stack,limits);
        const auto neutral=std::strcmp(fixture.tool,"saturation")==0?1000:std::strcmp(fixture.tool,"temperature")==0?6504:0;
        for(unsigned i=0;i<4;++i){CHECK(bits((*original.pixels)[i])==fixture.input[i]);if(i==3||fixture.parameter==neutral||fixture.input[3]==0)CHECK(bits((*result.pixels)[i])==fixture.input[i]);else {const auto expected=number(fixture.expected[i]);CHECK(std::abs(double((*result.pixels)[i])-expected)<=1e-7+2e-6*std::abs(double(expected)));}}++count;}
    for(const auto* tool:{"brightness","contrast","highlights","shadows","saturation","temperature"}){
        const int low=std::strcmp(tool,"brightness")==0?-1000:std::strcmp(tool,"saturation")==0?0:std::strcmp(tool,"temperature")==0?4000:-2000;
        const int high=std::strcmp(tool,"brightness")==0?1000:std::strcmp(tool,"temperature")==0?25000:2000;
        for(const auto value:{low-1,high+1})CHECK(document::parse_evaluation(request(tool,parameter(tool),value)).code==7);
        CHECK(document::parse_evaluation(request(tool,parameter(tool),low,2)).code==5);
        for(const char* invalid:{"0.1","1e0","\"1\"","null","true","-0"}){auto malformed=request(tool,parameter(tool),low);const auto at=malformed.find(std::to_string(low),malformed.find("parameters"));CHECK(at!=document::String::npos);malformed.replace(at,std::to_string(low).size(),invalid);CHECK(document::parse_evaluation(malformed).code!=0);}
    }
    const unsigned input[]={0x3e800000,0x3f000000,0x3f400000,0x3f800000};
    auto original=source(input);
    auto compose=[&](const document::String& first,const document::String& second){auto a=evaluation::parse(first),b=evaluation::parse(second);b[0].id=document::Id::parse("00000000000000000000000000000012");a.push_back(b[0]);auto combined=evaluation::evaluate(original,a,limits);auto one=evaluation::evaluate(original,evaluation::parse(first),limits);auto sequential=evaluation::evaluate(one,b,limits);CHECK(std::memcmp(combined.pixels->data(),sequential.pixels->data(),16)==0);return combined;};
    auto ec=compose(request("exposure","milli_ev",1000),request("contrast","milli_stops",1000));
    auto ce=compose(request("contrast","milli_stops",1000),request("exposure","milli_ev",1000));CHECK(std::memcmp(ec.pixels->data(),ce.pixels->data(),12)!=0);
    auto ts=compose(request("temperature","kelvin",4000),request("saturation","milli_ratio",0));auto st=compose(request("saturation","milli_ratio",0),request("temperature","kelvin",4000));CHECK(std::memcmp(ts.pixels->data(),st.pixels->data(),12)!=0);
    compose(request("brightness","milli_linear",500),request("highlights","milli_ev",1000));compose(request("highlights","milli_ev",1000),request("shadows","milli_ev",-1000));compose(request("contrast","milli_stops",1000),request("contrast","milli_stops",1000));
    auto stack=evaluation::parse(request("brightness","milli_linear",500));auto neutral=evaluation::parse(request("temperature","kelvin",6504));neutral[0].id=document::Id::parse("00000000000000000000000000000013");stack.push_back(neutral[0]);auto with_neutral=evaluation::evaluate(original,stack,limits);auto without=evaluation::evaluate(original,evaluation::parse(request("brightness","milli_linear",500)),limits);CHECK(std::memcmp(with_neutral.pixels->data(),without.pixels->data(),16)==0);
    std::printf("Independent tone/reference corpus PASS %u fixtures; integer boundaries/version/neutral/alpha/source invariance PASS\n",count);return 0;
}catch(const document::Failure& failure){std::fprintf(stderr,"tone FAIL status=%d\n",failure.code);return 1;}catch(const std::exception& failure){std::fprintf(stderr,"tone FAIL %s\n",failure.what());return 1;}}
