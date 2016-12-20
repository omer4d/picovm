#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include "pnode.h"

#define PLIST_COMMA ,
#define PLIST_SEMICOL ;
#define PLIST_LOC(X) X ## _loc

#define PLIST_STR(X) #X

#define PLIST_IGNORE(X)
#define PLIST_ONE(X) 1
#define PLIST_ID(X) X

#define PRIMITIVE_FUNC_LIST(T, N, TN, S)\
TN(halt)                S   \
                            \
                            \
TN(true)                S   \
TN(false)               S   \
TN(nil)                 S   \
				            \
				            \
TN(dup)                 S   \
TN(swap)                S   \
TN(drop)                S   \
T(push)    N("")         S   \
				            \
				            \
T(plus)    N("+")        S   \
T(minus)   N("-")        S   \
T(mul)     N("*")        S   \
T(div)     N("/")        S   \
T(mod)     N("%")        S   \
				            \
				            \
T(eq)      N("=")        S   \
T(not_eq)  N("not=")     S   \
T(gt)      N(">")        S   \
T(lt)      N("<")        S   \
T(gte)     N(">=")       S   \
T(lte)     N("<=")       S   \
				            \
				            \
TN(and)                 S   \
TN(or)                  S   \
TN(not)                 S   \
				            \
				            \
TN(exit)                S   \
T(jump)    N("")         S   \
T(cjump)   N("")         S   \
				            \
				            \
T(enter)   N("")         S   \
TN(leave)               S   \
T(pcall)   N("")         S   \
T(dcall)   N("")         S   \
				            \
				            \
TN(meta)                S   \
T(dgetf)   N("")         S   \
				            \
				            \
TN(get)                 S   \
TN(set)                 S   \
T(setmac)  N("setmac")   S   \
T(macro_qm)N("macro?")   S   \
TN(type)                S   \
TN(load)

#define PRIMITIVE_MACRO_LIST(T, N, TN, S) \
T(read_string)         N("\"")               S   \
T(program_read)        N(">>")               S   \
T(program_unread)      N("<<")               S   \
T(compile_literal)     N("compile-literal")  S   \
T(compile_call)        N("compile-call")     S   \
T(begin_compilation)   N("")                 S   \
T(end_compilation)     N("")                 S   \
T(jump_macro)          N("jump")             S   \
T(cjump_macro)         N("?jump")            S   \
T(resolve)             N("resolve:")         S   \
T(label)               N("label:")           S   \
T(to_label)            N("to-label")

enum {
    PRIMITIVE_FUNC_LIST(PLIST_LOC, PLIST_IGNORE, PLIST_LOC, PLIST_COMMA),
    PRIMITIVE_MACRO_LIST(PLIST_LOC, PLIST_IGNORE, PLIST_LOC, PLIST_COMMA),
    PRIMITIVE_NUM
};

struct VM_t;

void next(struct VM_t* vm);

extern const char const* primitive_names[];
extern const char const* primitive_internal_names[];
extern const PNODE primitives[];

extern const int PRIMITIVE_FUNC_NUM;
extern const int PRIMITIVE_MACRO_NUM;

#endif
