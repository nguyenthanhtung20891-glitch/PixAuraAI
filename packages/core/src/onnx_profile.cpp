#include "onnx_profile.hpp"
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <string_view>
namespace pixaura::inference {
namespace {
constexpr int32_t invalid=PIXAURA_INFERENCE_MODEL_INVALID,unsupported=PIXAURA_INFERENCE_MODEL_UNSUPPORTED;
struct Span { const uint8_t* p;size_t n; };
struct Field { uint32_t id,wire;uint64_t value;Span bytes; };
struct Message {
    std::array<Field,128> fields{};size_t count=0;
    uint64_t varint(Span s,size_t& at){
        uint64_t v=0;for(unsigned i=0;i<10;++i){require(at<s.n,invalid);const auto c=s.p[at++];
            require(i!=9||c<=1,invalid);v|=uint64_t(c&127)<<(i*7);
            if(!(c&128)){require(!i||c!=0,invalid);return v;}}
        throw Error{invalid};
    }
    explicit Message(Span s){size_t at=0;while(at<s.n){require(count<fields.size(),PIXAURA_INFERENCE_RESOURCE);
        const auto key=varint(s,at);require(key>=8&&key<=UINT32_MAX,invalid);
        Field f{static_cast<uint32_t>(key>>3),static_cast<uint32_t>(key&7),0,{nullptr,0}};
        if(f.wire==0)f.value=varint(s,at);
        else if(f.wire==2){const auto n=varint(s,at);require(n<=s.n-at,invalid);f.bytes={s.p+at,static_cast<size_t>(n)};at+=static_cast<size_t>(n);}
        else throw Error{unsupported};
        fields[count++]=f;
    }}
    void allowed(std::initializer_list<uint32_t> ids)const{
        for(size_t i=0;i<count;++i){bool found=false;for(auto id:ids){found=found||id==fields[i].id;}require(found,unsupported);}
    }
    size_t number(uint32_t id)const{size_t n=0;for(size_t i=0;i<count;++i){n+=fields[i].id==id;}return n;}
    const Field& one(uint32_t id,uint32_t wire)const{
        require(number(id)==1,invalid);for(size_t i=0;i<count;++i)if(fields[i].id==id){require(fields[i].wire==wire,invalid);return fields[i];}
        throw Error{invalid};
    }
    Span bytes(uint32_t id)const{return one(id,2).bytes;}
    uint64_t integer(uint32_t id)const{return one(id,0).value;}
    void text(uint32_t id,std::string_view value)const{const auto b=bytes(id);require(b.n==value.size()&&!std::memcmp(b.p,value.data(),b.n),unsupported);}
};
std::array<uint32_t,4> shape(Span bytes){
    Message s(bytes);s.allowed({1});require(s.number(1)==4,unsupported);std::array<uint32_t,4> out{};size_t i=0;
    for(size_t j=0;j<s.count;++j){require(s.fields[j].wire==2,invalid);Message d(s.fields[j].bytes);d.allowed({1});
        const auto n=d.integer(1);require(n&&n<=1024,unsupported);out[i++]=static_cast<uint32_t>(n);}
    uint64_t elements=1;for(auto d:out){require(elements<=4194304/d,unsupported);elements*=d;}return out;
}
std::array<uint32_t,4> tensor_type(Span bytes){
    Message type(bytes);type.allowed({1});Message tensor(type.bytes(1));tensor.allowed({1,2});
    require(tensor.integer(1)==1,unsupported);return shape(tensor.bytes(2));
}
std::array<uint32_t,4> value_info(Span bytes,std::string_view name){
    Message v(bytes);v.allowed({1,2});v.text(1,name);return tensor_type(v.bytes(2));
}
std::array<float,3> initializer(Span bytes){
    Message t(bytes);t.allowed({1,2,8,9});require(t.number(1)==4,unsupported);
    const std::array<uint64_t,4> dims{{1,3,1,1}};size_t i=0;
    for(size_t j=0;j<t.count;++j)if(t.fields[j].id==1){require(t.fields[j].wire==0&&t.fields[j].value==dims[i++],unsupported);}
    require(t.integer(2)==1,unsupported);t.text(8,"weights");const auto raw=t.bytes(9);require(raw.n==12,invalid);
    static_assert(sizeof(float)==4&&std::numeric_limits<float>::is_iec559,"IEEE float");
    std::array<float,3> out{};double sum=0;
    for(size_t k=0;k<3;++k){const auto* p=raw.p+k*4;const uint32_t bits=uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
        require(bits==0||bits==0x3e800000||bits==0x3f000000||bits==0x3f400000||bits==0x3f800000,unsupported);
        std::memcpy(&out[k],&bits,4);sum+=out[k];}
    require(sum==1,unsupported);return out;
}
void node(Span bytes){
    Message n(bytes);n.allowed({1,2,4});require(n.number(1)==2,unsupported);size_t i=0;
    const std::array<std::string_view,2> names{{"rgb","weights"}};
    for(size_t j=0;j<n.count;++j)if(n.fields[j].id==1){const auto& f=n.fields[j];const auto name=names[i++];require(f.wire==2&&f.bytes.n==name.size()&&!std::memcmp(f.bytes.p,name.data(),name.size()),unsupported);}
    n.text(2,"probability");n.text(4,"Conv");
}
}
Plan parse_onnx(const uint8_t* bytes,size_t size){
    require(bytes&&size,invalid);require(size<=4096,PIXAURA_INFERENCE_RESOURCE);
    Message m({bytes,size});m.allowed({1,7,8});require(m.integer(1)==9,unsupported);
    Message opset(m.bytes(8));opset.allowed({2});require(opset.integer(2)==13,unsupported);
    Message g(m.bytes(7));g.allowed({1,2,5,11,12});g.text(2,"pixaura-reference-v1");
    require(g.number(1)==1&&g.number(5)==1&&g.number(11)==1&&g.number(12)==1,unsupported);
    node(g.bytes(1));Plan p{};p.weights=initializer(g.bytes(5));p.input=value_info(g.bytes(11),"rgb");p.output=value_info(g.bytes(12),"probability");
    require(p.input[0]==1&&p.input[1]==3&&p.output[0]==1&&p.output[1]==1&&p.input[2]==p.output[2]&&p.input[3]==p.output[3],unsupported);return p;
}
bool valid_probabilities(const float* p,size_t n){for(size_t i=0;i<n;++i){uint32_t b=0;std::memcpy(&b,p+i,4);
    if((b&0x7fffffffu)>0x3f800000u||((b>>31)&&(b&0x7fffffffu)))return false;}return true;}
double binary32_value(float value){uint32_t b=0;std::memcpy(&b,&value,4);const auto e=(b>>23)&255u,m=b&0x7fffffu;
    if(!e)return std::ldexp(static_cast<double>(m),-149);
    return std::ldexp(static_cast<double>(m|0x800000u),static_cast<int>(e)-150);
}
float binary32_round(double value){
    static_assert(sizeof(double)==8&&std::numeric_limits<double>::is_iec559,"IEEE double");
    uint64_t b=0;std::memcpy(&b,&value,8);const auto raw=static_cast<uint32_t>((b>>52)&2047u);uint32_t result=0;
    if(raw){const auto exponent=static_cast<int>(raw)-1023;const auto significand=(b&UINT64_C(0xfffffffffffff))|UINT64_C(0x10000000000000);
        const auto shift=exponent>=-126?29:-exponent-97;
        uint64_t rounded=0;if(shift<=53){const auto quotient=significand>>shift,remainder=significand&((UINT64_C(1)<<shift)-1),half=UINT64_C(1)<<(shift-1);
            rounded=quotient+(remainder>half||(remainder==half&&(quotient&1)));}
        if(exponent>=-126)result=static_cast<uint32_t>((static_cast<uint64_t>(exponent+127)<<23)+(rounded-0x800000u));
        else result=static_cast<uint32_t>(rounded);
    }
    float out=0;std::memcpy(&out,&result,4);return out;
}
}
