#ifndef PIXAURA_QUOTA_HPP
#define PIXAURA_QUOTA_HPP
#include <cstdint>
#include <mutex>
#include <map>
#include <string>
struct sqlite3;
namespace pixaura::commerce {
// Internal component, not editing authority or authenticated store verification.
// Only trusted OS/store adapters may supply clock/entitlement facts. Not a C ABI.
struct Clock {int64_t utc_seconds,uptime_seconds;std::string boot;bool trusted=false;};
struct Entitlement {enum Kind{none,subscription,lifetime} kind=none;bool verified=false,revoked=false;int64_t expires=0;bool offline=false;};
struct Admission {enum Decision{allowed,photo_limit,ai_limit,busy,invalid} decision=invalid;std::string token;};
struct State {int photos=0,ai=0,pending=0;int64_t day=0,trial_start=-1;bool clock_uncertain=false;};
class Ledger final {
    friend class NativeCommercialOwner; // proposed versioned native integration, not a UI authority
    friend struct LedgerTestAccess; // deterministic SQLite storage-failure injection
    sqlite3* db_=nullptr;std::mutex mutex_;std::map<std::string,Entitlement> paid_;
    void exec(const char*);int64_t value(const char*);void value(const char*,int64_t);
    int64_t tick(const Clock&);bool unlimited(int64_t);
public:
    explicit Ledger(const std::string& trusted_private_path);
    ~Ledger();Ledger(const Ledger&)=delete;Ledger& operator=(const Ledger&)=delete;
private:
    void verified_store_fact(const std::string& product,const Entitlement&); // future native owner verified-adapter boundary
public:
    Admission reserve(const std::string& request_hex,const std::string& source_sha256,
                      bool ai,const Clock&);
    // Caller has native success/source-generation proof. AI delivery must already
    // be valid/usable; persistence + charge are atomic before exposing the result.
private:
    bool complete(const std::string& token,bool success,bool committed_edit,
                  const std::string& native_validated_proposal,const Clock&);
public:
    std::string recover_delivery(const std::string& token);
    void acknowledge_delivery(const std::string& token);
    State state(const Clock&);
    static constexpr bool recovery_allowed() noexcept{return true;}
};
}
#endif
