#include "../src/working.hpp"
#include "../src/srgb_table.hpp"
#include <iostream>
#include <cmath>
#include <cstring>
using namespace pixaura;
unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"working check "<<__LINE__<<": "<<#x<<'\n';std::exit(1);}}while(false)
template<class F> void error(int status,F fn){try{fn();CHECK(false);}catch(const document::Failure& f){CHECK(f.code==status);}}
decode::Metadata metadata(uint32_t w,uint32_t h,uint32_t o){decode::Metadata m;m.value.width=w;m.value.height=h;m.value.orientation=o;m.value.format=2;m.value.bit_depth=8;m.value.channels=4;decode::layout(m.value,decode::defaults());return m;}
int main(){auto l=working::defaults();
    const unsigned expected[8][6]={{0,1,2,3,4,5},{1,0,3,2,5,4},{5,4,3,2,1,0},{4,5,2,3,0,1},{0,2,4,1,3,5},{4,2,0,5,3,1},{5,3,1,4,2,0},{1,3,5,0,2,4}};
    uint8_t fixture[24]{};for(unsigned i=0;i<6;++i){fixture[i*4]=uint8_t(i*40);fixture[i*4+3]=255;}
    for(uint32_t o=1;o<=8;++o){const auto m=metadata(2,3,o);auto out=working::normalize(m,fixture,24,l);CHECK(out.metadata.width==(o>=5?3u:2u)&&out.metadata.height==(o>=5?2u:3u)&&out.metadata.orientation==1&&out.metadata.source_orientation==o);
        for(unsigned i=0;i<6;++i)CHECK((*out.pixels)[i*4]==working::srgb_table[fixture[expected[o-1][i]*4]]);
        const auto pass=working::identity(out,l);CHECK(*pass.pixels==*out.pixels&&pass.pixels.get()!=out.pixels.get());
    }
    for(unsigned i=0;i<256;++i){const double s=double(i)/255.0,linear=s<=0.04045?s/12.92:std::pow((s+0.055)/1.055,2.4);
        CHECK(std::abs(double(working::srgb_table[i])-linear)<3e-8);CHECK(std::abs(double(working::alpha_table[i])-s)<3e-8);
        uint8_t input[]={uint8_t(i),uint8_t(i),uint8_t(i),uint8_t(i)};auto out=working::normalize(metadata(1,1,1),input,4,l);
        CHECK(std::abs(double((*out.pixels)[0])-linear*s)<6e-8&&(*out.pixels)[0]<=(*out.pixels)[3]);
    }
    CHECK(working::srgb_table[0]==0&&working::srgb_table[255]==1);
    uint32_t seed=0x50495835;for(unsigned run=0;run<1024;++run){seed=seed*1664525+1013904223;const auto w=1+seed%17;seed=seed*1664525+1013904223;const auto h=1+seed%19,o=1+(seed>>16)%8;
        decode::Vector<uint8_t> pixels(w*h*4);for(unsigned i=0;i<w*h;++i){pixels[i*4]=uint8_t(i%256);pixels[i*4+1]=uint8_t(i/256);pixels[i*4+3]=255;}
        auto out=working::normalize(metadata(w,h,o),pixels.data(),pixels.size(),l);CHECK(out.pixels->size()==w*h*4);
        bool seen[323]{};for(unsigned i=0;i<w*h;++i){unsigned match=0;for(unsigned j=0;j<w*h;++j)if((*out.pixels)[i*4]==working::srgb_table[pixels[j*4]]&&(*out.pixels)[i*4+1]==working::srgb_table[pixels[j*4+1]]){CHECK(!seen[j]);seen[j]=true;++match;}CHECK(match==1);}
        CHECK(*working::identity(out,l).pixels==*out.pixels);
    }
    auto m=metadata(2,3,1);error(19,[&]{working::normalize(m,nullptr,24,l);});error(19,[&]{working::normalize(m,fixture,23,l);});error(19,[&]{working::normalize(m,fixture,25,l);});
    for(unsigned o:{0u,9u,UINT32_MAX}){m=metadata(2,3,1);m.value.orientation=o;error(17,[&]{working::normalize(m,fixture,24,l);});}
    m=metadata(2,3,1);m.value.row_stride=UINT64_MAX;error(17,[&]{working::normalize(m,fixture,24,l);});
    m=metadata(2,3,1);m.value.profile_type=1;m.value.profile_bytes=1;error(17,[&]{working::normalize(m,fixture,24,l);});m.profile.push_back(0);error(16,[&]{working::normalize(m,fixture,24,l);});
    m=metadata(2,3,1);m.srgb_compatible=false;error(16,[&]{working::normalize(m,fixture,24,l);});
    for(unsigned field=0;field<5;++field){auto small=l;if(field==0)small.width=1;if(field==1)small.height=2;if(field==2)small.pixel_count=5;if(field==3)small.row_stride=31;if(field==4)small.image_bytes=95;
        error(8,[&]{working::normalize(metadata(2,3,1),fixture,24,small);});}
    m=metadata(2,3,5);auto small=l;small.width=2;error(8,[&]{working::normalize(m,fixture,24,small);});
    m=metadata(2,3,1);m.value.width=UINT32_MAX;m.value.height=UINT32_MAX;m.value.orientation=5;m.value.display_width=UINT32_MAX;m.value.display_height=UINT32_MAX;m.value.row_stride=uint64_t(UINT32_MAX)*4;error(8,[&]{working::layout(m,l);});
    auto valid=working::normalize(metadata(2,3,1),fixture,24,l);valid.pixels->pop_back();error(19,[&]{working::identity(valid,l);});valid.pixels.reset();error(19,[&]{working::identity(valid,l);});
    CHECK(working::normalize(metadata(2,3,1),fixture,24,l).pixels->size()==24);
    std::cout<<"working checks "<<checks<<" passed; property seed 0x50495835 cases 1024\n";
}
