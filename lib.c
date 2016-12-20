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
    FUNC* dsetf_func = create_func(&primitives[dsetf_loc], vm->primitive_func_meta, "dsetf");
    FUNC* delete_object_func = create_func(&primitives[delete_object_loc], vm->primitive_func_meta, "delete_object");
    FUNC* detele_string_func = create_func(&primitives[delete_string_loc], vm->primitive_func_meta, "delete_string");
    FUNC* dcall_func = create_func(&primitives[dcall_loc], vm->primitive_func_meta, "dcall");
    
    // DECLARE CALL
    begin_compilation(c);
    compile_stub(c);
    PNODE* call_stub = end_compilation(c, "call-stub");


    // BEGIN GETF
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[meta_loc]);
    compile_call(c, &primitives[dup_loc]);
    compile_literal(c, (VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)vm->default_meta});
    compile_call(c, &primitives[eq_loc]);

    compile_call(c, &primitives[not_loc]);
    ANODE* jump0 = compile_cjump(c);
    compile_call(c, &primitives[drop_loc]);
    compile_call(c, &primitives[dgetf_loc]);
    ANODE* jump1 = compile_jump(c);
    resolve_jump(jump0, compiler_pos(c));
    compile_literal(c, symbol_value(vm, "index"));
    compile_call(c, &primitives[swap_loc]);
    compile_recur(c);
    compile_call(c, call_stub);
    resolve_jump(jump1, compiler_pos(c));

    compile_call(c, &primitives[leave_loc]);
    PNODE* getf = end_compilation(c, "getf");

    register_func(vm, getf, "getf", 0);


    // BEGIN CALL
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_literal(c, (VALUE){.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)pcall_func});
    compile_call(c, &primitives[eq_loc]);

    compile_call(c, &primitives[not_loc]);
    ANODE* jump2 = compile_cjump(c);
    compile_call(c, &primitives[pcall_loc]);
    ANODE* jump3 = compile_jump(c);
    resolve_jump(jump2, compiler_pos(c));
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[meta_loc]);
    compile_literal(c, symbol_value(vm, "call"));
    compile_call(c, &primitives[swap_loc]);
    compile_call(c, getf);
    compile_recur(c);
    resolve_jump(jump3, compiler_pos(c));

    compile_call(c, &primitives[leave_loc]);
    PNODE* call = end_compilation(c, "call");

    register_func(vm, call, "call", 0);
    
    
    // BEGIN RUN
    begin_compilation(c);
    compile_call(c, call);
    compile_call(c, &primitives[exit_loc]);
    compile_call(c, &primitives[leave_loc]);
    PNODE* run = end_compilation(c, "run");

    register_func(vm, run, "run", 0);


    // BEGIN COMPILE
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[type_loc]);
    compile_literal(c, symbol_value(vm, "number"));
    compile_call(c, &primitives[eq_loc]);

    compile_call(c, &primitives[not_loc]);
    ANODE* jump4 = compile_cjump(c);
    compile_call(c, &primitives[compile_literal_loc]);
    ANODE* jump5 = compile_jump(c);
    resolve_jump(jump4, compiler_pos(c));
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[get_loc]);
    compile_call(c, &primitives[macro_qm_loc]);
    compile_call(c, &primitives[not_loc]);

    compile_call(c, &primitives[not_loc]);
    ANODE* jump6 = compile_cjump(c);
    compile_call(c, &primitives[compile_call_loc]);
    ANODE* jump7 = compile_jump(c);
    resolve_jump(jump6, compiler_pos(c));
    compile_call(c, &primitives[get_loc]);
    compile_call(c, call);
    resolve_jump(jump7, compiler_pos(c));

    resolve_jump(jump5, compiler_pos(c));

    compile_call(c, &primitives[leave_loc]);
    PNODE* compile = end_compilation(c, "compile");

    register_macro(vm, compile, "compile", 0);

    
    
    
    
    // BEGIN DEFUN
    begin_compilation(c);
    compile_call(c, &primitives[program_read_loc]);
    compile_call(c, &primitives[begin_compilation_loc]);
    ANODE* lab0 = compiler_pos(c);
    compile_call(c, &primitives[program_read_loc]);
    compile_call(c, &primitives[dup_loc]);
    compile_literal(c, symbol_value(vm, "end"));
    compile_call(c, &primitives[eq_loc]);

    compile_call(c, &primitives[not_loc]);
    ANODE* jump8 = compile_cjump(c);
    compile_call(c, &primitives[drop_loc]);
    ANODE* jump9 = compile_jump(c);
    resolve_jump(jump8, compiler_pos(c));
    compile_call(c, compile);
    ANODE* jump10 = compile_jump(c);
    resolve_jump(jump10, lab0);
    resolve_jump(jump9, compiler_pos(c));

    compile_literal(c, symbol_value(vm, "leave"));
    compile_call(c, compile);
    compile_call(c, &primitives[end_compilation_loc]);
    compile_call(c, &primitives[leave_loc]);
    PNODE* defun = end_compilation(c, "defun");

    register_macro(vm, defun, "defun", 0);

    
    //call_stub[2].into = &call[1];
    set_method(vm, vm->default_meta, "index", dgetf_func);
    set_method(vm, vm->default_meta, "setf", dsetf_func);
    set_method(vm, vm->default_meta, "delete", delete_object_func);
    set_method(vm, vm->func_meta, "call", dcall_func);
    set_method(vm, vm->primitive_func_meta, "call", pcall_func);
    
    int i;
    for(i = 0; i < PRIMITIVE_FUNC_NUM; ++i) {
        register_func(vm, &primitives[i], primitive_names[i], 1);
    }
    
    for(i = PRIMITIVE_FUNC_NUM; i < PRIMITIVE_FUNC_NUM + PRIMITIVE_MACRO_NUM; ++i) {
        register_macro(vm, &primitives[i], primitive_names[i], 1);
    }
}
