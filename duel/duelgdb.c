/*   DUEL - A Very High Level Debugging Langauge.  */
/*   Public domain code                            */
/*   Written by Michael Golan mg@cs.princeton.edu  */
/*$Header: /tmp_mnt/n/fs/grad2/mg/duel/RCS/duelgdb.c,v 1.20 93/03/20 10:35:48 mg Exp $*/

/* debugger dependent module, it contains all of duel's access to
 * the outside world (debuggee, symbol table, etc)
 */

/*
 * $Log:	duelgdb.c,v $
 * Revision 1.30  08/11/14  23:14:00  serg
 * ported to gdb 6.6.2, glibc 2.5
 *
 * Revision 1.20  93/03/20  10:35:48  mg
 * fixed system("date")... didnt work on OS/2
 *
 * Revision 1.19  93/03/13  03:28:49  mg
 * bug fixed frame(0).unknown_name caused a crash
 * bug fixed duel didnt support enum of length zero but turns out gdb tables
 * have such things (specifically, for some gdb internals!)
 *
 * Revision 1.18  93/03/12  05:43:22  mg
 * Version 1.10 - tuint instead of uint etc, fixed gdb48 recognis. problem
 *
 * Revision 1.17  93/02/27  06:01:01  mg
 * improved unsigned char default machines support,
 * convert explicit uchar into "char" if they are the same.
 *
 * Revision 1.16  93/02/23  19:11:10  mg
 * beauty changes + gdb4.8 support
 *
 * Revision 1.15  93/02/04  02:09:18  mg
 * fixed enum enum names
 *
 * Revision 1.14  93/02/04  00:54:26  mg
 * arr. fixed.
 *
 * Revision 1.13  93/02/03  21:54:14  mg
 * create duel.out unless compiling with NO_DUEL_OUT
 *
 * Revision 1.12  93/02/03  21:46:33  mg
 * fixed problems with null gdb type names.
 * support "signed char"
 *
 * Revision 1.11  93/01/21  21:22:15  mg
 * *** empty log message ***
 *
 * Revision 1.10  93/01/13  16:19:33  mg
 * support mini symbol table lookup (malloc, printf on SUN didnt work)
 *
 * Revision 1.9  93/01/12  21:30:04  mg
 * cleanup and set for release
 *
 * Revision 1.8  93/01/06  23:59:21  mg
 * new memory alloc/release. moved target's malloc into duel code
 * allow ^c to work, fixed help, fixed variable lookup for specific frame.
 *
 * Revision 1.7  93/01/03  07:27:11  mg
 * support function calls!
 *
 * Revision 1.6  92/12/24  23:33:13  mg
 * frames support
 *
 * Revision 1.5  92/10/19  15:06:29  mg
 * made lcc happy
 * no svalues
 * new registers support and way of getting vars.
 * these are temporary changes, new frames supports soon
 *
 * Revision 1.4  92/10/14  02:03:53  mg
 * misc/gdb4.6/duel vars using malloc
 *
 * Revision 1.3  92/09/16  11:06:22  mg
 * a lot more functions: get typedef/tags, alloc debuggee mem. +cosmetics
 *
 * Revision 1.2  92/09/15  05:37:28  mg
 * fix enum size bug. added headers
 *
 */


#include <stddef.h>   /* used for ptrdiff_t and size_t */
#include <stdio.h>
#include <string.h>
#include <time.h>     /* for time/ctime stamping of duel.out */
#include <assert.h>

#include "defs.h"
#include "symtab.h"
#include "gdbtypes.h"
#include "gdbcore.h"
#include "frame.h"
#include "target.h"
#include "value.h"
#include "symfile.h"
#include "objfiles.h"
#include "gdbcmd.h"
#include "call-cmds.h"
#include "infcall.h"

#include "duel/global.h"
#include "duel/proto.h"

/* all output goes thru here */

#include <stdarg.h>

PROC duel_printf(const char *fmt, ...)    /* like printf, but for duel output */
{
  va_list args;
  va_start(args, fmt);
  vprintf_filtered(fmt,args);
  va_end(args);
}

