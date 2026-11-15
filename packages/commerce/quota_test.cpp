#include "quota.hpp"
#include "../core/vendor/sqlite/sqlite3.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <thread>
#include <vector>
namespace pixaura::commerce {
struct LedgerTestAccess {
    static bool complete(Ledger& l,const std::string& t,bool success,bool edit,const std::string& p,const Clock& c){return l.complete(t,success,edit,p,c);}
    static void fact(Ledger& l,const Entitlement& e,const std::string& product="synthetic.default"){l.verified_store_fact(product,e);}
    static void clock(Ledger& l,const char* key,int64_t value){l.value(key,value);}
    static int pages(Ledger& l){sqlite3_stmt* q=nullptr;sqlite3_prepare_v2(l.db_,"PRAGMA page_count",-1,&q,nullptr);sqlite3_step(q);int n=sqlite3_column_int(q,0);sqlite3_finalize(q);return n;}
    static void limit(Ledger& l,int n){const auto sql="PRAGMA max_page_count="+std::to_string(n);l.exec(sql.c_str());}
};
}
using namespace pixaura::commerce;
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"quota check %d\n",__LINE__);std::exit(1);}}while(false)
static std::string hex(unsigned x,unsigned width=32){char b[80];std::snprintf(b,sizeof b,"%0*x",int(width),x);return b;}
int main(int argc,char** argv){
    CHECK(argc>=2);const std::string path=argv[1];Clock c{86400*20000+86390,100,hex(1),false};
    if(argc==3){Ledger l(path);if(std::string(argv[2])=="inspect-pending"){CHECK(l.state(c).pending==0&&l.state(c).ai==0);std::_Exit(0);}if(std::string(argv[2])=="inspect-delivered"){const auto t=std::to_string(c.utc_seconds/86400)+":"+hex(1);CHECK(l.state(c).ai==1&&l.recover_delivery(t)=="native-valid-test-capsule");CHECK(LedgerTestAccess::complete(l,t,true,false,"duplicate",c));CHECK(l.state(c).ai==1&&l.recover_delivery(t)=="native-valid-test-capsule");std::_Exit(0);}auto r=l.reserve(hex(1),hex(1,64),true,c);CHECK(r.decision==Admission::allowed);if(std::string(argv[2])=="delivered")CHECK(LedgerTestAccess::complete(l,r.token,true,false,"native-valid-test-capsule",c));std::_Exit(0);}
    const auto foreign=path+".foreign";
    for(const char* table:{"unrelated_user_content","sqliteXuser_content"}){
        std::filesystem::remove(foreign);sqlite3* unrelated=nullptr;CHECK(sqlite3_open(foreign.c_str(),&unrelated)==SQLITE_OK);
        const auto sql=std::string("CREATE TABLE ")+table+"(value TEXT);INSERT INTO "+table+" VALUES('synthetic-original');";
        CHECK(sqlite3_exec(unrelated,sql.c_str(),nullptr,nullptr,nullptr)==SQLITE_OK);sqlite3_close(unrelated);
        const auto bytes=[&](){std::ifstream in(foreign,std::ios::binary);return std::string(std::istreambuf_iterator<char>(in),{});};const auto original=bytes();bool rejected=false;
        try{Ledger wrong(foreign);}catch(const std::exception&){rejected=true;}CHECK(rejected&&bytes()==original);std::filesystem::remove(foreign);
    }
    std::filesystem::remove(path);
    std::string delivered;
    {
        Ledger l(path);bool locked=false;try{Ledger second(path);}catch(const std::exception&){locked=true;}CHECK(locked);CHECK(Ledger::recovery_allowed());CHECK(l.state(c).trial_start==-1);
        CHECK(l.reserve("bad",hex(1,64),false,c).decision==Admission::invalid);
        auto fail=l.reserve(hex(1),hex(1,64),false,c);CHECK(LedgerTestAccess::complete(l,fail.token,false,false,"",c)==false);CHECK(l.state(c).photos==0&&l.state(c).trial_start==-1);
        for(unsigned i=1;i<=5;++i){auto r=l.reserve(hex(i),hex(i,64),false,c);CHECK(r.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,r.token,true,false,"",c));}
        CHECK(l.state(c).photos==5);
        CHECK(l.reserve(hex(7),hex(6,64),false,c).decision==Admission::photo_limit);
        for(unsigned i=10;i<110;++i){auto r=l.reserve(hex(i),hex(1,64),false,c);CHECK(r.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,r.token,true,false,"",c));}CHECK(l.state(c).photos==5);
        auto full=l.reserve(hex(109),hex(1,64),true,c);CHECK(full.decision==Admission::allowed);
        LedgerTestAccess::limit(l,LedgerTestAccess::pages(l));bool storage_failed=false;
        try{LedgerTestAccess::complete(l,full.token,true,false,std::string(8192,'x'),c);}catch(const std::exception&){storage_failed=true;}
        CHECK(storage_failed&&l.state(c).ai==0&&l.state(c).pending==1);
        LedgerTestAccess::limit(l,10000);CHECK(!LedgerTestAccess::complete(l,full.token,false,false,"",c));CHECK(l.state(c).pending==0);
        auto proposal_not_edit=l.reserve(hex(111),hex(1,64),true,c);CHECK(!LedgerTestAccess::complete(l,proposal_not_edit.token,true,true,"valid-capsule",c));CHECK(l.state(c).trial_start==-1&&l.state(c).ai==0);
        auto bad=l.reserve(hex(110),hex(1,64),true,c);CHECK(!LedgerTestAccess::complete(l,bad.token,true,false,"",c));CHECK(l.state(c).ai==0);
        std::atomic<unsigned> allowed{0};std::vector<std::thread> threads;std::mutex tokens_lock;std::vector<std::string> tokens;
        for(unsigned i=0;i<12;++i)threads.emplace_back([&,i]{auto r=l.reserve(hex(200+i),hex(1,64),true,c);if(r.decision==Admission::allowed){++allowed;std::lock_guard<std::mutex> lock(tokens_lock);tokens.push_back(r.token);}});
        for(auto& t:threads)t.join();CHECK(allowed==3&&l.state(c).pending==3);
        for(const auto& t:tokens)CHECK(LedgerTestAccess::complete(l,t,true,false,"native-valid-test-capsule",c));CHECK(l.state(c).ai==3);
        delivered=tokens[0];CHECK(LedgerTestAccess::complete(l,delivered,true,false,"must-not-replace",c));CHECK(l.recover_delivery(delivered)=="native-valid-test-capsule");CHECK(l.state(c).ai==3);CHECK(LedgerTestAccess::complete(l,delivered,false,false,"",c));CHECK(l.state(c).ai==3);
        // Wall/timezone changes neither advance quota date nor reverse it.
        auto changed=c;changed.utc_seconds+=86400*100;CHECK(l.state(changed).day==20000);changed.utc_seconds=1;CHECK(l.state(changed).day==20000);
        c.uptime_seconds+=11;CHECK(l.state(c).day==20001&&l.state(c).photos==0&&l.state(c).ai==0);
        CHECK(l.recover_delivery(delivered)=="native-valid-test-capsule");l.acknowledge_delivery(delivered);CHECK(l.recover_delivery(delivered).empty());
        auto pending=l.reserve(hex(300),hex(10,64),true,c);CHECK(pending.decision==Admission::allowed); // crash-cleaned next open
    }
    {
        Ledger l(path);CHECK(l.state(c).pending==0&&l.state(c).ai==0);auto r=l.reserve(hex(301),hex(1,64),false,c);CHECK(LedgerTestAccess::complete(l,r.token,true,true,"",c));const auto start=l.state(c).trial_start;CHECK(start>=0);
        // Trial bypasses daily limits, no proposal request starts/extends trial.
        for(unsigned i=0;i<8;++i){auto a=l.reserve(hex(400+i),hex(20+i,64),true,c);CHECK(a.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,a.token,true,false,"valid-capsule",c));l.acknowledge_delivery(a.token);}CHECK(l.state(c).ai==0&&l.state(c).trial_start==start);
        c.uptime_seconds+=14*86400;auto expired=l.reserve(hex(500),hex(1,64),true,c);CHECK(expired.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,expired.token,true,false,"valid-capsule",c));CHECK(l.state(c).ai==1);
        // A reboot/uptime rollback marks time uncertain. Wall jumps do not reset
        // the day; trusted time can reconcile forward, never backward.
        auto reboot=c;reboot.boot=hex(2);reboot.uptime_seconds=1;reboot.utc_seconds=1;const auto day=l.state(c).day;CHECK(l.state(reboot).clock_uncertain&&l.state(reboot).day==day);
        reboot.utc_seconds=86400*20030;reboot.trusted=true;CHECK(!l.state(reboot).clock_uncertain&&l.state(reboot).day==20030);
        LedgerTestAccess::fact(l,{Entitlement::lifetime,true,false,0});for(unsigned i=0;i<8;++i){auto a=l.reserve(hex(600+i),hex(40+i,64),false,reboot);CHECK(a.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,a.token,true,false,"",reboot));}
        auto authorized=l.reserve(hex(699),hex(89,64),false,reboot);CHECK(authorized.decision==Admission::allowed);
        LedgerTestAccess::fact(l,{Entitlement::lifetime,true,true,0});
        CHECK(LedgerTestAccess::complete(l,authorized.token,true,false,"",reboot));
