#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "vm.h"
#include "symbol.h"
#include "primitives.h"

int main() {
    VM* vm = create_vm();
    COMPILER* c = &vm->compiler;
   

// BEGIN DBL
begin_compilation(c);
compile_func_enter(c);
compile_call(c, &primitives[dup_loc]);
compile_call(c, &primitives[plus_loc]);
compile_call(c, &primitives[leave_loc]);
PNODE* dbl = end_compilation(c);


// BEGIN DBL_OR_QUAD
begin_compilation(c);
compile_func_enter(c);

compile_call(c, &primitives[not_loc]);
compile_cjump(c);
compile_call(c, dbl);
compiler_resolve(c, -1);
compiler_drop_marks(c, 1);

compile_call(c, &primitives[leave_loc]);
PNODE* dbl_or_quad = end_compilation(c);


// BEGIN PVM_MAIN
begin_compilation(c);
compile_func_enter(c);
compile_literal(c, num_value(4));
compile_call(c, &primitives[false_loc]);
compile_call(c, dbl_or_quad);
compile_call(c, &primitives[exit_loc]);
compile_call(c, &primitives[leave_loc]);
PNODE* pvm_main = end_compilation(c);



    
    vm->curr = pvm_main;
    
    next(vm);
    loop(vm);
    
    
    //while(1) {
    //    eval_str(vm);
   // }
    
    destroy_vm(vm);
    
    getch();
    return 0;
}
