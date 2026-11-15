import os, pathlib, sqlite3, subprocess, sys, tempfile, contextlib
exe = str(pathlib.Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory(prefix="pixaura-quota-") as directory:
    for scenario in ("pending", "delivered"):
        path = str((pathlib.Path(directory) / (scenario + ".sqlite")).resolve())
        env = dict(os.environ, TZ="Pacific/Kiritimati")
        subprocess.run([exe, path, scenario], env=env, check=True)
        with contextlib.closing(sqlite3.connect(path)) as db:
            state = db.execute("select done,payload from request").fetchall()
            assert len(state) == 1 and state[0][0] == (scenario == "delivered")
        env["TZ"] = "America/Los_Angeles"
        subprocess.run([exe, path, "inspect-" + scenario], env=env, check=True)
print("real process _Exit + restart: pending released, usable delivery retained/exactly-once, timezone independent PASS")
