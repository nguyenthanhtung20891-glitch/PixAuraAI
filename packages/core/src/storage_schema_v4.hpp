#ifndef PIXAURA_STORAGE_SCHEMA_V4_HPP
#define PIXAURA_STORAGE_SCHEMA_V4_HPP
namespace pixaura::storage {
// Additive to EXACT schema3 objects. user_version changes only after validation.
inline constexpr const char* schema_v4_addition=R"sql(
CREATE TABLE publication_origin(singleton INTEGER PRIMARY KEY CHECK(singleton=1),baseline_revisions INTEGER NOT NULL CHECK(baseline_revisions BETWEEN 1 AND 4096)) STRICT;
CREATE TRIGGER immutable_origin_update BEFORE UPDATE ON publication_origin BEGIN SELECT RAISE(ABORT,'immutable publication origin'); END;
CREATE TRIGGER immutable_origin_delete BEFORE DELETE ON publication_origin BEGIN SELECT RAISE(ABORT,'immutable publication origin'); END;
CREATE TABLE publication_receipts(
 event TEXT PRIMARY KEY CHECK(length(event)=32 AND event NOT GLOB '*[^0-9a-f]*'),
 ordinal INTEGER NOT NULL UNIQUE CHECK(ordinal BETWEEN 1 AND 4095),
 prior_seal TEXT NOT NULL CHECK(prior_seal='' OR (length(prior_seal)=64 AND prior_seal NOT GLOB '*[^0-9a-f]*')),
 project TEXT NOT NULL CHECK(length(project)=32 AND project NOT GLOB '*[^0-9a-f]*'),
 document TEXT NOT NULL REFERENCES documents(id), revision TEXT NOT NULL,
 parent TEXT NOT NULL, session TEXT NOT NULL CHECK(length(session)=32 AND session NOT GLOB '*[^0-9a-f]*'),
 source TEXT NOT NULL REFERENCES assets(hash), bytes INTEGER NOT NULL CHECK(bytes BETWEEN 1 AND 8589934592),
 epoch INTEGER NOT NULL CHECK(epoch BETWEEN 1 AND 9007199254740991),
 kind TEXT NOT NULL CHECK(kind IN('manual_apply','approved_ai_apply')),
 logical_utc INTEGER NOT NULL CHECK(logical_utc>=0), wall_utc INTEGER NOT NULL CHECK(wall_utc>=0),
 uptime INTEGER NOT NULL CHECK(uptime>=0), boot TEXT NOT NULL CHECK(length(boot)=32 AND boot NOT GLOB '*[^0-9a-f]*'),
 uncertain INTEGER NOT NULL CHECK(uncertain IN(0,1)), utc_authenticated INTEGER NOT NULL CHECK(utc_authenticated IN(0,1)),
 revision_digest TEXT NOT NULL CHECK(length(revision_digest)=64 AND revision_digest NOT GLOB '*[^0-9a-f]*'),
 seal TEXT NOT NULL CHECK(length(seal)=64 AND seal NOT GLOB '*[^0-9a-f]*'),
 UNIQUE(document,revision), UNIQUE(document,epoch),
 FOREIGN KEY(document,revision) REFERENCES revisions(document,id),
 FOREIGN KEY(document,parent) REFERENCES revisions(document,id)
) STRICT;
CREATE TRIGGER immutable_receipt_update BEFORE UPDATE ON publication_receipts BEGIN SELECT RAISE(ABORT,'immutable native receipt'); END;
CREATE TRIGGER immutable_receipt_delete BEFORE DELETE ON publication_receipts BEGIN SELECT RAISE(ABORT,'immutable native receipt'); END;
)sql";
}
#endif
