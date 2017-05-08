# gdb-tools

This repository contains various tools used to make the time spent in gdb more
comfortable.

I keep my gdb extensions in `~/.gdb.py` and my `~/.gdbinit` contains

    source /path/to/.gdb.py

## pretty_printer.py

A convenience helper to write **gdb pretty-printers**. Import this module and
write new pretty printers as easy as
```python
@PrettyPrinter
def st_bitmap(val):
    s=''
    for i in range((val['n_bits']+7)//8):
        s = format(int(val['bitmap'][i]), '032b') + s
    return "b'" + s[-int(val['n_bits']):] + "'"
```
Here `val` is a `gdb.Value` object to print, and `st_bitmap` is the type to
pretty-print (alternatively, a type can be passed to the decorator as an
argument, useful for types that aren't valid Python identifiers). If the type
has a name, either typedef'ed name or the underlying actual type can be used in
the pretty printer definition (useful, for types like
`typedef int set_of_flags`). Pointers are resolved automatically:
```
(gdb) p map
$1 = b'001010111'
(gdb) p &map
$1 = (st_bitmap *) 0x7fff8610 b'001010111'
```

## DUEL — Debugging U (might) Even Like

A high level language for examining various data structures. Public domain
project created by Michael Golan in 1993. "Insanely cool", according to gdb
developers. I've ported it to work with gdb 7 via the gdb Python API.
Usual `python setup.py build` to build, `import duel` in your `.gdb.py` to use,
run `dl` from gdb prompt for help. Few examples from the DUEL manual:

Command | Explanation
------------ | -------------
`x[10..20,22,24,40..60]` | display `x[i]` for the selected indexes
`x[9..0]` | display `x[i]` backwards
`x[..100] >? 5 <? 10` | display `x[i]` if `5<x[i]<10`
`val[..50].if(is_dx) x else y` | `val[i].x` or `val[i].y` depending on `val[i].is_dx`
`emp[..50].if(is_m) _` | return `emp[i]` if `emp[i].is_m`
`x[i:=..100]=y[i] ;` | assign `y[i]` to `x[i]`
`x[i:=..100] >? x[i+1]` | check whether `x[i]` is sorted
`(x[..100] >? 0)[[2]]` | return the 3rd positive `x[i]`
`argv[0..]@0` | `argv[0]`, `argv[1]`, etc until first null
`emp[0..]@(code==0)` | `emp[0]`, `emp[1]`, etc until `emp[n].code==0`
`head-->next->val` | `val` of each element in a linked list
`*head-->next[[20]]` | element 20 of list
`#/head-->next` | count elements on a linked list
`#/(head-->next-val>?5)` | count those over 5
`head-->(next!=?head)` | expand cyclic linked list
