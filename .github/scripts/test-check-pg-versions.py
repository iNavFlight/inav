import re,subprocess,tempfile,pathlib,os
script=str(pathlib.Path(__file__).with_name('check-pg-versions.sh').resolve())
workflow=pathlib.Path(__file__).parents[1]/'workflows'/'pg-version-check.yml'
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

# Registrations need not share the structure header's basename, and a conditional
# field must only require a bump for the build variant in which it exists.
for label, header, conditional, versions, expected in [
 ('different basename missing', 'battery_config_structs.h', False, [4], 1),
 ('different basename bumped', 'battery_config_structs.h', False, [5], 0),
 ('conditional field affected bumped', 'battery_config_structs.h', True, [5, 1], 0),
 ('conditional field unaffected bumped', 'battery_config_structs.h', True, [4, 2], 1),
 ('conditional field neither bumped', 'battery_config_structs.h', True, [4, 1], 1),
 ('compound condition affected bumped', 'battery_config_structs.h', 'compound', [5, 1], 0),
 ('compound condition unaffected bumped', 'battery_config_structs.h', 'compound', [4, 2], 1),
]:
 with tempfile.TemporaryDirectory() as d:
  def git(*a): return subprocess.run(['git','-c','user.name=CI Test','-c','user.email=ci@example.invalid',*a],cwd=d,check=True,capture_output=True,text=True).stdout.strip()
  p=pathlib.Path(d);git('init')
  def registration(v):
   rows=[f'PG_REGISTER_WITH_RESET_FN(config_t, config, PG_CONFIG, {x});' for x in v]
   return rows[0] if len(rows)==1 else '#ifdef LARGE\n'+rows[0]+'\n#else\n'+rows[1]+'\n#endif\n'
  (p/header).write_text('typedef struct config_s {\n int old;\n} config_t;\n')
  (p/'battery.c').write_text(registration([4,1] if conditional else [4]))
  git('add','.');git('commit','-m','base')
  field='#ifdef LARGE\n int added;\n#endif\n' if conditional else ' int added;\n'
  if conditional == 'compound': field = '#if defined(LARGE) && defined(EXTRA)\n int added;\n#endif\n'
  (p/header).write_text('typedef struct config_s {\n int old;\n'+field+'} config_t;\n')
  (p/'battery.c').write_text(registration(versions));git('add','.');git('commit','-m','head')
  env={k:v for k,v in os.environ.items() if k not in ('GITHUB_BASE_REF','GITHUB_HEAD_REF')}
  result=subprocess.run(['bash',script],cwd=d,capture_output=True,text=True,env=env)
  print(label,'exit',result.returncode,'expected',expected)
  assert result.returncode==expected,result.stdout+result.stderr


# A renamed + layout-changed struct header must still be checked: the old path's
# definition is otherwise lost when git diff collapses the rename to the new name.
with tempfile.TemporaryDirectory() as d:
 def git(*a): return subprocess.run(['git','-c','user.name=CI Test','-c','user.email=ci@example.invalid',*a],cwd=d,check=True,capture_output=True,text=True).stdout.strip()
 p=pathlib.Path(d);git('init')
 (p/'config.h').write_text('typedef struct config_s {\n int old;\n} config_t;\n')
 (p/'config.c').write_text('PG_REGISTER_WITH_RESET_FN(config_t, config, PG_CONFIG, 1);\n')
 git('add','.');git('commit','-m','base')
 git('mv','config.h','renamed_config.h')
 (p/'renamed_config.h').write_text('typedef struct config_s {\n int old;\n int added;\n} config_t;\n')
 git('add','.');git('commit','-m','head')
 env={k:v for k,v in os.environ.items() if k not in ('GITHUB_BASE_REF','GITHUB_HEAD_REF')}
 result=subprocess.run(['bash',script],cwd=d,capture_output=True,text=True,env=env)
 print('renamed header missing bump','exit',result.returncode,'expected',1)
 assert result.returncode==1,result.stdout+result.stderr


