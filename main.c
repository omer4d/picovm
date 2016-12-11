#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "vm.h"
#include "primitives.h"

int main() {
    VM* vm = create_vm();
    COMPILER* c = &vm->compiler;
    
    push(vm, num_value(1));
    push(vm, num_value(2));
    
    begin_compilation(c);
    compile_func_enter(c);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[plus_loc]);
    compile_call(c, &primitives[leave_loc]);
    PNODE* dbl = end_compilation(c);
    
    begin_compilation(c);
    compile_func_enter(c);
    compile_call(c, &primitives[plus_loc]);
    compile_literal(c, num_value(555));
    compile_call(c, &primitives[plus_loc]);
    compile_call(c, dbl);
    
    compile_literal(c, PVM_TRUE);
    
    compile_cjump(c);
    compile_call(c, &primitives[dup_loc]);
    //compile_call(c, &primitives[dup_loc]);
    
    compiler_resolve(c, -1);
    compile_call(c, &primitives[exit_loc]);
    
    
    PNODE* func = end_compilation(c);
    
    vm->curr = func;
    
    next(vm);
    loop(vm);
    
    
    //while(1) {
    //    eval_str(vm);
   // }
    
    destroy_vm(vm);
    
    getch();
    return 0;
}
