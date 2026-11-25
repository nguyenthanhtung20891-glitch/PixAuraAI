"""Real process termination around SQLite COMMIT; synthetic signatures only."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import uuid

binary = str(Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory(prefix="pixaura-store-journal-v3-") as parent:
    try:
        for point in ("before_commit", "after_commit"):
            # SQLite NOFOLLOW requires the canonical temporary parent on macOS.
            database = str(Path(parent).resolve() / (point + ".sqlite"))
            subprocess.run([binary, "--journal-prepare", database, "active"], check=True)
            result = subprocess.run([binary, "--journal-crash", database, point])
            assert result.returncode == 76, (point, result.returncode)
            expected = "active" if point == "before_commit" else "revoked"
            subprocess.run([binary, "--journal-inspect", database, expected], check=True)
    except BaseException:
        retained = Path.cwd() / ("store-journal-v3-failure-" + uuid.uuid4().hex)
        shutil.copytree(parent, retained)
        print("Synthetic crash artifacts retained: " + str(retained), file=sys.stderr)
        raise
print("Store journal: process death before COMMIT rolls back; after COMMIT restores revocation and identical acknowledgment; no trial/paid activation")
