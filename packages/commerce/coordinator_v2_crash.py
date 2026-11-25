from pathlib import Path
import subprocess,sys,tempfile,shutil,uuid
exe=str(Path(sys.argv[1]).resolve());fixture=str(Path(sys.argv[2]).resolve())
with tempfile.TemporaryDirectory(prefix='pixaura-commerce-v2-') as parent:
    for point in ['before_native','after_native','before_ack_commit','after_ack']:
        root=str((Path(parent)/point).resolve())
        subprocess.run([exe,'--prepare',root,fixture],check=True)
        result=subprocess.run([exe,'--crash',root,point]);assert result.returncode==74,(point,result.returncode)
        subprocess.run([exe,'--inspect',root,'no' if point=='before_native' else 'yes'],check=True)
print('4 actual crash orderings: before native / after native / before commercial COMMIT / after acknowledgment; exactlyonce native trial reconciliation PASS')

with tempfile.TemporaryDirectory(prefix='pixaura-store-format-v3-') as parent:
    try:
        for point in ['before_commit', 'after_commit']:
            root=str((Path(parent)/point).resolve())
            subprocess.run([exe,'--prepare',root,fixture],check=True)
            result=subprocess.run([exe,'--crash',root,'after_ack']);assert result.returncode==74
            result=subprocess.run([exe,'--store-format-crash',root,point]);assert result.returncode==75,(point,result.returncode)
            subprocess.run([exe,'--inspect-store-format',root,'2' if point=='before_commit' else '3'],check=True)
    except BaseException:
        preserved=Path.cwd()/('store-format-v3-failure-'+uuid.uuid4().hex)
        shutil.copytree(parent,preserved)
        print('Synthetic migration failure databases preserved at '+str(preserved),file=sys.stderr)
        raise
print('Private commerce format 2->3 actual DDL crash rollback/commit; signed native trial/revisions preserved; old-version/wrong-key rejection without file mutation PASS; no store grant')
