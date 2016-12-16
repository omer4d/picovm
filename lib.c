#include "lib.h"

#include "value.h"
#include "func.h"
#include "primitives.h"
#include "symbol.h"
#include "compiler.h"

void init_lib(VM* vm) {
    COMPILER* c = &vm->compiler;
    FUNC* pcall_func = create_func(&primitives[pcall_loc], vm->primitive_func_meta, "pcall");
    FUNC* dgetf_func = create_func(&primitives[dgetf_loc], vm->primitive_func_meta, "dgetf");
    FUNC* dcall_func = create_func(&primitives[dcall_loc], vm->primitive_func_meta, "dcall");
    
    // DECLARE CALL
    begin_compilation(c);
    compile_stub(c);
    PNODE* call_stub = end_compilation(c, "call_stub");
    

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
    PNODE* getf = end_compilation(c, "getf");
    
    
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
    PNODE* call = end_compilation(c, "call");
    
    // BEGIN COMPILE
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[type_loc]);
    compile_literal(c, symbol_value(vm, "number"));
    compile_call(c, &primitives[eq_loc]);
    
    compile_call(c, &primitives[not_loc]);
    compile_cjump(c);
    compile_call(c, &primitives[compile_literal_loc]);
    compile_jump(c);
    compiler_resolve(c, -2);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[get_loc]);
    compile_call(c, &primitives[macro_qm_loc]);
    compile_call(c, &primitives[not_loc]);
    
    compile_call(c, &primitives[not_loc]);
    compile_cjump(c);
    compile_call(c, &primitives[compile_call_loc]);
    compile_jump(c);
    compiler_resolve(c, -2);
    compile_call(c, &primitives[get_loc]);
    compile_call(c, call);
    compiler_resolve(c, -1);
    compiler_drop_marks(c, 2);
    
    compiler_resolve(c, -1);
    compiler_drop_marks(c, 2);
    
    compile_call(c, &primitives[leave_loc]);
    PNODE* compile = end_compilation(c, "compile");
    
    register_macro(vm, compile, "compile", 0);

    // BEGIN DEFUN
    begin_compilation(c);
    compile_call(c, &primitives[program_read_loc]);
    compile_call(c, &primitives[begin_compilation_loc]);
    compiler_push_label(c);
    compile_call(c, &primitives[program_read_loc]);
    compile_call(c, &primitives[dup_loc]);
    compile_literal(c, symbol_value(vm, "end"));
    compile_call(c, &primitives[eq_loc]);
    
    compile_call(c, &primitives[not_loc]);
    compile_cjump(c);
    compile_call(c, &primitives[drop_loc]);
    compile_jump(c);
    compiler_resolve(c, -2);
    compile_call(c, compile);
    compile_jump(c);
    compiler_resolve_to_label(c, -1);
    compiler_resolve(c, -1);
    compiler_drop_marks(c, 2);
    
    compiler_drop_labels(c, 1);
    
    compile_literal(c, symbol_value(vm, "leave"));
    compile_call(c, compile);
    compile_call(c, &primitives[end_compilation_loc]);
    compile_call(c, &primitives[leave_loc]);
    PNODE* defun = end_compilation(c, "defun");
    
    register_macro(vm, defun, "defun", 0);


    
    // BEGIN RUN
    begin_compilation(c);
    compile_call(c, call);
    compile_call(c, &primitives[exit_loc]);
    compile_call(c, &primitives[leave_loc]);
    PNODE* run = end_compilation(c, "run");
    
    // BEGIN DBL
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[plus_loc]);
    compile_call(c, &primitives[leave_loc]);
    PNODE* dbl = end_compilation(c, "dbl");

    
    //call_stub[2].into = &call[1];
    set_method(vm, vm->default_meta, "index", dgetf_func);
    set_method(vm, vm->func_meta, "call", dcall_func);
    set_method(vm, vm->primitive_func_meta, "call", pcall_func);
    
    int i;
    for(i = 0; i < PRIMITIVE_FUNC_NUM; ++i) {
        register_func(vm, &primitives[i], primitive_names[i], 1);
    }
    
    for(i = PRIMITIVE_FUNC_NUM; i < PRIMITIVE_FUNC_NUM + PRIMITIVE_MACRO_NUM; ++i) {
        register_macro(vm, &primitives[i], primitive_names[i], 1);
    }
    
    register_func(vm, getf, "getf", 0);
    register_func(vm, dbl, "dbl", 0);
    register_func(vm, call, "call", 0);
    register_func(vm, run, "run", 0);
}
