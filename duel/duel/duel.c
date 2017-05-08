/*   DUEL - A Very High Level Debugging Langauge.  */
/*   Public domain code                            */
/*   Written by Michael Golan mg@cs.princeton.edu  */
/*$Header: /tmp_mnt/n/fs/grad2/mg/duel/RCS/duel.c,v 1.11 93/03/13 04:03:07 mg Exp $*/

/* this module contains the entery point to duel - duel_eval_and_pasre.
 */

/*
 * $Log:	duel.c,v $
 * Revision 1.11  93/03/13  04:03:07  mg
 * moved VERSION to patchlevel.h
 *
 * Revision 1.10  93/03/12  05:41:54  mg
 * Version 1.10 - support (x)y cast, piped output.
 *
 * Revision 1.9  93/02/23  19:09:35  mg
 * new version 1.02 release (support gdb4.8)
 *
 * Revision 1.8  93/02/03  21:56:33  mg
 * version 1.01
 *
 * Revision 1.7  93/01/12  21:28:44  mg
 * cleanup and set for release
 *
 * Revision 1.6  93/01/06  23:57:10  mg
 * added alias, clear commands, new memory alloc/release
 *
 * Revision 1.5  93/01/03  07:26:47  mg
 * new printing setup
 *
 * Revision 1.4  92/12/24  23:32:38  mg
 * better struct support, misc changes
 *
 * Revision 1.3  92/10/19  15:02:04  mg
 * lcc happy, size zero arrays
 *
 * Revision 1.2  92/10/14  02:03:15  mg
 * misc
 *
 */

#include <setjmp.h>
#define DEF             /* define global variables */
#include "duel.h"
#include "patchlevel.h"

static jmp_buf duel_abort_jmp ; /* abort current execution */
static tnode *root ;            /* root of current eval node */


/* abort evaluation of current expression */

PROC duel_abort(void)
{
    longjmp(duel_abort_jmp,1);
}

PROC duel_cleanup(void *unused) /* cleanup malloc, etc, when duel eval ends*/
{
    duel_redirectable_output_abort();
    duel_free_nodes(root);
    root=NULL ;
}

