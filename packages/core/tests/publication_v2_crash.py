from pathlib import Path
import subprocess,sys,tempfile
exe=str(Path(sys.argv[1]).resolve());fixture=str(Path(sys.argv[2]).resolve())
with tempfile.TemporaryDirectory(prefix='pixaura-publication-v2-') as parent:
    for point in ['copy','schema','validate','commit','published']:
        root=str((Path(parent)/('migration-'+point)).resolve())
        subprocess.run([exe,'--prepare-schema3',root,fixture],check=True)
        result=subprocess.run([exe,'--migrate-crash',root,point])
        assert result.returncode==73,(point,result.returncode)
        subprocess.run([exe,'--inspect-migration',root,'4' if point=='published' else '3'],check=True)
    for point,count in [('before',0),('after',1)]:
        root=str((Path(parent)/point).resolve())
        subprocess.run([exe,'--prepare',root,fixture],check=True)
        result=subprocess.run([exe,'--crash',root,point])
        assert result.returncode==73,(point,result.returncode)
        subprocess.run([exe,'--inspect',root,str(count)],check=True)
print('Actual process death beforeCOMMIT: no receipt/revision; afterCOMMIT: exactly one authenticated receipt+ordinaryrevision PASS')