PROC duel_flush(void)
{ /* not yet. only really needed for redirections */
}

PROC duel_redirectable_output_start(char *s)
{
}

PROC duel_redirectable_output_end(void)
{
}

PROC duel_redirectable_output_abort(void)
{
}

PROC duel_redirectable_output_init(void)
{
}

/* all duel memory allocations go thru here */

FUNC void* duel_malloc(size_t size)
{
  void *p ;
  immediate_quit-- ;    /* disable ^C while xmalloc */
  p=xmalloc(size);      /* gdb's malloc, incl zero return chk */
  immediate_quit++ ;
  QUIT ;                /* check if ^C while xmalloc called */
  return p ;
}

PROC duel_free(void *p)
{
  immediate_quit-- ;
  free(p);
  immediate_quit++ ;
  QUIT ;
}

/* fetch n bytes from the target at the given memory address.
 * the address to fetch from is given by (from).
 * the value is stored at the 'to' location, which points to space for
 * n bytes in the debugger.
 * if the address can not be accessed, false is returned (if all ok, ret true)
 */

FUNC bool duel_get_target_bytes(ttarget_ptr from,void *to,size_t n)
{
  return target_read_memory((CORE_ADDR)from,to,n)==0;
}

/* store n bytes to the debuggee. reverse parms from above */
FUNC bool duel_put_target_bytes(ttarget_ptr to,void *from,size_t n)
{
   return target_write_memory((CORE_ADDR)to,from,n)==0;
}

/* fetch the value of a bitfield of a given structure.
 * struct_at - a pointer to the structure
 * bitpos    - the position of the field, in bits (as given in the type)
 * bitlen    - the length of the field in bits
 * to        - pointer to tuint  where value will be stored.
 * tkind     - the type kind of the field (used to do sign extention if needed)
 * note: I'm unaware of any compiler with bitfields that are not int/tuint.
 */

FUNC bool duel_get_target_bitfield(ttarget_ptr struct_at,int bitpos,
                                    int bitlen,void *to,tctype_kind tkind)
{
  tuint bits ;
  duel_assert(tkind==CTK_INT || tkind==CTK_UINT);
  if(!duel_get_target_bytes(struct_at+bitpos/8,&bits,sizeof(bits)))
      return FALSE;

  /* now, move the field to the 'bottom' of bits, by shifting right */
  bitpos%=8 ;                   /* fix it to a bit offset inside the 'bits' */
  if (BITS_BIG_ENDIAN)             /* defined by gdb's src code */
    bits >>= (sizeof(bits)*8-bitpos-bitlen) ;
  else
    bits >>= bitpos ;

 /* finally chop down 'bits' to only bitlen significant bits,
  * or sign-extend it if output field is 'int' and the sign is 1.
  * ~0 is all ones, shift it to have bitlen zeros. Complement to get
  * a bitlen string of 1's in the low-order bits. Common ugly hacks.
  * Note: this code assume 2's complement
  */

  if(tkind==CTK_INT && (bits & 1<<(bitlen-1))) /* int field, negative sign */
         bits|= ~0<<bitlen ;    /*sign extend */
  else   bits&= ~(~0<<bitlen) ; /* clear all bits above the sign bit */
  *(tuint*) to= bits ;

  return TRUE ;
}

LFUNC struct type* duel_convert_type_to_gdb(tctype *ct);
LFUNC tctype* duel_convert_type_from_gdb(struct type *gv);

/* convert gdb value into duel's. Used to fetch const, registers, func ret val
 * and convert a symbol into a reference.
 * otherwise, duel access target's memory directly.
 */

LPROC duel_convert_value_from_gdb(struct value *gv, tvalue *dv)
{
  dv->ctype=duel_convert_type_from_gdb(value_type(gv));
  if(VALUE_LVAL(gv) == lval_memory) {
      dv->val_kind=VK_LVALUE ;
      dv->u.lvalue= (ttarget_ptr) VALUE_ADDRESS(gv) ; /*address of variable*/
  }
  else
  if(ctype_kind_scalar(dv->ctype) && (VALUE_LVAL(gv) == not_lval ||
     VALUE_LVAL(gv) == lval_register)) {
      dv->val_kind=VK_RVALUE ;
      duel_bcopy(&dv->u,value_contents_raw(gv),dv->ctype->size);
  }
  else
  duel_fatal("unsupported value type returned by gdb");
}


