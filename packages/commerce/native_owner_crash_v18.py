"""Real processes, native Schema3 project and valid planner; no store mock admission."""
import pathlib
import subprocess
import sys
import tempfile
exe = str(pathlib.Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory(prefix="v18-native-crash-", dir=pathlib.Path.cwd()) as directory:
    for point in (1, 2, 3, 4):
        root = pathlib.Path(directory) / ("project-" + str(point))
        ledger = root / "commercial.sqlite"
        subprocess.run([exe, "setup", str(root)], check=True)
        subprocess.run([exe, "crash", str(root), str(ledger), str(point)], check=True)
        scenario = "inspect-delivered" if point == 4 else "inspect-pending"
        subprocess.run([exe, scenario, str(root), str(ledger)], check=True)
print("native advisory actual _Exit: reserved/validated/beforeSQL/afterSQL; fresh-session revalidation without charge PASS")
