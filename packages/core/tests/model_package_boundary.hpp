#ifndef PIXAURA_MODEL_PACKAGE_BOUNDARY_HPP
#define PIXAURA_MODEL_PACKAGE_BOUNDARY_HPP
#include <cstdio>
#include "model_package_fixtures.hpp"
namespace pixaura::model {
inline int boundary(){
    int failures=0;
    for(auto c:fixtures::cases()){
        Key key{7,2,1,1,fixtures::key};
        if(c.key_mode==1)key.bytes={0x3d,0x40,0x17,0xc3,0xe8,0x43,0x89,0x5a,0x92,0xb7,0x0a,0xa7,0x4d,0x1b,0x7e,0xbc,0x9c,0x98,0x2c,0xcf,0x2e,0xc4,0x96,0x8c,0xc0,0xcd,0x55,0xf1,0x2a,0xf4,0x66,0x0c};
        if(c.key_mode==2)key.bytes.fill(255);
        if(c.key_mode==3){key.bytes.fill(0);key.bytes[0]=1;}
        if(c.key_mode==4){key.bytes.fill(255);key.bytes[0]=0xed;key.bytes[31]=0x7f;}
        std::vector<File> files{{"manifest.pxm",c.manifest}};
        if(!c.signature.empty())files.push_back({"signature.pxs",c.signature});
        if(!c.missing)files.push_back({c.path,c.payload});
        if(c.extra)files.push_back({"extra.exe",{1}});
        auto* output=reinterpret_cast<pixaura_model_package*>(uintptr_t{1});
        pixaura_model_diagnostic diagnostic{};
        const auto result=verify_owned(1,std::move(files),{1,1,c.production?1u:2u,&key,1},268435456,&output,&diagnostic);
        if(result!=static_cast<int32_t>(c.reason)||diagnostic.reason!=c.reason)++failures;
        if(!result){pixaura_model_info info{};if(pixaura_model_package_info(1,output,&info)||info.file_count!=1||info.model_version!=1)++failures;pixaura_model_package_release(output);}
        else if(output!=reinterpret_cast<pixaura_model_package*>(uintptr_t{1}))++failures;
    }
    // Deterministic bounded mutations retain full canonical verification. No
    // mutation is allowed to create a verified package or alter output ownership.
    const auto valid=fixtures::cases().front();uint32_t seed=0x705ec001u;
    for(unsigned region=0;region<3;++region){for(unsigned iteration=0;iteration<32;++iteration){
        auto manifest=valid.manifest,signature_bytes=valid.signature,payload=valid.payload;
        auto& bytes=region==0?manifest:region==1?signature_bytes:payload;
        seed=seed*1664525u+1013904223u;bytes[seed%bytes.size()]^=static_cast<uint8_t>(1u<<(iteration%8));
        Key key{7,2,1,1,fixtures::key};pixaura_model_package* output=nullptr;pixaura_model_diagnostic diagnostic{};
        const auto result=verify_owned(1,{{"manifest.pxm",manifest},{"signature.pxs",signature_bytes},{valid.path,payload}},
            {1,1,2,&key,1},268435456,&output,&diagnostic);
        if(!result||output){++failures;if(output){pixaura_model_package_release(output);}}
    }}
    std::printf("Package bounded mutation corpus v1: seed=0x705ec001 cases=96 failures=%d\n",failures);
    // Independent RFC 8032 section 7.1 vector 1 (empty message), not our generator.
    const uint8_t signature[64]={0xe5,0x56,0x43,0x00,0xc3,0x60,0xac,0x72,0x90,0x86,0xe2,0xcc,0x80,0x6e,0x82,0x8a,0x84,0x87,0x7f,0x1e,0xb8,0xe5,0xd9,0x74,0xd8,0x73,0xe0,0x65,0x22,0x49,0x01,0x55,0x5f,0xb8,0x82,0x15,0x90,0xa3,0x3b,0xac,0xc6,0x1e,0x39,0x70,0x1c,0xf9,0xb4,0x6b,0xd2,0x5b,0xf5,0xf0,0x59,0x5b,0xbe,0x24,0x65,0x51,0x41,0x43,0x8e,0x7a,0x10,0x0b};
    const uint8_t empty[1]={0};
    if(crypto_sign_ed25519_verify_detached(signature,empty,0,fixtures::key.data()))++failures;
    auto modified=std::array<uint8_t,64>{};std::copy_n(signature,64,modified.begin());modified[0]^=1;
    if(!crypto_sign_ed25519_verify_detached(modified.data(),empty,0,fixtures::key.data()))++failures;
    return failures;
}
}
#endif