/* helper function converts duel values to gdb's values. used only to make
 * function calls to the target, so current support is weak.
 * gdb thinks long=int=pointers, so we go along with that.
 * To support arbitrary types like struct/union, we can probably fake it
 * by making a gdb lvalue. but i am not sure how/what
 */

LFUNC struct value *convert_duel_val_to_gdb_val(tvalue *v)
{
   struct value *gv ;
   if(v->val_kind!=VK_RVALUE) return 0 ; /* cant handle lvals */
   switch(v->ctype->type_kind) {
/* duel standardize func parms, so this code is not required. if this
 * function is used for more than func parms, as its name implies, we will
 * need to do better than this.
 * there is no gdb "builtin_type_signed_char", so we assume char
   case CTK_CHAR:
     gv=value_from_longest(builtin_type_char,v->u.rval_char);          break;
   case CTK_SCHAR:
     gv=value_from_longest(builtin_type_char,v->u.rval_schar);         break;
   case CTK_UCHAR:
     gv=value_from_longest(builtin_type_unsigned_char,v->u.rval_char); break;
   case CTK_USHORT:
     gv=value_from_longest(builtin_type_unsigned_short,v->u.rval_ushort);break;
   case CTK_SHORT:
     gv=value_from_longest(builtin_type_unsigned_short,v->u.rval_short);break;
*/
   case CTK_INT:
     gv=value_from_longest(builtin_type_int, v->u.rval_int);            break;
   case CTK_UINT:
     gv=value_from_longest(builtin_type_unsigned_int, v->u.rval_uint) ; break ;
   case CTK_LONG:
     gv=value_from_longest(builtin_type_long, v->u.rval_long)         ; break ;
   case CTK_ULONG:
     gv=value_from_longest(builtin_type_unsigned_long,v->u.rval_ulong); break ;
   case CTK_LONGLONG:
     gv=value_from_longest(builtin_type_long_long, v->u.rval_longlong)     ; break ;
   case CTK_ULONGLONG:
     gv=value_from_longest(builtin_type_unsigned_long_long,v->u.rval_ulonglong); break ;
   case CTK_FLOAT:
     gv=value_from_double(builtin_type_float, v->u.rval_float) ; break ;
   case CTK_DOUBLE:
     gv=value_from_double(builtin_type_double, v->u.rval_double) ; break ;
   case CTK_PTR:
     gv=value_from_longest(lookup_pointer_type(builtin_type_void),
                        (long) v->u.rval_ptr) ;break ;
   default: duel_assert(0);
   }
   return gv ;
}

/* make a function call to the target.
 * this is the only case where we convert duel tvalue into gdb's values.
 * gdb thinks long=int=pointers, so we go along with that.
 */

PROC duel_target_func_call(tvalue *func, tvalue *parms[],
                            int parms_no,tvalue *rval)
{
    struct value *gfunc, *grval, *gparms[20] ;
    int i ;
    struct type *gftype ;

    duel_assert(func->val_kind==VK_LVALUE);
    for(i=0 ; i<parms_no ; i++) {
        gparms[i]=convert_duel_val_to_gdb_val(parms[i]) ;
        if(gparms[i]==NULL)
            duel_op_error("unsupported func call parm type",0,parms[i],0);
    }
    gftype =duel_convert_type_to_gdb(func->ctype);
    if(!gftype) duel_op_error("unsupported func return parm type",0,func,0);
    gftype = lookup_pointer_type(gftype);
    if(!gftype)
            duel_op_error("unsupproted func return type",0,func,0);

    gfunc = value_from_longest(gftype,(LONGEST) func->u.lvalue);

    grval=call_function_by_hand(gfunc,parms_no,gparms);
    if(func->ctype->u.kid->type_kind==CTK_VOID) return ; /* no return val*/
    duel_convert_value_from_gdb(grval,rval);
}