# Advancing the base branch must not make changes outside the PR look like removals.
with tempfile.TemporaryDirectory() as d:
 def git(*a): return subprocess.run(['git','-c','user.name=CI Test','-c','user.email=ci@example.invalid',*a],cwd=d,check=True,capture_output=True,text=True).stdout.strip()
 p=pathlib.Path(d);git('init')
 (p/'config.h').write_text('typedef struct config_s {\n int old;\n} config_t;\n')
 (p/'config.c').write_text('PG_REGISTER_WITH_RESET_FN(config_t, config, PG_CONFIG, 1);\n')
 git('add','.');git('commit','-m','common');common=git('rev-parse','HEAD')
 (p/'readme.md').write_text('PR documentation only');git('add','.');git('commit','-m','PR');head=git('rev-parse','HEAD')
 git('checkout','--detach',common)
 (p/'config.h').write_text('typedef struct config_s {\n int old;\n int added;\n} config_t;\n');git('add','.');git('commit','-m','base advancement')
 git('update-ref','refs/remotes/origin/test-base','HEAD');git('checkout','--detach',head)
 env=dict(os.environ,GITHUB_BASE_REF='test-base',GITHUB_HEAD_REF='feature')
 result=subprocess.run(['bash',script],cwd=d,capture_output=True,text=True,env=env)
 print('advanced base uses merge-base','exit',result.returncode,'expected',0)
 assert result.returncode==0,result.stdout+result.stderr

# The workflow's own "Run PG version check script" step re-parses the checker's stdout
# with a bash guard and (on the next step) a JS filter, both keyed on a literal string.
# Run that guard for real, against the checker's real "issue found" output, so the two
# can't silently drift apart the way they did across two commits in this same PR chain
# (one added a '^### ' guard for the old bash script's Markdown headings, a later one
# rewrote the checker in Python with no '###' anywhere in its output).
run_block=re.search(r"- name: Run PG version check script\n(?:.*\n)*?        run: \|\n((?:( {10}.*)?\n)+)",workflow.read_text())
assert run_block,'could not find the "Run PG version check script" step in ' + str(workflow)
guard=run_block[1]
assert 'check-pg-versions.sh' in guard and 'exit_code' in guard,'unexpected step contents:\n' + guard
with tempfile.TemporaryDirectory() as d:
 def git(*a): return subprocess.run(['git','-c','user.name=CI Test','-c','user.email=ci@example.invalid',*a],cwd=d,check=True,capture_output=True,text=True).stdout.strip()
 p=pathlib.Path(d);git('init')
 (p/'config.h').write_text('typedef struct config_s {\n int old;\n} config_t;\n')
 (p/'config.c').write_text('PG_REGISTER_WITH_RESET_FN(config_t, config, PG_CONFIG, 1);\n')
 git('add','.');git('commit','-m','base');base=git('rev-parse','HEAD')
 (p/'config.h').write_text('typedef struct config_s {\n int old;\n int added;\n} config_t;\n')
 git('add','.');git('commit','--allow-empty','-m','head, missing version bump')
 git('update-ref','refs/remotes/origin/test-base',base)
 wrapper='#!/bin/bash\nset -e\ncd ' + d + '\n' + guard.replace('.github/scripts/check-pg-versions.sh', script)
 outputs=str(p/'github_output')
 env=dict(os.environ,GITHUB_BASE_REF='test-base',GITHUB_HEAD_REF='feature',GITHUB_OUTPUT=outputs)
 result=subprocess.run(['bash','-c',wrapper],capture_output=True,text=True,env=env)
 print('workflow guard accepts a real detected issue','exit',result.returncode,'expected',0)
 assert result.returncode==0,'the workflow step would hard-fail instead of posting a comment:\n'+result.stdout+result.stderr
 assert 'exit_code=1' in pathlib.Path(outputs).read_text(),'workflow step did not record the issue for the comment step'
