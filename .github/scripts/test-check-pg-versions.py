import subprocess,tempfile,pathlib,os
script=str(pathlib.Path(__file__).with_name('check-pg-versions.sh').resolve())
cases=[('unchanged',False,False,[1],[1],0),('missing bump',True,False,[1],[1],1),('bumped',True,False,[1],[2],0),('array missing',True,True,[4],[4],1),('array bumped',True,True,[4],[5],0),('conditional bumped',True,True,[4,1],[5,2],0),('conditional partial',True,True,[4,1],[5,1],1),('conditional decreased',True,True,[4,1],[3,2],1)]
for label,changed,array,old,new,expected in cases:
 with tempfile.TemporaryDirectory() as d:
  def git(*a): return subprocess.run(['git','-c','user.name=CI Test','-c','user.email=ci@example.invalid',*a],cwd=d,check=True,capture_output=True,text=True).stdout.strip()
  git('init'); p=pathlib.Path(d)
  def reg(versions):
   lines=[f'PG_REGISTER_{"ARRAY_" if array else ""}WITH_RESET_FN(config_t, {"3, " if array else ""}config, PG_CONFIG, {v});' for v in versions]
   return '\n'.join(lines) if len(lines)==1 else '#ifdef LARGE\n'+lines[0]+'\n#else\n'+lines[1]+'\n#endif\n'
  (p/'config.h').write_text('#define PG_REGISTER_FAKE(type, name, id, version) \"not a registration\"\n'+'typedef struct config_s {\n int old;\n} config_t;\n')
  (p/'config.c').write_text(reg(old))
  git('add','.'); git('commit','-m','base')
  if changed: (p/'config.h').write_text('#define PG_REGISTER_FAKE(type, name, id, version) \"not a registration\"\n'+'typedef struct config_s {\n int old;\n int added;\n} config_t;\n')
  (p/'config.c').write_text(reg(new))
  git('add','.'); git('commit','--allow-empty','-m','head')
  r=subprocess.run(['bash',script],cwd=d,capture_output=True,text=True,env={k:v for k,v in os.environ.items() if k not in ('GITHUB_BASE_REF','GITHUB_HEAD_REF')})
  print(label,'exit',r.returncode,'expected',expected)
  assert r.returncode==expected,r.stdout+r.stderr
  assert 'integer expression expected' not in r.stderr,r.stderr