#define TYPE_HASH_SIZE (1024*128)
#define type_hash_func(t) ( (((long)t&0xffff) + (((long)t>>16)&0xffff)) \
                             % TYPE_HASH_SIZE )
struct {
    struct type *t ;    /* gdb type ptr */
    tctype *ct ;        /* duel type ptr */
} duel_thash[TYPE_HASH_SIZE] ;

LPROC duel_add_hash(struct type *t, tctype *ct)
{
    int start,i=type_hash_func(t);
    start=i ;
    do {
        if(duel_thash[i].t==0) {
            duel_thash[i].t=t ;
            duel_thash[i].ct=ct ;
            return ;
        }
        if(duel_thash[i].t==t) {
            if(duel_thash[i].ct==ct) return ;
            duel_fatal("type hash table inconsistency ");
        }
        i= (i+1)%TYPE_HASH_SIZE ;
    } while(i!=start);
    duel_fatal("type hash table is full");
}

LFUNC tctype* duel_find_hash(struct type *t)
{
    int start,i=type_hash_func(t);
    start=i ;
    do {
        if(duel_thash[i].t==0) break ;
        if(duel_thash[i].t==t) return duel_thash[i].ct ;
        i= (i+1)%TYPE_HASH_SIZE ;
    } while(i!=start);
    return NULL ;
}

/* a simple conversion back to gdb types, used only for target func calls */
/* this is a hack and based on the assumption that the type was first
 * converted FROM gdb to duel. (turn out to be false for int func from
 * minimal symbols, so we do this as a special case.)
 */
LFUNC struct type* duel_convert_type_to_gdb(tctype *ct)
{
   int i ;
   for(i=0 ; i<TYPE_HASH_SIZE ; i++)
       if(duel_thash[i].ct==ct) return duel_thash[i].t ;

   if(ct->type_kind==CTK_FUNC && ct->u.kid->type_kind==CTK_INT)
	  return  lookup_function_type (builtin_type_int);
   return NULL ;
}

#define INTTYPE_CHAR                    1
#define INTTYPE_SHORT                   2
#define INTTYPE_INT                     0
#define INTTYPE_LONG                    4
#define INTTYPE_SIGNED                  0
#define INTTYPE_UNSIGNED               16

struct { char *name; int len; int codetype; } name_to_codetype[]=
{ {"char", 4, INTTYPE_CHAR },
  {"short", 5, INTTYPE_SHORT },
  {"int", 3, INTTYPE_INT },
  {"long", 4, INTTYPE_LONG },
  {"signed", 6, INTTYPE_SIGNED },
  {"unsigned", 8, INTTYPE_UNSIGNED },
  {0,0,0}
};

  /* given a gdb type t, return an equivalent duel type */

