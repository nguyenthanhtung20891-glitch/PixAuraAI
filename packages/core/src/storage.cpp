#include "storage.hpp"
#include "storage_files.hpp"
#include "storage_schema.hpp"
#include "storage_schema_v2.hpp"
#include "tone.hpp"
#include "sha256.hpp"
#include "../vendor/sqlite/sqlite3.h"
#include <cstring>
#include <limits>
#include <cstdlib>
namespace pixaura::storage {
#ifdef PIXAURA_STORAGE_TESTING
thread_local int migration_sqlite_error=SQLITE_OK;
#endif
namespace {
void require(bool ok,int32_t code=6){if(!ok)throw document::Failure{code};}
void checked(int rc){if(rc==SQLITE_OK||rc==SQLITE_ROW||rc==SQLITE_DONE)return;const auto c=rc&255;throw document::Failure{c==SQLITE_BUSY||c==SQLITE_LOCKED?busy:c==SQLITE_NOMEM||c==SQLITE_TOOBIG||c==SQLITE_INTERRUPT||c==SQLITE_FULL?8:c==SQLITE_CORRUPT||c==SQLITE_NOTADB||c==SQLITE_CONSTRAINT?6:12};}
void exec(sqlite3* db,const char* sql){checked(sqlite3_exec(db,sql,nullptr,nullptr,nullptr));}
struct Statement {
    sqlite3_stmt* p=nullptr;
    Statement(sqlite3* db,const char* sql){checked(sqlite3_prepare_v2(db,sql,-1,&p,nullptr));}
    ~Statement(){sqlite3_finalize(p);}
    Statement(const Statement&)=delete;Statement& operator=(const Statement&)=delete;
    void text(int i,std::string_view s){require(s.size()<=document::manifest_limit,8);checked(sqlite3_bind_text(p,i,s.data(),static_cast<int>(s.size()),SQLITE_TRANSIENT));}
    void number(int i,uint64_t n){require(n<=document::max_generation,8);checked(sqlite3_bind_int64(p,i,static_cast<sqlite3_int64>(n)));}
    void signed_number(int i,int64_t n){checked(sqlite3_bind_int64(p,i,n));}
    void null(int i){checked(sqlite3_bind_null(p,i));}
    bool row(){const auto rc=sqlite3_step(p);checked(rc);return rc==SQLITE_ROW;}
    void done(){require(!row());}
    uint64_t number(int i){require(sqlite3_column_type(p,i)==SQLITE_INTEGER&&sqlite3_column_int64(p,i)>=0);return static_cast<uint64_t>(sqlite3_column_int64(p,i));}
    String text(int i){require(sqlite3_column_type(p,i)==SQLITE_TEXT);const auto n=sqlite3_column_bytes(p,i);require(n>=0&&static_cast<std::size_t>(n)<=document::manifest_limit,8);const auto* s=sqlite3_column_text(p,i);require(s!=nullptr);return String(reinterpret_cast<const char*>(s),static_cast<std::size_t>(n));}
};
struct Transaction {
    sqlite3* db;bool committed=false;
    Transaction(sqlite3* value,bool write):db(value){exec(db,write?"BEGIN IMMEDIATE":"BEGIN");}
    ~Transaction(){if(!committed)sqlite3_exec(db,"ROLLBACK",nullptr,nullptr,nullptr);}
    void commit(){exec(db,"COMMIT");committed=true;}
};
uint64_t scalar(sqlite3* db,const char* sql){Statement q(db,sql);require(q.row());const auto n=q.number(0);require(!q.row());return n;}
String schema_digest(sqlite3* db) {
    Sha256 h;Statement q(db,"SELECT type,name,tbl_name,coalesce(sql,'') FROM sqlite_schema ORDER BY name");
    while(q.row())for(int i=0;i<4;++i){const auto value=q.text(i);const auto size=std::to_string(value.size());h.update(reinterpret_cast<const uint8_t*>(size.data()),size.size());const uint8_t separator=':';h.update(&separator,1);h.update(reinterpret_cast<const uint8_t*>(value.data()),value.size());}
    return h.finish();
}
String expected_schema(uint32_t version) {
    sqlite3* raw=nullptr;const auto rc=sqlite3_open_v2(":memory:",&raw,SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_PRIVATECACHE,nullptr);
    struct Close{sqlite3* db;~Close(){if(db)sqlite3_close(db);}}close{raw};checked(rc);exec(raw,version==1?schema_sql:schema_v2_sql);return schema_digest(raw);
}
template<class T>T unwrap(document::Result<T> r){if(r.code)throw document::Failure{r.code,r.index};return std::move(r.value);}
// SQLite JSON encodes normalized rows; the existing bounded C++ parser remains
// the semantic authority. JSON here is a derived read result, never a stored blob.
constexpr const char* load_sql=R"sql(
SELECT json_object('schema_version',d.manifest_schema,'project_id',d.project,'document_id',d.id,
'source',json_object('sha256',d.source,'byte_length',a.bytes,'metadata',json_object('width',d.width,'height',d.height,'orientation',d.orientation,'codec',d.codec,'has_alpha',json(CASE WHEN d.alpha=1 THEN 'true' ELSE 'false' END),'icc_sha256',d.icc)),
'current_revision_id',d.current,
'operations',json((SELECT json_group_array(json_object('id',id,'type',type,'operation_version',ov,'parameter_version',pv,'parameters',json(CASE type WHEN 'pixaura.exposure' THEN json_object('milli_ev',ev) WHEN 'pixaura.rotate' THEN json_object('quarter_turns',turns) ELSE json_object('x_ppm',x,'y_ppm',y,'width_ppm',w,'height_ppm',h) END))) FROM (SELECT * FROM operations WHERE document=d.id ORDER BY seq))),
'revisions',json((SELECT json_group_array(json_object('id',r.id,'parent_id',r.parent,'actor',r.actor,'plan_id',r.plan,'stack',json((SELECT json_group_array(operation) FROM (SELECT operation FROM stacks WHERE document=d.id AND revision=r.id ORDER BY pos))))) FROM (SELECT * FROM revisions WHERE document=d.id ORDER BY seq) r)),
'redo',json((SELECT json_group_array(revision) FROM (SELECT revision FROM redo WHERE document=d.id ORDER BY pos)))),d.session,d.generation,d.epoch
FROM documents d JOIN assets a ON a.hash=d.source WHERE d.singleton=1
)sql";
}
struct Repository::Impl {
    AssetStore assets;sqlite3* db=nullptr;String schema_hash_v1, schema_hash_v2;int progress_count=0;
    explicit Impl(std::string_view root):assets(root),schema_hash_v1(expected_schema(1)),schema_hash_v2(expected_schema(2)) {
        require(sqlite3_libversion_number()==SQLITE_VERSION_NUMBER,4);
        const auto path=assets.files_->database_path();
        const auto rc=sqlite3_open_v2(path.c_str(),&db,SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX|SQLITE_OPEN_PRIVATECACHE|SQLITE_OPEN_NOFOLLOW,nullptr);
        if(rc!=SQLITE_OK){if(db)sqlite3_close(db);db=nullptr;checked(rc);}
        try {
            checked(sqlite3_extended_result_codes(db,1));checked(sqlite3_busy_timeout(db,250));
            sqlite3_limit(db,SQLITE_LIMIT_LENGTH,static_cast<int>(document::manifest_limit));
            sqlite3_limit(db,SQLITE_LIMIT_SQL_LENGTH,65536);sqlite3_limit(db,SQLITE_LIMIT_COLUMN,128);
            sqlite3_limit(db,SQLITE_LIMIT_EXPR_DEPTH,64);sqlite3_limit(db,SQLITE_LIMIT_ATTACHED,0);sqlite3_limit(db,SQLITE_LIMIT_VARIABLE_NUMBER,64);
            checked(sqlite3_db_config(db,SQLITE_DBCONFIG_DEFENSIVE,1,nullptr));checked(sqlite3_db_config(db,SQLITE_DBCONFIG_TRUSTED_SCHEMA,0,nullptr));
            exec(db,"PRAGMA foreign_keys=ON; PRAGMA trusted_schema=OFF; PRAGMA cache_size=-2048; PRAGMA temp_store=MEMORY; PRAGMA fullfsync=ON; PRAGMA checkpoint_fullfsync=ON;");
            require(scalar(db,"PRAGMA foreign_keys")==1);
            const auto v=scalar(db,"PRAGMA user_version");
            if(v==0) {
                require(scalar(db,"SELECT count(*) FROM sqlite_schema")==0&&scalar(db,"PRAGMA application_id")==0,4);
                exec(db,"PRAGMA journal_mode=WAL; PRAGMA synchronous=FULL;");
                Transaction tx(db,true);exec(db,schema_sql);tx.commit();
            }else require(v==1||v==2,4);
            require(scalar(db,"PRAGMA application_id")==1346979905,4);validate_schema();
            Statement journal(db,"PRAGMA journal_mode=WAL");require(journal.row()&&journal.text(0)=="wal");require(!journal.row());
            exec(db,"PRAGMA synchronous=FULL; PRAGMA wal_autocheckpoint=1000; PRAGMA journal_size_limit=16777216; PRAGMA max_page_count=65536;");
            require(scalar(db,"PRAGMA synchronous")==2&&scalar(db,"PRAGMA page_count")<=65536&&scalar(db,"PRAGMA page_size")==4096,8);
            sqlite3_progress_handler(db,1000,[](void* p)->int{auto& count=*static_cast<int*>(p);return ++count>20000;},&progress_count);
        } catch(...){sqlite3_close(db);db=nullptr;throw;}
    }
    ~Impl(){if(db)sqlite3_close(db);}
    void admit_write() {
        progress_count=0;
        // A long reader must not permit indefinite WAL growth. Resume only after
        // a bounded truncate checkpoint can finish; never discard committed WAL.
        if(assets.files_->wal_bytes()>33554432)checked(sqlite3_wal_checkpoint_v2(db,nullptr,SQLITE_CHECKPOINT_TRUNCATE,nullptr,nullptr));
        require(assets.files_->wal_bytes()<=33554432,8);
    }
    uint32_t validate_schema(uint32_t migration_target=0){const auto v=scalar(db,"PRAGMA user_version");require(v==1||v==2,4);require(migration_target==0||(v==1&&migration_target==2),4);const auto format=migration_target?migration_target:static_cast<uint32_t>(v);require(scalar(db,"PRAGMA application_id")==1346979905,4);require(schema_digest(db)==(format==1?schema_hash_v1:schema_hash_v2));return format;}
    void verify_database(uint32_t migration_target=0) {
        progress_count=0;validate_schema(migration_target);Statement quick(db,"PRAGMA quick_check(1)");require(quick.row()&&quick.text(0)=="ok"&&!quick.row());
        Statement fk(db,"PRAGMA foreign_key_check");require(!fk.row());
        require(scalar(db,"SELECT count(*) FROM documents")==1);
        for(const auto* sql:{"SELECT count(*) FROM operations","SELECT count(*) FROM revisions"})require(scalar(db,sql)<=4096,8);
        require(scalar(db,"SELECT count(*) FROM redo")<=4095,8);
        require(scalar(db,"SELECT count(*) FROM stacks")<=1048576,8);
        require(scalar(db,"SELECT count(*) FROM assets")<=2,8);
        require(scalar(db,"SELECT count(*) FROM (SELECT 1 FROM operations GROUP BY document HAVING min(seq)!=0 OR max(seq)+1!=count(*) UNION ALL SELECT 1 FROM revisions GROUP BY document HAVING min(seq)!=0 OR max(seq)+1!=count(*) UNION ALL SELECT 1 FROM stacks GROUP BY document,revision HAVING min(pos)!=0 OR max(pos)+1!=count(*) UNION ALL SELECT 1 FROM redo GROUP BY document HAVING min(pos)!=0 OR max(pos)+1!=count(*))")==0);
    }
    Loaded load(uint32_t migration_target=0) {
        verify_database(migration_target);String query(load_sql);
        if(validate_schema(migration_target)==2){const auto at=query.find("ELSE json_object('x_ppm'");require(at!=String::npos);query.insert(at,"WHEN 'pixaura.brightness' THEN json_object('milli_linear',brightness) WHEN 'pixaura.contrast' THEN json_object('milli_stops',contrast) WHEN 'pixaura.highlights' THEN json_object('milli_ev',highlights) WHEN 'pixaura.shadows' THEN json_object('milli_ev',shadows) WHEN 'pixaura.saturation' THEN json_object('milli_ratio',saturation) WHEN 'pixaura.temperature' THEN json_object('kelvin',temperature) ");}
        Statement q(db,query.c_str());require(q.row());const auto json=q.text(0),session=q.text(1);const auto generation=q.number(2),epoch=q.number(3);require(epoch>0&&epoch<=document::max_generation&&generation<=document::max_generation);
        auto snapshot=unwrap(document::restore_generation(unwrap(document::deserialize(json,session)),generation));require(!q.row());
        assets.verify({snapshot->source().sha256.text(),snapshot->source().byte_length});
        if(snapshot->source().metadata.icc){Statement icc(db,"SELECT bytes FROM assets WHERE hash=?");icc.text(1,snapshot->source().metadata.icc->text());require(icc.row());assets.verify({snapshot->source().metadata.icc->text(),icc.number(0)});require(!icc.row());}
        return {std::move(snapshot),epoch};
    }
    void insert_asset(const Asset& a){assets.verify(a);Statement q(db,"INSERT INTO assets(hash,bytes) VALUES(?,?) ON CONFLICT(hash) DO NOTHING");q.text(1,a.digest);q.number(2,a.bytes);q.done();Statement check(db,"SELECT bytes FROM assets WHERE hash=?");check.text(1,a.digest);require(check.row()&&check.number(0)==a.bytes&&!check.row());}
    void append(const document::ImageDocument& doc,std::size_t old_operations,std::size_t old_revisions) {
        const auto& id=doc.identity().document.text();
        const auto version=validate_schema();
        for(std::size_t i=old_operations;i<doc.operations().size();++i) {
            const auto& op=doc.operations()[i];require(version==2||!tone::find(op.type),4);
            Statement q(db,version==1?"INSERT INTO operations(document,id,seq,type,ov,pv,ev,turns,x,y,w,h) VALUES(?,?,?,?,?,?,?,?,?,?,?,?)":"INSERT INTO operations VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");q.text(1,id);q.text(2,op.id.text());q.number(3,i);q.text(4,op.type);q.number(5,op.operation_version);q.number(6,op.parameter_version);for(int col=7;col<=(version==1?12:18);++col)q.null(col);
            if(const auto* p=std::get_if<document::Exposure>(&op.parameters))q.signed_number(7,p->milli_ev);
            if(const auto* p=std::get_if<document::Rotate>(&op.parameters))q.number(8,p->quarter_turns);
            if(const auto* p=std::get_if<document::Crop>(&op.parameters)){q.number(9,p->x_ppm);q.number(10,p->y_ppm);q.number(11,p->width_ppm);q.number(12,p->height_ppm);}
            if(const auto* p=std::get_if<document::Tone>(&op.parameters)){const auto column=op.type=="pixaura.brightness"?13:op.type=="pixaura.contrast"?14:op.type=="pixaura.highlights"?15:op.type=="pixaura.shadows"?16:op.type=="pixaura.saturation"?17:18;q.signed_number(column,p->value);}q.done();
        }
        for(std::size_t i=old_revisions;i<doc.revisions().size();++i) {
            const auto& r=doc.revisions()[i];Statement q(db,"INSERT INTO revisions VALUES(?,?,?,?,?,?)");q.text(1,id);q.text(2,r.id.text());q.number(3,i);if(r.parent)q.text(4,r.parent->text());else q.null(4);q.text(5,r.actor);if(r.plan)q.text(6,r.plan->text());else q.null(6);q.done();
            for(std::size_t j=0;j<r.stack.size();++j){Statement s(db,"INSERT INTO stacks VALUES(?,?,?,?)");s.text(1,id);s.text(2,r.id.text());s.number(3,j);s.text(4,r.stack[j].text());s.done();}
        }
    }
    void redo(const document::ImageDocument& doc) {
        exec(db,"DELETE FROM redo");for(std::size_t i=0;i<doc.redo().size();++i){Statement q(db,"INSERT INTO redo VALUES(?,?,?)");q.text(1,doc.identity().document.text());q.number(2,i);q.text(3,doc.redo()[i].text());q.done();}
    }
    bool checkpoint_locked() {
        admit_write();Transaction tx(db,true);auto loaded=load();const auto bytes=unwrap(document::serialize(*loaded.snapshot));
        // Reparse canonical bytes before publishing; epoch/session are DB-only.
        const auto verified=unwrap(document::serialize(*unwrap(document::deserialize(bytes,loaded.snapshot->session().id.text()))));require(bytes==verified);
        assets.files_->checkpoint(bytes);tx.commit();return true;
    }
    Saved after_commit(uint64_t epoch) {
        try{fault(Point::after_commit);return {epoch,checkpoint_locked()};}catch(...){return {epoch,false};}
    }
};
Repository::Repository(std::string_view root):impl_(std::make_unique<Impl>(root)){}
Repository::~Repository()=default;
uint32_t Repository::version(){std::lock_guard<std::mutex> lock(mutex_);impl_->progress_count=0;return impl_->validate_schema();}
Loaded Repository::read(){std::lock_guard<std::mutex> lock(mutex_);Transaction tx(impl_->db,false);auto value=impl_->load();tx.commit();return value;}
Loaded Repository::open_session(std::string_view fresh) {
    std::lock_guard<std::mutex> lock(mutex_);const auto id=document::Id::parse(fresh);impl_->admit_write();Transaction tx(impl_->db,true);auto value=impl_->load();require(id!=value.snapshot->session().id,9);require(value.epoch<document::max_generation,8);
    Statement q(impl_->db,"UPDATE documents SET session=?,generation=0,epoch=epoch+1 WHERE epoch=?");q.text(1,fresh);q.number(2,value.epoch);q.done();require(sqlite3_changes(impl_->db)==1,9);fault(Point::in_transaction);auto next=impl_->load();tx.commit();return next;
}
Saved Repository::create(const document::ImageDocument& doc) {
    std::lock_guard<std::mutex> lock(mutex_);impl_->progress_count=0;impl_->validate_schema();unwrap(document::serialize(doc));require(doc.session().generation==0,9);
    // Source is already safely published; validation happens before any reference.
    impl_->assets.verify({doc.source().sha256.text(),doc.source().byte_length});
    impl_->admit_write();Transaction tx(impl_->db,true);require(scalar(impl_->db,"SELECT count(*) FROM documents")==0);
    impl_->insert_asset({doc.source().sha256.text(),doc.source().byte_length});
    // ICC bytes require independent ingestion; no fabricated size/reference is accepted.
    if(doc.source().metadata.icc) {
        impl_->insert_asset(impl_->assets.files_->inspect(doc.source().metadata.icc->text()));
    }
    Statement q(impl_->db,"INSERT INTO documents VALUES(?,?,1,1,?,?,?,?,?,?,?,?,?,?,1)");q.text(1,doc.identity().document.text());q.text(2,doc.identity().project.text());q.text(3,doc.source().sha256.text());if(doc.source().metadata.icc)q.text(4,doc.source().metadata.icc->text());else q.null(4);
    const auto& m=doc.source().metadata;q.number(5,m.width);q.number(6,m.height);q.number(7,m.orientation);q.text(8,m.codec);q.number(9,m.has_alpha?1:0);q.text(10,doc.current().text());q.text(11,doc.session().id.text());q.number(12,0);q.done();
    impl_->append(doc,0,0);impl_->redo(doc);fault(Point::in_transaction);impl_->load();tx.commit();return impl_->after_commit(1);
}
Saved Repository::apply(std::string_view command,uint64_t expected_epoch) {
    std::lock_guard<std::mutex> lock(mutex_);impl_->admit_write();Transaction tx(impl_->db,true);auto prior=impl_->load();require(prior.epoch==expected_epoch,9);require(prior.epoch<document::max_generation,8);
    auto next=unwrap(document::transition(*prior.snapshot,command));impl_->append(*next,prior.snapshot->operations().size(),prior.snapshot->revisions().size());impl_->redo(*next);
    Statement q(impl_->db,"UPDATE documents SET current=?,generation=?,epoch=epoch+1 WHERE epoch=?");q.text(1,next->current().text());q.number(2,next->session().generation);q.number(3,expected_epoch);q.done();require(sqlite3_changes(impl_->db)==1,9);fault(Point::in_transaction);tx.commit();return impl_->after_commit(expected_epoch+1);
}
bool Repository::checkpoint(){std::lock_guard<std::mutex> lock(mutex_);return impl_->checkpoint_locked();}
void Repository::migrate(uint32_t expected,uint32_t target) {
    std::lock_guard<std::mutex> lock(mutex_);
    require((expected==1&&(target==1||target==2))||(expected==2&&target==2),4);
    impl_->admit_write();Transaction tx(impl_->db,true);require(impl_->validate_schema()==expected,4);
    {Statement quick(impl_->db,"PRAGMA quick_check(1)");require(quick.row()&&quick.text(0)=="ok"&&!quick.row());Statement fk(impl_->db,"PRAGMA foreign_key_check");require(!fk.row());}
    const auto populated=scalar(impl_->db,"SELECT count(*) FROM documents")!=0;
    std::optional<Loaded> prior;
    String before;
    if(populated){prior=impl_->load();before=unwrap(document::serialize(*prior->snapshot));}
    else for(const auto* table:{"assets","operations","revisions","stacks","redo"}){
        String query("SELECT count(*) FROM ");query.append(table);require(scalar(impl_->db,query.c_str())==0);
    }
    if(expected!=target){
        // The Architect explicitly authorizes transactionally replacing only
        // the operation table. Deferred FKs are verified before COMMIT; no IDs,
        // navigation rows, envelope values or assets are regenerated.
        const std::string_view sql(schema_v2_sql);
        const auto start=sql.find("CREATE TABLE operations("),end=sql.find("CREATE TABLE revisions(");
        require(start!=std::string_view::npos&&end>start,12);
        const String table(sql.substr(start,end-start));
        exec(impl_->db,"PRAGMA defer_foreign_keys=ON; CREATE TEMP TABLE migration_operations AS SELECT * FROM operations;");
        fault(Point::migration_copy);
        exec(impl_->db,"DROP TABLE operations;");exec(impl_->db,table.c_str());
        exec(impl_->db,"INSERT INTO operations(document,id,seq,type,ov,pv,ev,turns,x,y,w,h) SELECT document,id,seq,type,ov,pv,ev,turns,x,y,w,h FROM temp.migration_operations ORDER BY document,seq; DROP TABLE temp.migration_operations;");
        exec(impl_->db,"CREATE TRIGGER immutable_operation_update BEFORE UPDATE ON operations BEGIN SELECT RAISE(ABORT,'immutable operation'); END; CREATE TRIGGER immutable_operation_delete BEFORE DELETE ON operations BEGIN SELECT RAISE(ABORT,'immutable operation'); END;");
        fault(Point::migration_schema);
#ifdef PIXAURA_STORAGE_TESTING
        // Simulate a failed SQLite return after DDL, exercising resource and IO
        // error mapping plus the same RAII rollback as real SQLite failures.
        const auto injected=migration_sqlite_error;migration_sqlite_error=SQLITE_OK;checked(injected);
#endif
        require(schema_digest(impl_->db)==impl_->schema_hash_v2);
        Statement fk(impl_->db,"PRAGMA foreign_key_check");require(!fk.row());
        if(populated){const auto after=impl_->load(2);require(unwrap(document::serialize(*after.snapshot))==before&&after.epoch==prior->epoch&&after.snapshot->session().id==prior->snapshot->session().id&&after.snapshot->session().generation==prior->snapshot->session().generation);}
        else {require(impl_->validate_schema(2)==2);Statement quick(impl_->db,"PRAGMA quick_check(1)");require(quick.row()&&quick.text(0)=="ok"&&!quick.row());}
        fault(Point::migration_validate);
        // Version is changed only after all v2 invariants/replay equivalence.
        exec(impl_->db,"PRAGMA user_version=2;");
    }
    fault(Point::in_transaction);fault(Point::migration_commit);tx.commit();
#ifdef PIXAURA_STORAGE_TESTING
    // Post-COMMIT crash proves new-state recovery; never manufacture a returned
    // failure after successful publication, which cannot promise rollback.
    if(crash_fault&&fault_point==static_cast<int>(Point::migration_published))std::_Exit(73);
#endif
}
}
