#ifndef PIXAURA_STORAGE_SCHEMA_V3_HPP
#define PIXAURA_STORAGE_SCHEMA_V3_HPP
namespace pixaura::storage {
inline constexpr const char* schema_v3_sql = R"sql(
CREATE TABLE assets(hash TEXT PRIMARY KEY CHECK(length(hash)=64 AND hash NOT GLOB '*[^0-9a-f]*'), bytes INTEGER NOT NULL CHECK(bytes BETWEEN 1 AND 8589934592)) STRICT;
CREATE TABLE documents(
 id TEXT PRIMARY KEY CHECK(length(id)=32 AND id NOT GLOB '*[^0-9a-f]*'),
 project TEXT NOT NULL UNIQUE CHECK(length(project)=32 AND project NOT GLOB '*[^0-9a-f]*'),
 singleton INTEGER NOT NULL UNIQUE CHECK(singleton=1), manifest_schema INTEGER NOT NULL CHECK(manifest_schema=1),
 source TEXT NOT NULL REFERENCES assets(hash), icc TEXT REFERENCES assets(hash),
 width INTEGER NOT NULL CHECK(width BETWEEN 1 AND 65535), height INTEGER NOT NULL CHECK(height BETWEEN 1 AND 65535 AND width*height<=268435456),
 orientation INTEGER NOT NULL CHECK(orientation BETWEEN 1 AND 8), codec TEXT NOT NULL CHECK(codec IN ('jpeg','png')),
 alpha INTEGER NOT NULL CHECK(alpha IN (0,1) AND (codec!='jpeg' OR alpha=0)),
 current TEXT NOT NULL, session TEXT NOT NULL CHECK(length(session)=32 AND session NOT GLOB '*[^0-9a-f]*'),
 generation INTEGER NOT NULL CHECK(generation BETWEEN 0 AND 9007199254740991), epoch INTEGER NOT NULL CHECK(epoch BETWEEN 1 AND 9007199254740991),
 FOREIGN KEY(id,current) REFERENCES revisions(document,id) DEFERRABLE INITIALLY DEFERRED
) STRICT;
CREATE TABLE operations(
 document TEXT NOT NULL REFERENCES documents(id) DEFERRABLE INITIALLY DEFERRED,
 id TEXT NOT NULL CHECK(length(id)=32 AND id NOT GLOB '*[^0-9a-f]*'), seq INTEGER NOT NULL CHECK(seq BETWEEN 0 AND 4095),
 type TEXT NOT NULL CHECK(type IN ('pixaura.exposure','pixaura.crop','pixaura.rotate','pixaura.brightness','pixaura.contrast','pixaura.highlights','pixaura.shadows','pixaura.saturation','pixaura.temperature','pixaura.blur','pixaura.sharpen')), ov INTEGER NOT NULL CHECK(ov=1), pv INTEGER NOT NULL CHECK(pv=1),
 ev INTEGER, turns INTEGER, x INTEGER, y INTEGER, w INTEGER, h INTEGER,
 brightness INTEGER, contrast INTEGER, highlights INTEGER, shadows INTEGER, saturation INTEGER, temperature INTEGER, blur INTEGER, sharpen INTEGER,
 CHECK((type='pixaura.exposure' AND ev IS NOT NULL AND ev BETWEEN -5000 AND 5000 AND turns IS NULL AND x IS NULL AND y IS NULL AND w IS NULL AND h IS NULL)
 OR (type='pixaura.rotate' AND turns IS NOT NULL AND turns BETWEEN 0 AND 3 AND ev IS NULL AND x IS NULL AND y IS NULL AND w IS NULL AND h IS NULL)
 OR (type='pixaura.crop' AND ev IS NULL AND turns IS NULL AND x IS NOT NULL AND y IS NOT NULL AND w IS NOT NULL AND h IS NOT NULL AND x>=0 AND y>=0 AND w>0 AND h>0 AND x+w<=1000000 AND y+h<=1000000)
 OR (type='pixaura.brightness' AND brightness IS NOT NULL AND brightness BETWEEN -1000 AND 1000)
 OR (type='pixaura.contrast' AND contrast IS NOT NULL AND contrast BETWEEN -2000 AND 2000)
 OR (type='pixaura.highlights' AND highlights IS NOT NULL AND highlights BETWEEN -2000 AND 2000)
 OR (type='pixaura.shadows' AND shadows IS NOT NULL AND shadows BETWEEN -2000 AND 2000)
 OR (type='pixaura.saturation' AND saturation IS NOT NULL AND saturation BETWEEN 0 AND 2000)
 OR (type='pixaura.temperature' AND temperature IS NOT NULL AND temperature BETWEEN 4000 AND 25000)
 OR (type='pixaura.blur' AND blur IS NOT NULL AND blur BETWEEN 0 AND 1000)
 OR (type='pixaura.sharpen' AND sharpen IS NOT NULL AND sharpen BETWEEN 0 AND 1000)),
 CHECK((ev IS NOT NULL)+(turns IS NOT NULL)+(x IS NOT NULL)+(y IS NOT NULL)+(w IS NOT NULL)+(h IS NOT NULL)+(brightness IS NOT NULL)+(contrast IS NOT NULL)+(highlights IS NOT NULL)+(shadows IS NOT NULL)+(saturation IS NOT NULL)+(temperature IS NOT NULL)+(blur IS NOT NULL)+(sharpen IS NOT NULL)=CASE WHEN type='pixaura.crop' THEN 4 ELSE 1 END),
 PRIMARY KEY(document,id), UNIQUE(document,seq)
) STRICT;
CREATE TABLE revisions(
 document TEXT NOT NULL REFERENCES documents(id) DEFERRABLE INITIALLY DEFERRED,
 id TEXT NOT NULL CHECK(length(id)=32 AND id NOT GLOB '*[^0-9a-f]*'), seq INTEGER NOT NULL CHECK(seq BETWEEN 0 AND 4095),
 parent TEXT, actor TEXT NOT NULL, plan TEXT CHECK(plan IS NULL OR (length(plan)=32 AND plan NOT GLOB '*[^0-9a-f]*')),
 CHECK((seq=0 AND parent IS NULL AND actor='import' AND plan IS NULL) OR (seq>0 AND parent IS NOT NULL AND ((actor='manual' AND plan IS NULL) OR (actor='ai' AND plan IS NOT NULL)))),
 PRIMARY KEY(document,id), UNIQUE(document,seq), FOREIGN KEY(document,parent) REFERENCES revisions(document,id)
) STRICT;
CREATE TRIGGER revision_parent BEFORE INSERT ON revisions WHEN NEW.parent IS NOT NULL BEGIN
 SELECT CASE WHEN NOT EXISTS(SELECT 1 FROM revisions WHERE document=NEW.document AND id=NEW.parent AND seq<NEW.seq) THEN RAISE(ABORT,'invalid parent order') END;
END;
CREATE TABLE stacks(document TEXT NOT NULL, revision TEXT NOT NULL, pos INTEGER NOT NULL CHECK(pos BETWEEN 0 AND 255), operation TEXT NOT NULL,
 PRIMARY KEY(document,revision,pos), UNIQUE(document,revision,operation),
 FOREIGN KEY(document,revision) REFERENCES revisions(document,id), FOREIGN KEY(document,operation) REFERENCES operations(document,id)) STRICT;
CREATE TABLE redo(document TEXT NOT NULL REFERENCES documents(id), pos INTEGER NOT NULL CHECK(pos BETWEEN 0 AND 4094), revision TEXT NOT NULL,
 PRIMARY KEY(document,pos), UNIQUE(document,revision), FOREIGN KEY(document,revision) REFERENCES revisions(document,id)) STRICT;
CREATE TRIGGER immutable_operation_update BEFORE UPDATE ON operations BEGIN SELECT RAISE(ABORT,'immutable operation'); END;
CREATE TRIGGER immutable_operation_delete BEFORE DELETE ON operations BEGIN SELECT RAISE(ABORT,'immutable operation'); END;
CREATE TRIGGER immutable_revision_update BEFORE UPDATE ON revisions BEGIN SELECT RAISE(ABORT,'immutable revision'); END;
CREATE TRIGGER immutable_revision_delete BEFORE DELETE ON revisions BEGIN SELECT RAISE(ABORT,'immutable revision'); END;
CREATE TRIGGER immutable_stack_update BEFORE UPDATE ON stacks BEGIN SELECT RAISE(ABORT,'immutable stack'); END;
CREATE TRIGGER immutable_stack_delete BEFORE DELETE ON stacks BEGIN SELECT RAISE(ABORT,'immutable stack'); END;
CREATE TRIGGER immutable_source BEFORE UPDATE OF id,project,source,icc,width,height,orientation,codec,alpha,manifest_schema ON documents BEGIN SELECT RAISE(ABORT,'immutable source'); END;
CREATE TRIGGER immutable_asset_update BEFORE UPDATE ON assets BEGIN SELECT RAISE(ABORT,'immutable asset'); END;
CREATE TRIGGER immutable_asset_delete BEFORE DELETE ON assets BEGIN SELECT RAISE(ABORT,'immutable asset'); END;
PRAGMA application_id=1346979905;
PRAGMA user_version=3;
)sql";
}
#endif