LPROC longhelp(void)
{
        duel_printf(
"Duel(1)                                                                Duel(1)\n\
\n\
NAME\n\
       duel - A high level C debugging language extension to gdb\n\
\n\
DESCRIPTION\n\
       Duel  is  a  special purpose language designed for concise state explo-\n\
       ration of debugged C programs,  currently  implemented  under  the  GNU\n\
       debugger  gdb(1).   Duel  is invoked by entering the shell command duel\n\
       instead of gdb.  It is identical to  gdb  except  for  comments,  which\n\
       begin with `##' instead of `#', and the new dl command for Duel expres-\n\
       sions:\n\
\n\
       gdb> dl x[1..10] >? 5\n\
       x[3] = 14\n\
       x[8] = 6\n\
\n\
       prints the array elements x[1] to x[10] that are greater than  5.   The\n\
       output  includes  the values 14 and 6, as well as their symbolic repre-\n\
       sentation \"x[3]\" and \"x[8]\".\n\
\n\
DUEL QUICK START\n\
       Duel is implemented by adding the dl command to gdb. All  gdb  commands\n\
       work  as  before. The dl command, however, is interpreted by duel.  Gdb\n\
       concepts (such as the value history) do not work in the dl command, and\n\
       duel concepts are not understood by other gdb command.\n\
\n\
       Duel  is  based  on expressions which return multiple values.  The x..y\n\
       operator returns the integers from x to y; the x,y operator  returns  x\n\
       and then y, e.g.\n\
\n\
       gdb> dl (1,9,12..15,22)\n\
\n\
       prints  1, 9, 12, 13, 14, 15 and 22. Such expressions can be used wher-\n\
       ever a single value is used, e.g.\n\
\n\
       gdb> dl x[0..99]=0 ;\n\
\n\
       assigns zero to the first 100 elements of array  x.  The  semantics  of\n\
       x[i]  are  the  same  as  in C. They are applied for each of the values\n\
       returned by 0..99, which can be thought of as an implied external loop.\n\
       The trailing semicolon indicates evaluation for side-effects only, with\n\
       no output.  Duel  incorporates  C  operators,  casts  C  statements  as\n\
       expressions, and supports limited  variable declaration:\n\
\n\
       gdb> dl int i;for(i=0;i<100;i++)\n\
                        if(x[i]<0) printf(\"x[%%d]=%%d\\n\",i,x[i]);\n\
       x[7] = -4\n\
\n\
       The semicolon prevents Duel output; only output from printf is printed.\n\
       Aliases are defined with x:=y and provide an  alternative  to  variable\n\
       declaration. We could also return x[i] instead of using printf:\n\
\n\
       gdb> dl if(x[i:=0..99]<0) x[i]\n\
       x[i] = -4\n\
\n\
       The  symbolic output \"x[i]\" can be fixed by surrounding i with {}, i.e.\n\
\n\
       gdb> dl if(x[i:=0..99]<0) x[{i}]\n\
       x[7] = -4\n\
\n\
       The {} are like (), but force the symbolic evaluation to use i's value,\n\
       instead  of \"i\". You can usually avoid this altogether with direct Duel\n\
       operators:\n\
\n\
       gdb> dl x[..100] <? 0\n\
       x[7] = -4\n\
\n\
       The ..n operator is a shorthand for 0..n-1, i.e. ..100 is the  same  as\n\
       0..99.  The x<?y, x==?y, x>=?y, etc., operators compare their left side\n\
       operand to their right side operand as in C, but return the  left  side\n\
       value  if  the  comparison result is true. Otherwise, they look for the\n\
       next values to compare, without returning anything.\n\
\n\
       Duel's x.y and x->y allow an expression y, evaluated under x's scope:\n\
\n\
       gdb> dl emp[..100].(if(code>400) (code,name))\n\
       emp[46].code = 682\n\
       emp[46].name = \"Ela\"\n\
\n\
       The if() expression is evaluated under the scope  of  each  element  of\n\
       emp[], an array of structures. In C terms, we had to write:\n\
\n\
       gdb> dl int i; for(i=0; i<100 ; i++)\n\
             if(emp[i].code>400) emp[{i}].code,emp[{i}].name\n\
\n\
       A  useful  alternative  to loops is the x=>y operator. It returns y for\n\
       each value of x, setting `_' to reference x's value, e.g.\n\
\n\
       gdb> ..100 => if(emp[_].code>400) emp[_].code,emp[_].name\n\
\n\
       Using `_' instead of `i' also avoids the need  for  {i}.  Finally,  the\n\
       x-->y  operator expands lists and other data structures. If head points\n\
       to a linked list threaded through the next field, then:\n\
\n\
       gdb> dl head-->next->data\n\
       head->data = 12\n\
       head->next->data = 14\n\
       head-->next[[2]]->data = 20\n\
       head-->next[[3]]->data = 26\n\
\n\
       produce the data field for each node in  the  list.  x-->y  returns  x,\n\
       x->y,  x->y->y,  x->y->y->y,  ...  until a NULL is found.  The symbolic\n\
       output \"x-->y[[n]]\" indicates that ->y was applied n times.  x[[y]]  is\n\
       also the selection operator:\n\
\n\
       gdb> dl head-->next[[50..60]]->data\n\
\n\
       return the 50th through the 60th elements in the list. The #/x operator\n\
       counts the number of values, so\n\
\n\
       gdb> dl #/( head-->next->data >? 50 )\n\
\n\
       counts the number of data elements over 50 on the list.  Several  other\n\
       operators,  including  x@y,  x#y  and  active  call  stack  access  are\n\
       described in the operators section.\n\
\n\
OPERATORS SUMMARY\n\
       Assoc   Operators                   Details\n\
       left    {} () [] -> . f() -->       x-->y expands x->y x->y->y ...\n\
               x[[y]] x#y x@y              generate x; select, index or stop-at y\n\
       right   #/ - * & ! ~ ++ -- (cast)   #/x number of x values\n\
               frame(n) sizeof(x)          reference to call stack level n\n\
       left    x/y x*y x%%y                 multiply, divide, reminder\n\
       left    x-y x+y                     add, subtract\n\
       left    x<<y x>>y                   shift left/right\n\
       none    x..y ..y x..                ..y = 0..y-1. x..y return x, x+1...y\n\
       left    < > <= >= <? >? <=? >=?     x>?y return x if x>y\n\
       left    == != ==? !=?               x==?y return x if x==y\n\
       left    x&y                         bit-and\n\
       left    x^y                         bit-xor\n\
       left    x|y                         bit-or\n\
       left    x&&y &&/x                   &&/x are all x values non-zero?\n\
       left    x||y ||/x                   ||/x is any x value non-zero?\n\
       right   x? y:z                      foreach x, if(x) y else z\n\
       right   x:=y x=y x+=y ...           x:=y set x as an alias to y\n\
       left    x,y                         return x, then y\n\
       right   x=>y                        foreach x, evaluate y with x value `_'\n\
       right   if() else  while()  for()   C statements cast as operators\n\
       left    x;y                         evaluate and ignore x, return y\n\
\n\
\n\
EXAMPLES\n\
       dl (0xff-0x12)*3                    compute simple expression\n\
       dl (1..10)*(1..10)                  display multiplication table\n\
       dl x[10..20,22,24,40..60]           display x[i] for the selected indexes\n\
       dl x[9..0]                          display x[i] backwards\n\
       dl x[..100] >? 5                    display x[i] that are greater than 5\n\
       dl x[..100] >? 5 <? 10              display x[i] if 5<x[i]<10\n\
       dl x[..100] ==? (6..9)              same\n\
       dl x[0..99]=>if(_>5 && _<10) _      same\n\
       dl y[x[..100] !=? 0]                display y[x[i]] for each non-zero x[i]\n\
       dl emp[..50].code                   display emp[i].code for i=0 to 49\n\
       dl emp[..50].(code,name)            display emp[i].code & emp[i].name\n\
       dl val[..50].(is_dbl? x:y)          display val[i].x or val[i].y depending\n\
                                           on val[i].is_dbl.\n\
       dl val[..50].if(is_dbl) x else y    same as above\n\
       dl x[..100]=0 ;                     assign 0 to x[i]\n\
       dl x[i:=..100]=y[i] ;               assign y[i] to x[i]\n\
       dl x[..100]=y[..100] *ERR*          assign y[99] to each x[j]\n\
       dl x[i:=..3]=(4,5,9)[[i]]           assign x[0]=4 x[1]=5 x[2]=9\n\
       dl x[..3]=(4,5,9)    *ERR*          assign 9 to each element\n\
       dl if(x[i:=..100]<0) x[i]=0 ;       assign 0 to negative x[i]\n\
       dl (hash[..1024]!=?0)->scope        hash[i].scope for non-null hash[i]\n\
       dl x[i:=..100] >? x[i+1]            check if x[i] is not sorted\n\
       dl x[i:=..100] ==? x[j:=..100]=>    checks if x has non-unique elements\n\
          if(i<j) x[{i,j}]\n\
       dl if(x[i:=..99] ==                 same\n\
          x[j:=i+1..99]) x[{i,j}]\n\
       dl (x[..100] >? 0)[[0]]             the 1st (0th element) positive x[i]\n\
\n\
       dl (x[..100] >? 0)[[2]]             return the 3rd positive x[i]\n\
       dl (x[..100] >? 0)[[..5]]           return the first 5 positive x[i]\n\
       dl (x[0..] >? 6)[[0]]               return the first x[i]>6, no limit on i\n\
       dl argv[0..]@0                      argv[0] argv[1] .. until first null\n\
       dl x[0..]@-1 >? 9                   x[0..n]>9 where n is first x[n]== -1\n\
       dl emp[0..]@(code==0)               emp[0]..emp[n-1] where emp[n].code==0\n\
\n\
       dl head-->next->val                 val of each element in a linked list\n\
       dl head-->next[[20]]                the 21st element of a linked list\n\
       dl *head-->next[[20]]               display above as a struct\n\
       dl strcmp(head-->next->msg,         search linked list for a string\n\
            \"testing\") ==? 0\n\
       dl #/head-->next                    count elements on a linked list\n\
       dl x-->y[[#/x-->y - 1]]             last element of a linked list\n\
       dl x-->y[[#/x-->y - 10..1]]         last 10 elements of a linked list\n\
       dl head-->next->                    check if the list is sorted by val\n\
          if(next) val >? next->val\n\
\n\
       dl head-->(next!=?head)             expand cyclic linked list (tail->head)\n\
       dl head-->(next!=?_)                handle termination with p->next==p\n\
       dl root-->(left,right)->key         expand binary tree, show keys\n\
       dl root-->(left,right)->(           check bin tree sorted by key\n\
          (left!=?0)->key>=?key, (right!=?0)->key<=?key)\n\
\n\
       dl (1000..=>if(&&/(2,3.._-1=>__%%_   find first 10 primes over 1000\n\
                  ) _)[[..10]]\n\
       dl (T mytype) x                     convert x to user defined type mytype\n\
       dl (struct s*) x                    convert x to struct s pointer\n\
       dl if(x) y; else z *ERR*            ';' must be followed by an expression\n\
       dl {x} y *ERR*                      '}' requires ';' if followed by exp\n\
\n\
SEMANTICS\n\
       Duel's semantics are modeled after the Icon programming language.   The\n\
       input  consists  of  expressions  which  return sequences of values.  C\n\
       statements are cast as expressions, too.  Expressions are  parsed  into\n\
       abstract syntax trees, which are traversed during evaluation. The eval-\n\
       uation of most nodes (operators) recursively evaluates the  next  value\n\
       for  each  operand,  and  then applies the operator to produce the next\n\
       result. Only one value is produced each time, and Duel's eval  function\n\
       keeps a `state' for each node (backtracking, co-routines, consumer-pro-\n\
       ducer or threads are good metaphors for the evaluation mechanism.)\n\
\n\
       For example, in (5,3)+6..8, the evaluation of `+' first  retrieves  the\n\
       operands 5 and 6, to compute and return 5+6. Then 7, the next right op-\n\
       erand is retrieved and 5+7 is returned, followed by 5+8.   Since  there\n\
       are  no other right operand value, the next left operand, 3 is fetched.\n\
       The right operand's computation is restarted returning 6,  and  3+6  is\n\
       returned. The final return values are 3+7 and 3+8.\n\
\n\
       The  computation for operators like x>?y is similar, but when x<=y, the\n\
       next values are fetched instead of returning a value, forming the basis\n\
       for an implicit search. Operators like `..' return a sequence of values\n\
       for each pair of operands. For a better understanding of the evaluation\n\
       mechanism,  see  the  USENIX  Winter/93 conference paper \"DUEL - A Very\n\
       High Level Debugging Language\".\n\
\n\
       Duel values follow the C semantics. A value is either an \"lvalue\"  (can\n\
       be  used  as the left hand side of assignment), or an \"rvalue\".  There-\n\
       for, objects like arrays can  not  be  directly  manipulated  (However,\n\
       operators like x..y can accomplish such tasks.)\n\
\n\
       Duel  types  also  follow  the C semantics, with some important differ-\n\
       ences. C types are checked  statically; Duel  types  are  checked  when\n\
       operators  are  applied,  e.g., (1,1.0)/2 returns 0 (int) and 0.5 (dou-\n\
       ble); (x,y).z returns x.z and y.z even if x  and  y  are  of  different\n\
       types, as long as they both have a field z.\n\
\n\
       Values  and  types  of  symbols  are looked up at run-time (using gdb's\n\
       lookup rules), allowing dynamic scoping and types, but causing a  pars-\n\
       ing  problem:  (x)(y) can be parsed as either a function call x(y) or a\n\
       cast (x)y; x*y can be parsed as a declaration of y as (x*) or as multi-\n\
       plication.\n\
\n\
       To  avoid  this  ambiguity,  the  keyword T must precede a user defined\n\
       type. For example, if value is a typedef, C's (value (*)()) x is  writ-\n\
       ten  in  Duel  as:  (T value (*)()) x. Types that begin with a reserved\n\
       keyword don't need T, e.g. (struct value*) x  and  (long  *[5])  y  are\n\
       accepted.  As special cases, (type)x and (type*)x are accepted but dis-\n\
       couraged (it causes (printf)(\"hi\"), which is valid in C, to  fail).   A\n\
       side effect is that \"sizeof x\" must be written as sizeof(x).\n\
\n\
OPERATORS\n\
       x+y  x-y  x*y  x/y  x%%y  x^y  x|y  x&y  x<<y  x>>y\n\
       x>y  x<y  x>=y  x<=y  x==y  x!=y  x=y  x[y]\n\
\n\
       These  binary  operators follow their C semantics. For each value of x,\n\
       they are evaluated for every value of y, .e.g. (5,2)>(4,1) evaluates as\n\
       5>4,  5>1,  2>4, 2>1 returning  1, 1, 0, 1.  The y values are re-evalu-\n\
       ated for each new value of x, e.g.  i=4; (4,5)>i++ evaluates as 4>4 and\n\
       5>5.   Beware  of  multiple y values in assignment, e.g. x[..3]=(4,6,9)\n\
       does not set x[0]=4, x[1]=6 and x[2]=9. It assigns 4, 6 and 9  to  each\n\
       element, having the same effect as x[..3]=9. Use x[i:=..3]=(4,6,9)[[i]]\n\
       to achieve the desired effect.\n\
\n\
       -x  ~x  &x  *x  !x  ++x  --x  x++  x--  sizeof(x)  (type)x\n\
\n\
       These unary operators follow their C semantics.  They  are  applied  to\n\
       each  value  of  x.  The  increment  and decrement operators require an\n\
       lvalue, so i:=0 ; i++ produces an error because i is an alias to 0,  an\n\
       rvalue.  Parenthesis  must  be  used  with sizeof(x), \"sizeof x\" is not\n\
       allowed. Cast to user defined type requires generally requires T, e.g.,\n\
        (T val(*)())x, but (val)x and (val*)x are accepted as special cases.\n\
\n\
       x&&y   x||y\n\
\n\
       These  logical  operators  also follow their C semantics, but have non-\n\
       intuitive results for multi-valued x  and  y,  e.g.  (1,0,0)  ||  (1,0)\n\
       returns  1,1,0,1,0  --  the  right hand-side (1,0) is returned for each\n\
       left-hand side 0. It is best to use  these  operators  only  in  single\n\
       value expressions.\n\
\n\
       x? y:z   if(x)y   if(x)y else z\n\
\n\
       These  expressions  return  the  values  of  y  for each non-zero value\n\
       returned by x, and the values of z for each zero value returned  by  x,\n\
       e.g.   if(x[..100]==0)  y  returns y for every x[i]==0, not if all x[i]\n\
       are zero (if(&&/(x[..100]==0)) y  does that).  Also, \"if(x) y; else  z\"\n\
       is illegal. Duel's semicolon is an expression separator, not a termina-\n\
       tor.\n\
\n\
       while(x)y   for(w;x;y)z\n\
\n\
       The while(x)y expression returns y as long as all values of x are  non-\n\
       zero.   The  for()  expression  is similar and both have the expected C\n\
       semantics. For example, \"for(i=0 ; i<100 ; i++) x[i]\" is  the  same  as\n\
       x[..100].  Unlike  the  if() expression, while(x[..100]==0) continue to\n\
       execute only if all elements of x are zero, i.e. the condition is eval-\n\
       uated into a single value using an implicit &&/x.\n\
\n\
       Variable declaration:  type name [,name ...] ; ...\n\
\n\
       Expressions  can  begin with variables declaration (but not initializa-\n\
       tion).  Internally, a declaration sets an alias to space  allocated  in\n\
       the  target by calling malloc(), e.g. `int x' is the same as \"x:= *(int\n\
       *) malloc(sizeof(int))\". This is oblivious to the user.  The  allocated\n\
       memory  is  not  claimed when a variable is redeclared.  Declared vari-\n\
       ables addresses can be passed to  functions  and  used  in  other  data\n\
       structures.  The keyword `T' must precede user defined types (typedef),\n\
       e.g. if val is a user defined  type,  The  C  code  \"val  *p=(val*)  x\"\n\
       becomes \"T val *p; p=(T val *) x\" in Duel.\n\
\n\
       Function calls:  func(parm,...)\n\
\n\
       Function  calls  to  the  debugged  program can be intermixed with Duel\n\
       code. Multi-valued parameters are handled  as  with  binary  operators.\n\
       The  call  value  can  have multiple values, e.g. (x,y)() calls x() and\n\
       y(). Currently, struct/union parameters and return values are not  sup-\n\
       ported.\n\
\n\
       x,y   x..y   ..x    x..\n\
\n\
       These operators produce multiple values for single value operands.  x,y\n\
       returns x, then y. x..y returns the integers from x to y.  When x>y the\n\
       sequence  is  returned  in descending order, i.e. 5..3 returns 5, 4, 3.\n\
       The operator ..x is a shorthand for 0..x-1, e.g. ..3 returns 0,  1,  2.\n\
       The  x..  operator  is a shorthand for x..maxint. It returns increasing\n\
       integer values starting at x indefinitely, and  should  be  bounded  by\n\
       [[n]]  or  @n  operators.   `,'  retains its precedence level in C. The\n\
       precedence of `..'  is above `<' and  below  arithmetic  operators,  so\n\
       0..n-1 and x==1..9 work as expected.\n\
\n\
       x<?y  x>?y  x>=?y  x<=?y  x!=?y  x==?y\n\
\n\
       These operators work like their C counterparts but return x if the com-\n\
       parison is true. If the comparison is false, the next  (x,y)  value  is\n\
       tried, forming the basis of an implicit search.\n\
\n\
       (x)  {x}  x;y  x=>y\n\
\n\
       Both () and {} act as C parenthesis.  The curly braces set the returned\n\
       symbolic value as the actual value, e.g. if i=5 and x[5]=3,  then  x[i]\n\
       produces  the  output \"x[i] = 3\", x[{i}] produces \"x[5] = 3\" and {x[i]}\n\
       produces just \"3\".  The semicolon is  an  operator.  x;y  evaluates  x,\n\
       ignoring  the results, then evaluate and return y, e.g. (i:=1..3 ; i+5)\n\
       sets i to 3 and return 8.  The x=>y operator evaluate and return y  for\n\
       each  value  of  x, e.g. (i:=1..3 => i+5) returns 6, 7 and 8. The value\n\
       returned by x is also stored implicitly in `_' which can be used in  y,\n\
       e.g.  1..5  =>  z[_][_]  will output z[1][1], z[2][2] etc. The symbolic\n\
       value for _ is that of the left side value, hence {_} is not needed.\n\
       Semicolon has the lowest precedence, so it must be used inside () or {}\n\
       for  compound  expressions.  The  precedence of `=>' is just below `,'.\n\
       Beware that \"if(a) x; else {y;} z\"  is  illegal;  a  semicolon  is  not\n\
       allowed before '}' or 'else' and must be inserted before z.\n\
\n\
       x->y   x.y\n\
\n\
       These expression work as in C for a symbol y. If y is an expression, it\n\
       is evaluated under the scope of x. e.g. x.(a+b) is the same as x.a+x.b,\n\
       if a and b are field of x (if they are not, they are looked up as local\n\
       or global variables). x may return multiple values of different  types,\n\
       e.g.  (u,v).a returns u.a and v.a, even if u and v are different struc-\n\
       tures.  Also, the value of  x  is  available  as  `_'  inside  y,  e.g.\n\
       x[..100].(if(a)  _)  produces  x[i]  for each x[i].a!=0. Nested x.y are\n\
       allowed, e.g.  u.(v.(a+b)) would lookup a and b  first  under  v,  then\n\
       under u.\n\
\n\
       Aliases:  x:=y\n\
\n\
       Aliases  store  a  reference  to  y  in  x.  Any reference to x is then\n\
       replaced by y. If y is a constant or an rvalue, its value  is  replaced\n\
       for  x. If y is an lvalue (e.g. a variable), a reference to same lvalue\n\
       is returned. for example, x:=emp[5] ; x=9 assigns 9 to emp[5].  Aliases\n\
       retain  their values across invocation of the \"dl\" command. An alias to\n\
       a local variable will reference a stray address when the variable  goes\n\
       out  of  scope.  The special command \"dl clear\" delete all the aliases,\n\
       and \"dl alias\" show all current  aliases.  Symbols  are  looked  up  as\n\
       aliases first, so an alias x will hide a local x.\n\
\n\
       x-->y\n\
\n\
       The expansion operator x-->y expands a data structure x following the y\n\
       links.  It returns x, x->y, x->y->y, until a null is  found.  If  x  is\n\
       null,  no  values  are produced. If y returns multiple values, they are\n\
       stacked and each is further expanded in a depth-first notion. For exam-\n\
       ple,  if r is the root of a tree with children u->childs[..u->nchilds],\n\
       then u-->(childs[..nchilds]) expands the whole tree. y is an  arbitrary\n\
       expression, evaluated exactly like x->y (this includes `_'.)\n\
\n\
       x@y\n\
\n\
       The expression x@y produces the values of x until x.y is non-zero, e.g.\n\
       for(i=0 ; x[i].code!= -1 &&  i<100  ;  i++)  x[i]  can  be  written  as\n\
       x[..100]@(code==-1).  The evaluation of x is stopped as soon as y eval-\n\
       uates to true.  x->y or x=>y are used to evaluate y when  x  is  not  a\n\
       struct  or  a  union.  If y is a constant,(_==y) is used. e.g. s[0..]@0\n\
       produces the characters in string s up to but not including the  termi-\n\
       nating null.\n\
\n\
       #/x   &&/x   ||/x\n\
\n\
       These  operator  return  a  single  \"summary\"  value for all the values\n\
       returned by x. #/x returns the number of values  returned  by  x,  e.g.\n\
       #/(x[..100]>?0)  counts  the number of positive x[i]. &&/x returns 1 if\n\
       all the values produced by x are non-zero, and ||/x returns 1 if any of\n\
       x's  values  are  non-zero.  Like in C, the evaluation stops as soon as\n\
       possible.  For example, ||/(x[..100]==0) and &&/(x[..100]==0) check  if\n\
       one or all of x[i] are zero, respectively.\n\
\n\
       x#y  x[[y]]\n\
\n\
       The  operator  x#y produces the values of x and arranges for y to be an\n\
       alias for the index of each value in x. It is commonly used with  x-->y\n\
       to produce the element's index, e.g. head-->next->val#i=i  assigns each\n\
       val field its element number in the list.\n\
       The selection operator x[[y]] produces  the  yth  result  of  x.  If  y\n\
       returns   multiple   value,   each   select   a   value   of   x,  e.g.\n\
       (5,7,11,13)[3,0,2] returns 13, 5 and 11 (13 is the 3rd  element,  5  is\n\
       the  0th  element).   Don't use side effects in x, since its evaluation\n\
       can be restarted depending on  y,  e.g.  after  (x[0..i++])[[3,5]]  the\n\
       value of i is unpredictable.\n\
\n\
       frame(n)   frames_no   func.x\n\
\n\
       frame(n)  for  an integer n returns a reference to the nth frame on the\n\
       stack (0 is the inner most function and frame(frames_no-1) is  main()).\n\
       Frame   values   can   be   compared   to   function   pointers,   e.g.\n\
       frame(3)==myfunc is true if the 4th frame is a call to myfunc,  and  in\n\
       scope  resolution,  e.g.  frame(3).x return the local variable x of the\n\
       4th frame.  frames_no is the number of active frames on the stack, e.g.\n\
       (frames(..frames_no)  ==?  myfunc).x  displays x for all active invoca-\n\
       tions of myfunc. As a  special  case,  (frames(..frames_no)==?f)[[0]].x\n\
       can be written as f.x (x can be an expression).\n\
\n\
\n\
BUGS\n\
       Both  `{}'  and `;' are operators, not statements or expression separa-\n\
       tors; \"if(x) y; else {z;} u\" is illegal; use \"if(x) y else  {z}  ;  u\".\n\
       Ambiguities  require  preceding  user-defined  types (typedef) with the\n\
       keyword T, e.g., if value is a  user  type,  C's  \"sizeof(value*)\"   is\n\
       written  \"sizeof(T  value*)\",  except for the casts \"(t)x\" and \"(t*)x\";\n\
       sizeof(x) requires parenthesis for variable x.\n\
\n\
       Unimplemented C  idiom  include:  modified-assignment  (x+=y),  switch,\n\
       break,  continue, do, goto, scopes, function declarations, initializing\n\
       declared variables, assignment to bit-fields  and  register  variables,\n\
       and  calling  functions  with a struct/union parameter or return value.\n\
       gdb does not store function prototypes, so parameters are not  checked.\n\
\n\
       Gdb  itself  is  buggy, which shows up, especially in symbol tables and\n\
       calling target functions. Before you report bug, try to do the  closest\n\
       thing under gdb's \"print\". Send bug to: mg@cs.princeton.edu.\n\
\n\
FILES\n\
       duel.out tracks duel commands usage. Help analyze duel's use by mailing\n\
       a copy to mg@cs.princeton.edu.\n\
       Duel is available by anonymous ftp at ftp.cs.princeton.edu:/duel.\n\
\n\
AUTHOR\n\
       Duel is public domain code -- no copy left or right. See the  internals\n\
       documentation for details on porting Duel and using its code.  Duel was\n\
       designed and written by Michael Golan as part of a PhD  thesis  in  the\n\
       Computer  Science  Department of Princeton University.  I would like to\n\
       thank my advisor, Dave Hanson, who helped in all phases of this project\n\
       and to Matt Blaze for his support and useful insight.\n\
\n\
       Duel stands for Debugging U (might) Even Like, or Don't Use this Exotic\n\
       Language. Judge for yourself!\n\
\n\
Version 1.10                        Mar 93                             Duel(1)\n\n");
}