LFUNC tctype* duel_convert_type_from_gdb(struct type *t)
{
  tctype *ct=duel_find_hash(t);
  if(ct) return ct ;

  if (duel_debug)
    duel_printf("gdb type: {%d, %s}\n", TYPE_CODE(t), TYPE_NAME(t));

  switch (TYPE_CODE (t)) {
   case TYPE_CODE_BOOL:
      if(strcmp(TYPE_NAME(t),"bool")==0) ct=ctype_uchar ; /* XXX HACK */
      break;
   case TYPE_CODE_INT: {
      int i, type=0;
      const char *s=TYPE_NAME(t);
      while (*s)
        for (i=0; name_to_codetype[i].name; i++) {
          if (strncmp(s,name_to_codetype[i].name,name_to_codetype[i].len) == 0) {
            s+=name_to_codetype[i].len;
            if (*s == ' ') s++;
            type+= name_to_codetype[i].codetype;
            break;
          }
        }
      if(type == INTTYPE_CHAR) ct=ctype_char ;
      else if(type == INTTYPE_CHAR + INTTYPE_UNSIGNED) ct=ctype_uchar ;
      else if(type == INTTYPE_SHORT) ct=ctype_short ;
      else if(type == INTTYPE_SHORT + INTTYPE_UNSIGNED) ct=ctype_ushort ;
      else if(type == INTTYPE_INT) ct=ctype_int ;
      else if(type == INTTYPE_UNSIGNED)   ct=ctype_uint ;
      else if(type == INTTYPE_LONG) ct=ctype_long ;
      else if(type == INTTYPE_UNSIGNED + INTTYPE_LONG)  ct=ctype_ulong ;
      else if(type == INTTYPE_LONG + INTTYPE_LONG) ct=ctype_longlong ;
      else if(type == INTTYPE_UNSIGNED + INTTYPE_LONG + INTTYPE_LONG)  ct=ctype_ulonglong ;
      break;
     }
   case TYPE_CODE_FLT:
      if(strcmp(TYPE_NAME(t),"float")==0)  ct=ctype_float ;
      else
      if(strcmp(TYPE_NAME(t),"double")==0) ct=ctype_double ;
      break;
   case TYPE_CODE_VOID:
      if(strcmp(TYPE_NAME(t),"void")==0) ct=ctype_void ;
      break;
   case TYPE_CODE_PTR:
   case TYPE_CODE_REF:
      {
       /* the pointer might get defined when converting the target, so
        * check the hashing again (reason: self-referencing structs)
        */
        tctype *target=duel_convert_type_from_gdb(TYPE_TARGET_TYPE(t));
        if((ct=duel_find_hash(t))==NULL) ct=duel_mkctype_ptr(target);
      }
      break ;
   case TYPE_CODE_FUNC:
      ct=duel_mkctype_func(duel_convert_type_from_gdb(TYPE_TARGET_TYPE(t)));
      break ;
   case TYPE_CODE_ARRAY:
      { int n=TYPE_LENGTH(TYPE_TARGET_TYPE(t));
        if(n!=0) n=TYPE_LENGTH(t)/n;
        ct=duel_mkctype_array(
               duel_convert_type_from_gdb(TYPE_TARGET_TYPE(t)),n);
      }
      break;
   case TYPE_CODE_STRUCT:
   case TYPE_CODE_UNION:
      { int i,n=TYPE_NFIELDS(t);
        const char *name=TYPE_NAME(t);
	if(name == NULL) name="" ; /* duel can't handle null ptr! */
        if(strncmp(name,"struct ",7)==0) name+=7 ;
        if(strncmp(name,"union ",6)==0) name+=6 ;
        ct=duel_mkctype_struct(name,TYPE_LENGTH(t),n,
                        TYPE_CODE(t)==TYPE_CODE_UNION);
        duel_add_hash(t,ct);  /* so a pointer to myself is recognized */
        for(i=0 ; i<n ; i++)
           duel_mkctype_struct_field(ct,i,TYPE_FIELD_NAME(t,i),
                TYPE_FIELD_BITPOS(t,i), TYPE_FIELD_BITSIZE(t,i),
                duel_convert_type_from_gdb(TYPE_FIELD_TYPE(t,i)));
      }
   break ;
   case TYPE_CODE_ENUM:
        /* TYPE_LENGTH(t) tell how big it is. I assume signed integral types.
         * it is unclear if gdb supports unsigned enums and how
         * (e.g. enum { x=0,y=250 } stored in uchar
         * FIELDS contain the tags, BITPOS is the assigned value.
         */
      { int i,n=TYPE_NFIELDS(t),len=TYPE_LENGTH(t);
	const char *name=TYPE_NAME(t);
        tctype_kind k ;
	if(name==NULL) name="" ;	/* duel can't handle null ptr */
        if(strncmp(name,"enum ",5)==0) name+=5 ;
        /* select 'real' stored type. note order important if short==int.
         * long is not allowed as far as I know ANSI C (enums are conv. to int)
	 * Amazingly, some internal gdb struct (sym) can have an enum of
	 * size zero (enum language, gdb4.8). We allow this as int but warn.
	 * gdb> p sizeof(sym->ginfo.lang_specific.language) gives zero
         */
	if(len==0) {
	    duel_printf("Warning: enum %s is size zero. assumed int\n",name);
	    len=sizeof(int);
	}
        if(len==sizeof(int))        k=CTK_INT ;
        else if(len==sizeof(short)) k=CTK_SHORT ;
        else if(len==sizeof(char))  k=CTK_CHAR ;
        else duel_assert(0);

        ct=duel_mkctype_enum(name,k,len,n);
        for(i=0 ; i<n ; i++)
           duel_mkctype_enumerator(ct,i,TYPE_FIELD_NAME(t,i),
                TYPE_FIELD_BITPOS(t,i));
      }
   break ;
   case TYPE_CODE_TYPEDEF:
      ct=duel_convert_type_from_gdb(check_typedef(t));
      break ;
   case TYPE_CODE_UNDEF:
      break;
  }
  if(ct==0) duel_fatal("unsupported C type returned by gdb");
  duel_add_hash(t,ct);
  return ct ;
}

