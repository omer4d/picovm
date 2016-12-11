#include "lib.h"

#include "value.h"
#include "func.h"
#include "primitives.h"
#include "symbol.h"
#include "compiler.h"

void init_lib(VM* vm) {
    COMPILER* c = &vm->compiler;
    FUNC* pcall_func = create_func(&primitives[pcall_loc], vm->primitive_func_meta);
    FUNC* dgetf_func = create_func(&primitives[dgetf_loc], vm->primitive_func_meta);
    FUNC* dcall_func = create_func(&primitives[dcall_loc], vm->primitive_func_meta);
    
    // DECLARE CALL
    begin_compilation(c);
    compile_stub(c);
    PNODE* call_stub = end_compilation(c);
    
    
    // BEGIN GETF
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[meta_loc]);
    compile_call(c, &primitives[dup_loc]);
    compile_literal(c, (VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)vm->default_meta});
    compile_call(c, &primitives[eq_loc]);
    
    compile_call(c, &primitives[not_loc]);
    compile_cjump(c);
    compile_call(c, &primitives[drop_loc]);
    compile_call(c, &primitives[dgetf_loc]);
    compile_jump(c);
    compiler_resolve(c, -2);
    compile_literal(c, symbol_value(vm, "index"));
    compile_call(c, &primitives[swap_loc]);
    compile_recur(c);
    compile_call(c, call_stub);
    compiler_resolve(c, -1);
    compiler_drop_marks(c, 2);
    
    compile_call(c, &primitives[leave_loc]);
    PNODE* getf = end_compilation(c);
    
    
    // BEGIN CALL
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_literal(c, (VALUE){.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)pcall_func});
    compile_call(c, &primitives[eq_loc]);
    
    compile_call(c, &primitives[not_loc]);
    compile_cjump(c);
    compile_call(c, &primitives[pcall_loc]);
    compile_jump(c);
    compiler_resolve(c, -2);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[meta_loc]);
    compile_literal(c, symbol_value(vm, "call"));
    compile_call(c, &primitives[swap_loc]);
    compile_call(c, getf);
    compile_recur(c);
    compiler_resolve(c, -1);
    compiler_drop_marks(c, 2);
    
    compile_call(c, &primitives[leave_loc]);
    PNODE* call = end_compilation(c);
    
    
    
    
    
    call_stub[2].into = &call[1];
    set_method(vm, vm->default_meta, "index", dgetf_func);
    set_method(vm, vm->func_meta, "call", dcall_func);
    set_method(vm, vm->primitive_func_meta, "call", pcall_func);
    
    int i;
    for(i = 0; i < PRIMITIVE_NUM; ++i) {
        register_func(vm, &primitives[i], primitive_names[i], 1);
    }
    
    register_func(vm, call, "call", 0);
}