LPROC help(void)
{
        duel_printf(
"Duel - Debugging U (might) Even Like -- A high level debugging language\n\n\
Duel was designed to overcome problems with traditional debuggers' print\n\
statement. It supports the C operators, many C constructs, and many new\n\
operators for easy exploration of the program's space, e.g.\n\
x[..100] >? 0                 show positive x[i] for i=0 to 99\n\
y[10..20].code !=? 0          show non-zero y[i].code for i=10 to 20\n\
h-->next->code                expand linked list h->next, h->next->next ...\n\
head-->next.if(code>0) name   show name for each element with code>0\n\
x[i:=..100]=y[i];             array copy. i is an alias to vals 0..99\n\
head-->next[[10..15]]         the 10th to 15th element of a linked list\n\
#/(head-->next->val==?4)      count elements with val==4\n\
head-->next->if(next) val >? next->val    check if list is sorted by val\n\
\n\
Duel was written by Michael Golan at Princeton University. Send email to\n\
mg@cs.princeton.edu. Duel is public domain code. No copy left or right.\n\
all but 500 lines are independent of gdb. Port it! Make it Commercial!\n\
\n\
Try \"dl ops\" for op summary; \"dl\" alone lists all commands\n");
}

LPROC examples(void)
{
    duel_printf("\
x[10..20,22,24,40..60]    display x[i] for the selected indexes\n\
x[9..0]                   display x[i] backwards\n\
x[..100] >? 5 <? 10       display x[i] if 5<x[i]<10\n\
x[0..99]=>if(_>5 && _<10) _     same\n\
val[..50].if(is_dx) x else y   \
val[i].x or val[i].y depending on val[i].is_dx\n\
emp[..50].if(is_m) _      return emp[i] if emp[i].is_m.\n\
x[i:=..100]=y[i] ;        assign y[i] to x[i]\n\
x[i:=..100] >? x[i+1]     check if x[i] is not sorted\n\
(x[..100] >? 0)[[2]]      return the 3rd positive x[i]\n\
argv[0..]@0               argv[0] argv[1] .. until first null\n\
emp[0..]@(code==0)        emp[0]..emp[n-1] where emp[n].code==0\n\
head-->next->val          val of each element in a linked list\n\
*head-->next[[20]]        element 20 of list, '*' display struct w/fields\n\
#/head-->next             count elements on a linked list\n\
#/(head-->next-val>?5)    count those over 5\n\
head-->(next!=?head)      expand cyclic linked list (tail->head)\n\
T mytype x ;              declare var for user defined type (need 'T')\n\
int i ; for(i=0 ;i<5 ..   declare variable, use C construct.\n");
}