auto revoked=l.reserve(hex(700),hex(90,64),true,reboot);CHECK(revoked.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,revoked.token,true,false,"valid-capsule",reboot));CHECK(l.state(reboot).ai==1);
        // Last verified subscription expiry+7days is exclusive, never seven days
        // from a failed verification. Unverified facts never grant paid access.
        LedgerTestAccess::fact(l,{Entitlement::subscription,true,false,reboot.utc_seconds-1,false});
        std::vector<std::string> online_expired;
        for(unsigned i=0;i<3;++i){auto r=l.reserve(hex(800+i),hex(90,64),true,reboot);if(i<2){CHECK(r.decision==Admission::allowed);online_expired.push_back(r.token);}else CHECK(r.decision==Admission::ai_limit);}
        for(const auto& t:online_expired)CHECK(!LedgerTestAccess::complete(l,t,false,false,"",reboot));
        LedgerTestAccess::fact(l,{Entitlement::subscription,true,false,reboot.utc_seconds-7*86400+1,true});auto grace=l.reserve(hex(701),hex(91,64),true,reboot);CHECK(grace.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,grace.token,true,false,"valid-capsule",reboot));CHECK(l.state(reboot).ai==1);
        LedgerTestAccess::fact(l,{Entitlement::subscription,true,false,reboot.utc_seconds-7*86400,true});auto end=l.reserve(hex(702),hex(92,64),true,reboot);CHECK(end.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,end.token,true,false,"valid-capsule",reboot));CHECK(l.state(reboot).ai==2);
        LedgerTestAccess::fact(l,{Entitlement::lifetime,false,false,0});auto unverified=l.reserve(hex(703),hex(93,64),true,reboot);CHECK(unverified.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,unverified.token,true,false,"valid-capsule",reboot));CHECK(l.state(reboot).ai==3);
        LedgerTestAccess::fact(l,{Entitlement::lifetime,true,false,0},"synthetic.life");
        LedgerTestAccess::fact(l,{Entitlement::subscription,true,false,reboot.utc_seconds-8*86400,true},"synthetic.monthly");
        LedgerTestAccess::fact(l,{Entitlement::lifetime,false,true,0},"synthetic.life");
        auto retained=l.reserve(hex(850),hex(99,64),true,reboot);CHECK(retained.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,retained.token,true,false,"valid-capsule",reboot));CHECK(l.state(reboot).ai==3);
        LedgerTestAccess::fact(l,{Entitlement::lifetime,true,true,0},"synthetic.life");CHECK(l.reserve(hex(851),hex(90,64),true,reboot).decision==Admission::ai_limit);
        const auto intact=l.state(reboot);LedgerTestAccess::clock(l,"utc",std::numeric_limits<int64_t>::min());bool corrupt_rejected=false;
        try{l.state(reboot);}catch(const std::exception&){corrupt_rejected=true;}CHECK(corrupt_rejected&&Ledger::recovery_allowed());LedgerTestAccess::clock(l,"utc",reboot.utc_seconds);CHECK(l.state(reboot).ai==intact.ai);
        auto before=l.state(reboot);CHECK(!LedgerTestAccess::complete(l,"forged",true,true,"valid",reboot));CHECK(l.state(reboot).ai==before.ai);CHECK(Ledger::recovery_allowed());
    }
    // Fresh reopen retains successful charges; no new trial from reinstall/cache failure.
    {Ledger l(path);Clock time{86400*20030,1,hex(2),false};CHECK(l.state(time).ai==3);}
    const auto midnight=path+".midnight";std::filesystem::remove(midnight);
    {
        Ledger l(midnight);Clock c2{86400*20060+86399,100,hex(10),false};std::vector<std::string> old;
        for(unsigned i=0;i<3;++i){auto r=l.reserve(hex(i+1),hex(i+1,64),true,c2);CHECK(r.decision==Admission::allowed);old.push_back(r.token);}
        c2.uptime_seconds+=2;CHECK(l.state(c2).day==20061);
        CHECK(l.reserve(hex(4),hex(4,64),true,c2).decision==Admission::ai_limit);
        for(const auto& t:old)CHECK(LedgerTestAccess::complete(l,t,true,false,"valid-capsule",c2));
        CHECK(l.state(c2).ai==3&&l.state(c2).photos==3);
        auto replay=l.reserve(hex(1),hex(1,64),true,c2);CHECK(replay.decision==Admission::allowed&&replay.token==old[0]);CHECK(l.state(c2).ai==3);
    }
    std::filesystem::remove(midnight);
    const auto backlog=path+".backlog";std::filesystem::remove(backlog);
    {
        Ledger l(backlog);Clock c3{86400*20060,100,hex(11),false};LedgerTestAccess::fact(l,{Entitlement::lifetime,true,false,0});
        for(unsigned i=0;i<64;++i){auto r=l.reserve(hex(i+1),hex(1,64),true,c3);CHECK(r.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,r.token,true,false,"valid-capsule",c3));}
        CHECK(l.reserve(hex(65),hex(1,64),true,c3).decision==Admission::busy);
        auto manual=l.reserve(hex(66),hex(2,64),false,c3);CHECK(manual.decision==Admission::allowed);CHECK(LedgerTestAccess::complete(l,manual.token,true,true,"",c3));
    }
    std::filesystem::remove(backlog);
    std::filesystem::remove(path);std::puts("quota: source dedup, concurrent reservations, durable delivery, cancellation, UTC midnight, wall/timezone/boot, trial/expiry/grace/revocation/recovery PASS");
}
