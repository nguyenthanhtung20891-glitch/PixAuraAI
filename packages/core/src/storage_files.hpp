#ifndef PIXAURA_STORAGE_FILES_HPP
#define PIXAURA_STORAGE_FILES_HPP
#include "storage.hpp"
namespace pixaura::storage {
class Files {
    struct Impl;
    std::unique_ptr<Impl> impl_;
public:
    explicit Files(std::string_view root);
    ~Files();
    Asset ingest(Reader&, uint64_t, std::string_view);
    void verify(const Asset&) const;
    Asset inspect(std::string_view digest) const;
    String database_path();
    uint64_t wal_bytes() const;
    bool checkpoint(std::string_view bytes);
};
}
#endif
