#ifndef PIXAURA_STORAGE_HPP
#define PIXAURA_STORAGE_HPP
#include "document.hpp"
#include <memory>
#include <mutex>
namespace pixaura::storage {
using document::String;
constexpr uint32_t storage_version = 1;
constexpr uint64_t asset_limit = 8589934592ULL;
constexpr std::size_t stream_buffer = 65536;
constexpr int32_t busy = 15;
enum class Point { before_temp, asset_write, before_publish, after_publish, in_transaction, after_commit, checkpoint_write, checkpoint_publish, checkpoint_renamed, migration_copy, migration_schema, migration_validate, migration_commit, migration_published };
#ifdef PIXAURA_STORAGE_TESTING
extern thread_local int fault_point;
extern thread_local bool crash_fault;
extern thread_local void (*point_hook)(Point);
extern thread_local const char* temp_name_override;
extern thread_local int migration_sqlite_error;
#endif
void fault(Point);
struct Reader {
    virtual ~Reader() = default;
    // Return 0 only at EOF; throw Failure{IO_ERROR} on read failure.
    virtual std::size_t read(uint8_t*, std::size_t capacity) = 0;
};
struct Asset { String digest; uint64_t bytes = 0; };
class Files;
class AssetStore {
    std::shared_ptr<Files> files_;
    friend class Repository;
public:
    explicit AssetStore(std::string_view private_root);
    Asset ingest(Reader&, uint64_t expected_bytes, std::string_view expected_digest = {}, std::string_view filename = {});
    void verify(const Asset&) const;
    // Incomplete staging files are ignored, never promoted or trusted.
};
struct Loaded { document::Snapshot snapshot; uint64_t epoch; };
struct Saved { uint64_t epoch; bool checkpoint_published; };
class Repository {
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::mutex mutex_;
public:
    // Root must be an existing caller-owned OS-private directory, never a user filename.
    explicit Repository(std::string_view private_root);
    ~Repository();
    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;
    uint32_t version();
    // Inspection/current-session refresh. Editing reopen must acquire a fresh
    // OS-generated session via open_session, invalidating earlier candidates.
    Loaded read();
    Loaded open_session(std::string_view fresh_session);
    Saved create(const document::ImageDocument&);
    Saved apply(std::string_view command, uint64_t expected_epoch);
    bool checkpoint();
    // Explicit verified same-version no-op or transactional 1->2 / 2->3 migration.
    // Ordinary open preserves the existing version and never migrates.
    void migrate(uint32_t expected, uint32_t target);
};
}
#endif
