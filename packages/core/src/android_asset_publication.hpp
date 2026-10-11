#ifndef PIXAURA_ANDROID_ASSET_PUBLICATION_HPP
#define PIXAURA_ANDROID_ASSET_PUBLICATION_HPP
#include "document.hpp"
#include <cerrno>
#include <linux/fs.h>
#include <sys/syscall.h>
#include <unistd.h>
namespace pixaura::storage {
struct AndroidPublicationSyscalls {
    int rename_no_replace(int source, const char* name, int destination, const char* digest) {
        return static_cast<int>(syscall(SYS_renameat2, source, name, destination, digest, RENAME_NOREPLACE));
    }
    int sync_directory(int descriptor) { return fsync(descriptor); }
};
// Also exercised with a test-owned syscall adapter. Production always supplies
// the real adapter; neither unsupported operations nor sync errors have fallback.
template<class Syscalls> bool android_asset_publish(int source, const char* name,
    int destination, const char* digest, Syscalls& os) {
    if (os.rename_no_replace(source, name, destination, digest) == 0) {
        if (os.sync_directory(destination) != 0 || os.sync_directory(source) != 0)
            throw document::Failure{12};
        return true;
    }
    if (errno != EEXIST) throw document::Failure{12};
    return false;
}
}
#endif
