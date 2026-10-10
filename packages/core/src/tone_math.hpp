#ifndef PIXAURA_TONE_MATH_HPP
#define PIXAURA_TONE_MATH_HPP
#include "tone.hpp"
#include "tone_constants.hpp"
#include "exposure_gain.hpp"
#include "document.hpp"
#include <array>
#include <cmath>
#include <limits>
namespace pixaura::tone {
using Triple=std::array<double,3>;
inline Triple transform(const double (&matrix)[3][3],const Triple& color){Triple result{};for(unsigned i=0;i<3;++i)result[i]=((matrix[i][0]*color[0])+(matrix[i][1]*color[1]))+(matrix[i][2]*color[2]);return result;}
inline Triple white(int32_t temperature){const double u=1.0/temperature;const double x=temperature<=7000?(((-4607000000.0*u)+2967800.0)*u+99.11)*u+0.244063:(((-2006400000.0*u)+1901800.0)*u+247.48)*u+0.237040;const double y=((-3.0*x)+2.87)*x-0.275;return {x/y,1.0,((1.0-x)-y)/y};}
class Kernel {
    const Spec* spec_;int32_t value_;double strength_=1;Triple ratio_{};
public:
    Kernel(const Spec& spec,int32_t value):spec_(&spec),value_(value){
        if(value==spec.neutral)return;
        if(spec.type=="pixaura.temperature"){const auto source=transform(bradford,white(value)),target=transform(bradford,white(6504));for(unsigned i=0;i<3;++i)ratio_[i]=target[i]/source[i];}
        else if(spec.type=="pixaura.brightness"||spec.type=="pixaura.saturation")strength_=static_cast<double>(value)/1000.0;
        else strength_=evaluation::exposure_gain(value);
    }
    bool neutral() const{return value_==spec_->neutral;}
    void apply(float* pixel) const {
        if(neutral()||pixel[3]==0)return;
        const double alpha=pixel[3];Triple c{pixel[0],pixel[1],pixel[2]},out{};
        if(spec_->type=="pixaura.brightness"){const double offset=alpha*strength_;for(unsigned i=0;i<3;++i)out[i]=c[i]+offset;}
        else if(spec_->type=="pixaura.contrast"){const double pivot=0.18*alpha;for(unsigned i=0;i<3;++i)out[i]=pivot+strength_*(c[i]-pivot);}
        else if(spec_->type=="pixaura.temperature"){auto lms=transform(bradford,transform(rgb_xyz,c));for(unsigned i=0;i<3;++i)lms[i]*=ratio_[i];out=transform(xyz_rgb,transform(bradford_inverse,lms));}
        else {
            const double y=c[0]==c[1]&&c[1]==c[2]?c[0]:((0.2126*c[0])+(0.7152*c[1]))+(0.0722*c[2]);
            if(spec_->type=="pixaura.saturation"){for(unsigned i=0;i<3;++i)out[i]=y+strength_*(c[i]-y);}
            else {const double l=y/alpha;const double selected=spec_->type=="pixaura.highlights"?(l-0.18)/0.82:l/0.18;const double t=selected<=0?0:selected>=1?1:selected;const double smooth=(t*t)*(3.0-(2.0*t));const double weight=spec_->type=="pixaura.highlights"?smooth:1.0-smooth;if(weight==0)return;const double factor=1.0+weight*(strength_-1.0);for(unsigned i=0;i<3;++i)out[i]=c[i]*factor;}
        }
        for(unsigned i=0;i<3;++i){if(!std::isfinite(out[i])||std::abs(out[i])>std::numeric_limits<float>::max())throw document::Failure{7};pixel[i]=static_cast<float>(out[i]);}
    }
};
}
#endif
