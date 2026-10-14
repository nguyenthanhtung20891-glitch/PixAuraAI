#include "pixaura/model_package.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include "model_package_fixtures.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <cstring>
#endif
extern "C" int32_t pixaura_model_test_boundary();
int main(int argc,char** argv){
    int n=pixaura_model_test_boundary();
    namespace fs=std::filesystem;
    const auto parent=fs::absolute(argc>1?argv[1]:".").make_preferred();const auto root=parent/"model-package-test";
    fs::create_directories(root); // Dedicated corpus directory under caller's build root.
    auto write=[&](const char* name,const std::vector<uint8_t>& b){std::ofstream out(root/name,std::ios::binary|std::ios::trunc);out.write(reinterpret_cast<const char*>(b.data()),static_cast<std::streamsize>(b.size()));if(!out)++n;};
    const auto valid=pixaura::model::fixtures::cases().front();
    write("manifest.pxm",valid.manifest);write("signature.pxs",valid.signature);write("model.onnx",valid.payload);
    auto verify=[&](uint32_t expected){auto* p=reinterpret_cast<pixaura_model_package*>(uintptr_t{1});pixaura_model_diagnostic d{};const auto path=root.u8string();const auto result=pixaura_model_package_verify(1,path.data(),path.size(),268435456,&p,&d);if(result!=static_cast<int32_t>(expected)||d.reason!=expected||p!=reinterpret_cast<pixaura_model_package*>(uintptr_t{1}))++n;};
    verify(PIXAURA_MODEL_TRUST_DOMAIN);
    fs::create_directory(root/"unexpected");verify(PIXAURA_MODEL_UNSAFE_FILE);fs::remove(root/"unexpected");
    fs::create_hard_link(root/"model.onnx",root/"alias.onnx");verify(PIXAURA_MODEL_UNSAFE_FILE);fs::remove(root/"alias.onnx");
    std::error_code error;
#ifdef _WIN32
    // A directory junction requires no symlink privilege/Developer Mode. Exercise
    // the actual reparse boundary without modifying machine policy or skipping it.
    const auto link=root/"link.onnx";fs::create_directory(link);
    const auto h=CreateFileW(link.c_str(),GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS,nullptr);
    const auto target=std::wstring(L"\\??\\")+root.wstring();const auto length=static_cast<uint16_t>(target.size()*sizeof(wchar_t));
    std::vector<uint8_t> reparse(20+length,0);const uint32_t tag=IO_REPARSE_TAG_MOUNT_POINT;const uint16_t data_length=static_cast<uint16_t>(12+length),print_offset=static_cast<uint16_t>(length+2);
    std::memcpy(reparse.data(),&tag,4);std::memcpy(reparse.data()+4,&data_length,2);std::memcpy(reparse.data()+10,&length,2);std::memcpy(reparse.data()+12,&print_offset,2);std::memcpy(reparse.data()+16,target.data(),length);
    DWORD returned=0;const bool created=h!=INVALID_HANDLE_VALUE&&DeviceIoControl(h,FSCTL_SET_REPARSE_POINT,reparse.data(),static_cast<DWORD>(reparse.size()),nullptr,0,&returned,nullptr);
    if(h!=INVALID_HANDLE_VALUE){CloseHandle(h);}
    if(!created){error=std::error_code(static_cast<int>(GetLastError()),std::system_category());}
#else
    fs::create_symlink(root/"model.onnx",root/"link.onnx",error);
#endif
    if(error){std::fprintf(stderr,"Cannot create required symlink fixture: %s\n",error.message().c_str());++n;}
    else {verify(PIXAURA_MODEL_UNSAFE_FILE);fs::remove(root/"link.onnx");}
    write("manifest.pxm",std::vector<uint8_t>(65537));verify(PIXAURA_MODEL_MANIFEST_LIMIT_REASON);
    write("manifest.pxm",valid.manifest);
    fs::resize_file(root/"model.onnx",67108865);verify(PIXAURA_MODEL_SIZE);
    write("model.onnx",valid.payload);
    for(int i=0;i<8;++i){const auto name="extra"+std::to_string(i)+".onnx";write(name.c_str(),{1});}
    verify(PIXAURA_MODEL_COUNT);
    for(int i=0;i<8;++i){fs::remove(root/("extra"+std::to_string(i)+".onnx"));}
    fs::remove(root/"manifest.pxm");fs::remove(root/"signature.pxs");fs::remove(root/"model.onnx");fs::remove(root);
    std::printf("Model package corpus: 45 package rows + 2 independent RFC checks + 7 filesystem checks; failures=%d\n",n);return n?1:0;
}