/* optimize frame access so frame(100..0) doesnt start the search from 0
 * everytime. similar to selected_frame etc, but we dont want to mess up
 * gdb's own frame setup (for up/down/print etc)
 * this optimization should have been part of gdb, not here.
 * ie. duel_select_frame should be a simple fast gdb call.
 * we dont optimze going to frame 7 from frame 5 etc, this isn't typical.
 * set last/tot frames to -2 to assure recomputeations (-1 is not good enuf)
 */

static struct frame_info * last_frame ;       /* last frame pointer we used */
static int  last_frame_no ;     /* last frame number we used */
static int  tot_frames_no ;     /* tot number of frames */

LFUNC struct frame_info * duel_select_frame(int frame_no)
{
    struct frame_info * frame ;
    if(last_frame_no==frame_no)   frame=last_frame ;
    else
    if(last_frame_no==frame_no-1) frame=get_prev_frame(last_frame);
    else
    if(last_frame_no==frame_no+1) frame=get_next_frame(last_frame);
    else {
        int count=frame_no ;
        frame=get_current_frame();
        while (frame && --count >= 0)
            frame = get_prev_frame (frame);
    }
    duel_assert(frame); /* callee should have checked frames no*/
    last_frame = frame ;
    last_frame_no = frame_no ;
    return frame ;
}

FUNC bool duel_get_target_variable(const char *name, int frame_no, tvalue *v)
{
  struct symbol *sym;
  struct frame_info * frame ;
  struct block *blk ;
  int len ;
  struct value *gv ;                    /* gdb value */

  if(frame_no== -1) {           /* use the user selected frame and block */
      frame = get_selected_frame("Error!") ;
      blk = get_selected_block(0) ;
  }
  else {
      frame=duel_select_frame(frame_no) ;
      blk = get_frame_block(frame, 0);
  }
  sym = lookup_symbol (name, blk, VAR_DOMAIN,0,0);
  if(!sym) {		/* look up the symbol that has no debug info*/
     struct minimal_symbol *m ;
     if(frame_no != -1) return FALSE ; /* only locals looked up- not found*/
     m=lookup_minimal_symbol (name,NULL, 0); /* find printf, malloc etc */
     if(m == NULL) return FALSE ;
     v->val_kind=VK_LVALUE ;
#ifdef SYMBOL_LANGUAGE
     /* in gdb4.8 the mini table changed. I use the existance of the above
      * #define symbol as hack to indicate gdb.4.8 */
     v->u.lvalue=(ttarget_ptr) SYMBOL_VALUE_ADDRESS(m);
#else
     v->u.lvalue=(ttarget_ptr) m->address ;
#endif
	/* guess it is an int if it is a data type, an int func if text */
     if(m->type == mst_data || m->type == mst_bss) v->ctype=ctype_int ;
     else
     if(m->type == mst_text) v->ctype=duel_mkctype_func(ctype_int);
     else return FALSE ;
     return TRUE ;
  }
  if(SYMBOL_CLASS(sym)==LOC_TYPEDEF) return FALSE ;
  /* if frame specificed, allow only local variables to be found */
  if(frame_no!= -1 && (SYMBOL_CLASS(sym)==LOC_STATIC ||
   SYMBOL_CLASS(sym)==LOC_BLOCK || SYMBOL_CLASS(sym)==LOC_CONST)) return FALSE;
  gv=read_var_value(sym,frame);
  if(gv==0) return FALSE ; /* frame not found or illegal */
  duel_convert_value_from_gdb(gv,v);
  return TRUE ;
}


