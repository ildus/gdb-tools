from sys import path
from os.path import dirname, abspath
path.append(dirname(abspath(__file__)) + '/build/lib.linux-x86_64-2.7')
import gdb
import duelpy

class Duel (gdb.Command):
  """Evaluate Duel expressions. Duel is a very high level debugging language\n"dl help" for help."""

  def __init__ (self):
    super(Duel, self).__init__('duel', gdb.COMMAND_DATA, gdb.COMPLETE_EXPRESSION)
    gdb.execute('alias -a dl = duel')

  def invoke (self, arg, from_tty):
    duelpy.do_duel(arg)

Duel()
