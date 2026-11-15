"""Preserve native exit status when CTest output alone loses platform detail."""
import json
import subprocess
import sys

result = subprocess.run(sys.argv[1:], check=False)
print(json.dumps({"native_exit_code": result.returncode,
                  "native_exit_hex32": hex(result.returncode & 0xffffffff)}), flush=True)
sys.exit(0 if result.returncode == 0 else 1)