LPROC operators(void)
{
    duel_printf("\
DUEL operators in decreasing precedence. All C operators are accepted!\n\
note: precede typedefs by 'T', eg \"sizeof(T uint)\"\n\n\
{x}     same as (x) but x's value is used for symbol                emp[{i}]\n\
x-->y   expands data structure from x, using y      root-->(left,right)->key\n\
x.y     eval y under x's scope, like pascal \"with\". x is accesible as '_'\n\
x->y    same as (*x).y.                             hash[..50]->if(_!=0) key\n\
x[[y]]  select the y'th elements of x                     head-->next[[..6]]\n\
x@y     eval x, stop as soon as y true (_==y if y const).        argv[0..]@0\n\
x#y     eval x, set alias y as values counter        (x-->next#n)->level==?n");
    duel_printf("\n\
#/x     count number of values from x                          #/head-->next\n\
frame(n) use with '.' to reference stack frames.                frame(3).val\n\
x..y    x, x+1, x+2 .. y. (if x>y, return them backwards)          x[10..20]\n\
..y     like 0..y-1                                                x[..1024]\n\
x..     like x..maxint. Caution: use only with x@y or x[[y]]     name[0..]@0\n\
x>?y    x if x>y else nothing. also <? <=? >=? ==? and !=?        x[..10]<?0\n\
&&/x    1 or 0 if x!=0 for all x values. ||/x similar         &&/x[..100]>=0\n\
x,y     x, then y.				      head-->next->(val,key)\n\
x=>y    eval y for each x, setting '_' to x's values            x[..50]=>_*_\n\
if(x) y C statements are operators. Also for(), while(), if() else\n\
x;y     Evaluate and ignore x's value, then return y\n");
}

