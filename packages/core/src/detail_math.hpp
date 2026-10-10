#ifndef PIXAURA_DETAIL_MATH_HPP
#define PIXAURA_DETAIL_MATH_HPP
#include "working.hpp"
#include "document.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
namespace pixaura::detail {
template<class Check> void apply(decode::Vector<float>& pixels,uint32_t width,uint32_t height,
    bool sharpen,int32_t strength,decode::Vector<uint8_t>& scratch,Check&& check) {
    if(!strength)return;
    const std::size_t row=static_cast<std::size_t>(width)*16;
    if(scratch.size()<row*3)throw document::Failure{8};
    const auto copy_row=[&](uint32_t y){check();std::memcpy(scratch.data()+(y%3)*row,pixels.data()+static_cast<std::size_t>(y)*width*4,row);};
    copy_row(0);if(height>1)copy_row(1);
    const double t=static_cast<double>(strength)/1000;
    constexpr int weights[3]={1,2,1};
    for(uint32_t y=0;y<height;++y){
        check();
        for(uint32_t x=0;x<width;++x){
            if(x%128==0)check();
            float center[4];std::memcpy(center,scratch.data()+(y%3)*row+static_cast<std::size_t>(x)*16,16);
            auto* destination=pixels.data()+(static_cast<std::size_t>(y)*width+x)*4;
            if(sharpen&&center[3]==0){std::memcpy(destination,center,16);continue;}
            double g[4]={0,0,0,0};
            for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){
                const auto sy=static_cast<uint32_t>(std::max(0,std::min(static_cast<int>(height)-1,static_cast<int>(y)+dy)));
                const auto sx=static_cast<uint32_t>(std::max(0,std::min(static_cast<int>(width)-1,static_cast<int>(x)+dx)));
                float sample[4];std::memcpy(sample,scratch.data()+(sy%3)*row+static_cast<std::size_t>(sx)*16,16);
                const double weight=weights[dy+1]*weights[dx+1];
                for(unsigned c=0;c<4;++c)g[c]+=static_cast<double>(sample[c])*weight;
            }
            for(auto& channel:g)channel/=16;
            double result[4];
            for(unsigned c=0;c<3;++c){
                const double color=center[c];
                if(sharpen){if(!(g[3]>0))throw document::Failure{7};const double estimate=static_cast<double>(center[3])*(g[c]/g[3]);result[c]=color+t*(color-estimate);}
                else result[c]=(1-t)*color+t*g[c];
            }
            result[3]=sharpen?static_cast<double>(center[3]):(1-t)*static_cast<double>(center[3])+t*g[3];
            for(unsigned c=0;c<4;++c){if(!std::isfinite(result[c])||std::abs(result[c])>std::numeric_limits<float>::max())throw document::Failure{7};destination[c]=static_cast<float>(result[c]);}
            if(sharpen)std::memcpy(destination+3,center+3,4);
            else if(destination[3]==0)destination[0]=destination[1]=destination[2]=0;
        }
        if(y+2<height)copy_row(y+2);
    }
}
}
#endif
