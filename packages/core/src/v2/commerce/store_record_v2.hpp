#ifndef PIXAURA_PLATFORM_STORE_RECORD_V2_HPP
#define PIXAURA_PLATFORM_STORE_RECORD_V2_HPP
#include "../../publication_v2.hpp"
#include <charconv>
#include <set>
#include <string>
#include <vector>
namespace pixaura::commerce {
// Private metadata only. A local MAC is NOT an Apple/Google transaction signature.
// Production may seal fresh handoffs ONLY inside the actual verified OS/backend
// adapter. Protected-cache replay must reuse its previous seal, never mint one.
struct StoreCatalogV2 {
    struct Product {std::string id;uint32_t kind=0;};
    std::string application;uint32_t store=0,environment=0;std::vector<Product> products;
};
struct StoreObservationV2 {
    std::string product,transaction_digest;uint32_t kind=0,status=0;
    int64_t signed_at=0,expires=0;
};
namespace store_v2 {
inline void need(bool ok){if(!ok)throw document::Failure{6};}
inline bool identifier(const std::string& value,size_t maximum){
    if(value.empty()||value.size()>maximum)return false;
    for(unsigned char c:value)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='.'||c=='_'||c=='-'))return false;
    return true;
}
inline void catalog(const StoreCatalogV2& c){
    need(identifier(c.application,255)&&(c.store==1||c.store==2)&&(c.environment==1||c.environment==2)&&!c.products.empty()&&c.products.size()<=8);
    std::set<std::string> ids;for(const auto& p:c.products)need(identifier(p.id,128)&&(p.kind==1||p.kind==2)&&ids.insert(p.id).second);
}
inline void observation(const StoreCatalogV2& c,const StoreObservationV2& o){
    catalog(c);bool admitted=false;for(const auto& p:c.products)if(p.id==o.product&&p.kind==o.kind)admitted=true;
    need(admitted&&o.status>=1&&o.status<=3&&o.signed_at>0&&o.signed_at<=INT64_C(8640000000000)&&o.expires>=0&&o.expires<=INT64_C(8640000000000));
    (void)document::Digest::parse(o.transaction_digest);
    need(o.kind==1?o.expires>0:o.expires==0);need(!(o.kind==2&&o.status==2));
}
inline bool same_seal(std::string_view a,std::string_view b){
    if(a.size()!=64||b.size()!=64)return false;
    unsigned difference=0;
    for(size_t i=0;i<64;++i)difference|=static_cast<unsigned char>(a[i])^static_cast<unsigned char>(b[i]);
    return difference==0;
}
inline std::string canonical(const StoreCatalogV2& c,const StoreObservationV2& o){
    observation(c,o);
    return "PIXAURA_PLATFORM_STORE_CACHE_V2\n"+c.application+'\n'+std::to_string(c.store)+'\n'+std::to_string(c.environment)+'\n'+o.product+'\n'+std::to_string(o.kind)+'\n'+std::to_string(o.status)+'\n'+std::to_string(o.signed_at)+'\n'+std::to_string(o.expires)+'\n'+o.transaction_digest+'\n';
}
inline StoreObservationV2 parse(const StoreCatalogV2& c,std::string_view body){
    need(body.size()<=1024);std::vector<std::string> fields;size_t start=0;
    for(size_t i=0;i<body.size();++i)if(body[i]=='\n'){fields.emplace_back(body.substr(start,i-start));start=i+1;}
    need(start==body.size()&&fields.size()==10&&fields[0]=="PIXAURA_PLATFORM_STORE_CACHE_V2"&&fields[1]==c.application);
    auto number=[&](size_t at){int64_t out=0;const auto& s=fields.at(at);const auto result=std::from_chars(s.data(),s.data()+s.size(),out);need(!s.empty()&&result.ec==std::errc{}&&result.ptr==s.data()+s.size()&&std::to_string(out)==s);return out;};
    need(number(2)==c.store&&number(3)==c.environment);
    const auto kind=number(5),status=number(6);need(kind>=1&&kind<=2&&status>=1&&status<=3);
    StoreObservationV2 out{fields[4],fields[9],static_cast<uint32_t>(kind),static_cast<uint32_t>(status),number(7),number(8)};
    need(canonical(c,out)==body);return out;
}
class AuthenticatedCache final {
    StoreObservationV2 value_;
    explicit AuthenticatedCache(StoreObservationV2 value):value_(std::move(value)){}
public:
    static AuthenticatedCache verify(const StoreCatalogV2& catalog,std::string_view body,std::string_view seal,storage::ReceiptAuthorityV2& key){
        auto value=parse(catalog,body);need(same_seal(seal,key.authenticate(body)));return AuthenticatedCache(std::move(value));
    }
    const StoreObservationV2& value()const{return value_;}
};
}
}
#endif
