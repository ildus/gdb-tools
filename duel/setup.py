from distutils.core import setup, Extension
from distutils.command.build_ext import build_ext
from distutils.log import INFO
import subprocess

class BuildDuelpy(build_ext):
  def run(self):
    self.announce('running yacc', level=INFO)
    subprocess.Popen(['yacc','parse.y'], cwd='duel')
    build_ext.run(self)

duel = Extension('duelpy',
                 #extra_compile_args = ['-ggdb3','-O0'],
                 sources = ['duelgdb.c', 'duel/duel.c', 'duel/types.c',
                     'duel/eval.c', 'duel/misc.c', 'duel/y.tab.c',
                     'duel/error.c', 'duel/evalops.c', 'duel/print.c'])

setup (name = 'duelpy',
       version = '1.0',
       description = 'A Very High Level Debugging Language',
       cmdclass={ 'build_ext' : BuildDuelpy },
       ext_modules = [duel])