/* entry point into duel. s is the expression to evaluate */

void duel_parse_and_eval(char *s)
{
   static int first=1 ;
   if(first)  {                 /* init stuff */
      duel_init_basic_ctypes();
      duel_printf("%s.%d, public domain debugging language. \"dl\" for help\n",
               VERSION,PATCHLEVEL);
      duel_redirectable_output_init();
      first=0 ;
   }
   if(!s || *s==0) {  /* no input, give some help */
       duel_printf("\
Supported DUEL commands:\n\
duel help      - give basic help (shortcut: dl ?)\n\
duel longhelp  - give a longer help (dl ?\?)\n\
duel examples  - show useful usage examples (dl ex)\n\
duel operators - operators summary (dl ops)\n\
duel aliases   - show current aliases (dl alias)\n\
duel clear     - clear all aliases\n\
duel debug     - toggle duel debug mode\n");
       return ;
   }
   if(strcmp(s,"?")==0 || strcmp(s,"help")==0) {
       help();
       return ;
   }
   if(strcmp(s,"?\?")==0 || strcmp(s,"longhelp")==0) {
       longhelp();
       return ;
   }
   if(strcmp(s,"examples")==0 || strcmp(s,"ex")==0 ) {
       examples();
       return ;
   }
   if(strcmp(s,"operators")==0 || strcmp(s,"ops")==0) {
       operators();
       return ;
   }
   if(strcmp(s,"debug")==0) {           /* turn debugging of duel itself */
       duel_debug= !duel_debug ;
       duel_printf("duel debug mode %d\n",duel_debug);
       return ;
   }
   if(strcmp(s,"clear")==0) {
       duel_clear_aliases();
       duel_printf("Aliases table cleared\n");
       return ;
   }
   if(strcmp(s,"alias")==0 || strcmp(s,"aliases")==0) {
       duel_show_aliases();
       return ;
   }
   if(setjmp(duel_abort_jmp)==0) {              /* set abort point */
     if((root=duel_parse(s))!=NULL) {
       tvalue v ;
       duel_set_input_string(s);	  /* for src-location err management */
       duel_reset_eval();
       duel_redirectable_output_start(s); /* allow eval output to go to pipe */
       while(duel_eval(root,&v)) {
           duel_print_value(&v);
	   duel_flush();
       }
       duel_redirectable_output_end();
     }
   }
   duel_cleanup(0);
}