/* return the total number of frames on the stack */

FUNC int duel_get_frames_number(void)
{
    int n ;
    struct frame_info * frame ;
    if(tot_frames_no!= -2) return tot_frames_no ;
    frame=get_current_frame();
    for(n=0 ; frame ; n++)
        frame = get_prev_frame (frame);
    return tot_frames_no=n ;
}

FUNC ttarget_ptr duel_get_function_for_frame(int frame_no)
{
   struct frame_info * frame=duel_select_frame(frame_no);
   struct symbol *sym = get_frame_function(frame);
   struct value *val = read_var_value(sym,frame);
   duel_assert(val!=0 && VALUE_LVAL(val) == lval_memory);

   return (ttarget_ptr) VALUE_ADDRESS(val) ;
}

FUNC tctype* duel_get_target_typedef(const char *name)
{
  struct symbol *sym;
  sym = lookup_symbol (name, get_selected_block(0), VAR_DOMAIN,0,0);
  if(!sym || SYMBOL_CLASS(sym)!=LOC_TYPEDEF) {
      sym = lookup_symbol (name, 0, VAR_DOMAIN, 0,0);
      if(!sym || SYMBOL_CLASS(sym)!=LOC_TYPEDEF) return NULL ;
  }
  return duel_convert_type_from_gdb(SYMBOL_TYPE(sym));
}

FUNC tctype* duel_get_target_struct_tag(const char *name)
{
  struct symbol *sym;
  sym = lookup_symbol (name, get_selected_block(0), STRUCT_DOMAIN,0,0);
  if(!sym || TYPE_CODE(SYMBOL_TYPE(sym))!=TYPE_CODE_STRUCT) {
      sym = lookup_symbol (name, 0, STRUCT_DOMAIN, 0,0);
      if(!sym || TYPE_CODE(SYMBOL_TYPE(sym))!=TYPE_CODE_STRUCT) return NULL ;
  }
  return duel_convert_type_from_gdb(SYMBOL_TYPE(sym));
}

FUNC tctype* duel_get_target_union_tag(const char *name)
{
  struct symbol *sym;
  sym = lookup_symbol (name, get_selected_block(0), STRUCT_DOMAIN,0,0);
  if(!sym || TYPE_CODE(SYMBOL_TYPE(sym))!=TYPE_CODE_UNION) {
      sym = lookup_symbol (name, 0, STRUCT_DOMAIN, 0,0);
      if(!sym || TYPE_CODE(SYMBOL_TYPE(sym))!=TYPE_CODE_UNION) return NULL ;
  }
  return duel_convert_type_from_gdb(SYMBOL_TYPE(sym));
}

FUNC tctype* duel_get_target_enum_tag(const char *name)
{
  struct symbol *sym;
  sym = lookup_symbol (name, get_selected_block(0), STRUCT_DOMAIN,0,0);
  if(!sym || TYPE_CODE(SYMBOL_TYPE(sym))!=TYPE_CODE_ENUM) {
      sym = lookup_symbol (name, 0, STRUCT_DOMAIN, 0,0);
      if(!sym || TYPE_CODE(SYMBOL_TYPE(sym))!=TYPE_CODE_ENUM) return NULL ;
  }
  return duel_convert_type_from_gdb(SYMBOL_TYPE(sym));
}

/*
 * entry point from gdb.
 * produce help in gdb's format, or call duel enter point.
 * we allow ^c to quit immidiatly, and setup memory release cleanup.
 */

void duel_command(char *exp,int from_tty)
{
  last_frame_no = -2 ;  /* recompute frame location on each dl command */
  tot_frames_no = -2 ;

  make_cleanup(duel_cleanup, 0);       /* clear all allocated mem */
  immediate_quit++ ;
  duel_parse_and_eval(exp);
  immediate_quit-- ;
}

void
_initialize_duel()
{
  add_com ("duel", class_vars, duel_command,
"Evaluate Duel expressions. Duel is a very high level debugging language.\n\
\"dl help\" for help.\n");
  add_com_alias ("dl", "duel", class_vars, 1);

}
