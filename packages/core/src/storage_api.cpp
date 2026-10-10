#include "pixaura/storage.h"
#include "storage.hpp"
#include "../vendor/sqlite/sqlite3.h"
#include <new>
int32_t pixaura_storage_check(uint32_t version,const uint8_t* root,uint64_t bytes,pixaura_storage_info* output) {
    if(version!=1)return 2;
    if(!root||!output||bytes==0||bytes>1024)return 1;
    try {
        pixaura::storage::Repository repository(std::string_view(reinterpret_cast<const char*>(root),static_cast<std::size_t>(bytes)));
        pixaura_storage_info result{1,sizeof(pixaura_storage_info),repository.version(),static_cast<uint32_t>(sqlite3_libversion_number())};*output=result;return 0;
    } catch(const pixaura::document::Failure& e){return e.code;}
    catch(const std::bad_alloc&){return 8;}
    catch(...){return 14;}
}
int32_t pixaura_storage_migrate(uint32_t version,const uint8_t* root,uint64_t bytes,uint32_t expected,uint32_t target,pixaura_storage_info* output){
    if(version!=1)return 2;
    if(!root||!output||bytes==0||bytes>1024)return 1;
    if(!((expected==1&&(target==1||target==2))||(expected==2&&target==2)))return 4;
    try{
        pixaura::storage::Repository repository(std::string_view(reinterpret_cast<const char*>(root),static_cast<std::size_t>(bytes)));
        repository.migrate(expected,target);
        const pixaura_storage_info result{1,sizeof(pixaura_storage_info),target,static_cast<uint32_t>(sqlite3_libversion_number())};*output=result;return 0;
    }catch(const pixaura::document::Failure& e){return e.code;}catch(const std::bad_alloc&){return 8;}catch(...){return 14;}
}
