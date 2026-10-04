#include "sha256.hpp"
#include "document.hpp"
#include <algorithm>
#include <cstring>
namespace pixaura::storage {
namespace {
constexpr uint32_t k[] = {
0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
uint32_t r(uint32_t x, unsigned n) { return (x >> n) | (x << (32 - n)); }
}
void Sha256::compress() {
    uint32_t w[64];
    for (std::size_t i=0;i<16;++i) w[i]=(uint32_t(block_[i*4])<<24)|(uint32_t(block_[i*4+1])<<16)|(uint32_t(block_[i*4+2])<<8)|block_[i*4+3];
    for (std::size_t i=16;i<64;++i) w[i]=w[i-16]+(r(w[i-15],7)^r(w[i-15],18)^(w[i-15]>>3))+w[i-7]+(r(w[i-2],17)^r(w[i-2],19)^(w[i-2]>>10));
    auto a=state_[0],b=state_[1],c=state_[2],d=state_[3],e=state_[4],f=state_[5],g=state_[6],h=state_[7];
    for (std::size_t i=0;i<64;++i) {
        const auto t1=h+(r(e,6)^r(e,11)^r(e,25))+((e&f)^(~e&g))+k[i]+w[i];
        const auto t2=(r(a,2)^r(a,13)^r(a,22))+((a&b)^(a&c)^(b&c));
        h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
    }
    state_[0]+=a;state_[1]+=b;state_[2]+=c;state_[3]+=d;state_[4]+=e;state_[5]+=f;state_[6]+=g;state_[7]+=h;
}
void Sha256::update(const uint8_t* data, std::size_t size) {
    if (size > UINT64_MAX/8 - bytes_) throw document::Failure{8};
    bytes_+=size;
    while (size) {
        const auto n=std::min(size,64-used_);std::memcpy(block_.data()+used_,data,n);used_+=n;data+=n;size-=n;
        if (used_==64) {compress();used_=0;}
    }
}
document::String Sha256::finish() {
    const auto bits=bytes_*8;block_[used_++]=0x80;
    if (used_>56) {std::fill(block_.begin()+static_cast<std::ptrdiff_t>(used_),block_.end(),uint8_t{0});compress();used_=0;}
    std::fill(block_.begin()+static_cast<std::ptrdiff_t>(used_),block_.begin()+56,uint8_t{0});
    for (unsigned i=0;i<8;++i) block_[63-i]=static_cast<uint8_t>(bits>>(i*8));
    compress();document::String result(64,'0');constexpr char hex[]="0123456789abcdef";
    for (unsigned i=0;i<32;++i) {const auto byte=static_cast<uint8_t>(state_[i/4]>>(24-8*(i%4)));result[i*2]=hex[byte>>4];result[i*2+1]=hex[byte&15];}
    return result;
}
}
