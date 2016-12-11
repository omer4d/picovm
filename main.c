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
compile_call(c, &primitives[dup_loc]);
compile_call(c, &primitives[plus_loc]);
compile_call(c, &primitives[leave_loc]);
PNODE* dbl = end_compilation(c);


// BEGIN EVEN
begin_compilation(c);
compile_call(c, &primitives[dup_loc]);
compile_literal(c, num_value(2));
compile_call(c, &primitives[mod_loc]);
compile_literal(c, num_value(0));
compile_call(c, &primitives[eq_loc]);
compile_call(c, &primitives[leave_loc]);
PNODE* even = end_compilation(c);


// BEGIN DBL_OR_QUAD
begin_compilation(c);

compile_call(c, &primitives[not_loc]);
compile_cjump(c);
compile_call(c, dbl);
compile_jump(c);
compiler_resolve(c, -2);
compile_call(c, even);

compile_call(c, &primitives[not_loc]);
compile_cjump(c);
compile_call(c, dbl);
compile_jump(c);
compiler_resolve(c, -2);
compile_literal(c, num_value(1));
compile_call(c, &primitives[plus_loc]);
compiler_resolve(c, -1);
compiler_drop_marks(c, 2);

compile_call(c, dbl);
compile_call(c, dbl);
compiler_resolve(c, -1);
compiler_drop_marks(c, 2);

compile_call(c, &primitives[leave_loc]);
PNODE* dbl_or_quad = end_compilation(c);


// BEGIN FOOBAR
begin_compilation(c);
compile_call(c, &primitives[dup_loc]);
compile_literal(c, num_value(0));
compile_call(c, &primitives[gt_loc]);

compile_call(c, &primitives[not_loc]);
compile_cjump(c);
compile_literal(c, symbol_value(vm, "again"));
compile_call(c, &primitives[swap_loc]);
compile_literal(c, num_value(1));
compile_call(c, &primitives[minus_loc]);
compile_recur(c);
compiler_resolve(c, -1);
compiler_drop_marks(c, 1);

compile_call(c, &primitives[leave_loc]);
PNODE* foobar = end_compilation(c);


// BEGIN PVM_MAIN
begin_compilation(c);
compile_literal(c, num_value(4));
compile_call(c, &primitives[true_loc]);
compile_call(c, dbl_or_quad);
compile_call(c, foobar);
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
