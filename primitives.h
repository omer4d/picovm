#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include "pnode.h"

#define PRIMITIVE_LIST(T, S)\
T(true)         S \
T(false)        S \
T(nil)          S \
                  \
                  \
T(dup)          S \
T(swap)         S \
T(drop)         S \
T(push)         S \
                  \
                  \
T(plus)         S \
T(minus)        S \
T(mul)          S \
T(div)          S \
T(mod)          S \
                  \
                  \
T(eq)           S \
T(not_eq)       S \
T(gt)           S \
T(lt)           S \
T(gte)          S \
T(lte)          S \
                  \
                  \
T(and)          S \
T(or)           S \
T(not)          S \
                  \
                  \
T(exit)         S \
T(jump)         S \
T(cjump)        S \
                  \
                  \
T(enter)        S \
T(leave)        S \
T(pcall)        S \
T(dcall)        S \
                  \
                  \
T(meta)         S \
T(dgetf)        S \
                  \
                  \
T(get)          S \
T(set)          S \
T(setmac)       S \
T(macro_qm)     S \
T(type)

#define PLIST_COMMA ,
#define PLIST_SEMICOL ;
#define PLIST_LOC(X) X ## _loc
#define PLIST_STR(X) #X

enum {
    PRIMITIVE_LIST(PLIST_LOC, PLIST_COMMA), PRIMITIVE_NUM
};

struct VM_t;

void next(struct VM_t* vm);

extern const char const* primitive_names[];
extern const PNODE primitives[];

#endif
