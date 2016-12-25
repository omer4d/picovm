#include "lib.h"

#include "value.h"
#include "func.h"
#include "primitives.h"
#include "symbol.h"
#include "compiler.h"

void init_lib(VM* vm) {
    COMPILER* c = &vm->compiler;
    FUNC* pcall_func = create_func(&primitives[pcall_loc], vm->primitive_func_meta, "pcall", 0);
    FUNC* dgetf_func = create_func(&primitives[dgetf_loc], vm->primitive_func_meta, "dgetf", 0);
    FUNC* dsetf_func = create_func(&primitives[dsetf_loc], vm->primitive_func_meta, "dsetf", 0);
    FUNC* delete_object_func = create_func(&primitives[delete_object_loc], vm->primitive_func_meta, "delete_object", 0);
    FUNC* delete_string_func = create_func(&primitives[delete_string_loc], vm->primitive_func_meta, "delete_string", 0);
    FUNC* dcall_func = create_func(&primitives[dcall_loc], vm->primitive_func_meta, "dcall", 0);


    // DECLARE CALL
    PNODE* call_stub = compile_stub(c, "call-stub");


    // BEGIN GETF
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[meta_loc]);
    compile_call(c, &primitives[dup_loc]);
    compile_literal(c, (VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)vm->default_meta});
    compile_call(c, &primitives[eq_loc]);

    compile_call(c, &primitives[not_loc]);
    ANODE* node0 = compile_cjump(c);
    compile_call(c, &primitives[drop_loc]);
    compile_call(c, &primitives[dgetf_loc]);
    ANODE* node1 = compile_jump(c);
    resolve_jump(node0, compiler_pos(c));
    compile_literal(c, symbol_value(vm, "index"));
    compile_call(c, &primitives[swap_loc]);
    compile_recur(c);
    compile_call(c, call_stub);
    resolve_jump(node1, compiler_pos(c));

    compile_call(c, &primitives[leave_loc]);
    PNODE* getf = end_compilation(c, "getf");

    register_func(vm, getf, "getf", 0);


    // BEGIN CALL
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_literal(c, (VALUE){.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)pcall_func});
    compile_call(c, &primitives[eq_loc]);

    compile_call(c, &primitives[not_loc]);
    ANODE* node2 = compile_cjump(c);
    compile_call(c, &primitives[pcall_loc]);
    ANODE* node3 = compile_jump(c);
    resolve_jump(node2, compiler_pos(c));
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[meta_loc]);
    compile_literal(c, symbol_value(vm, "call"));
    compile_call(c, &primitives[swap_loc]);
    compile_call(c, getf);
    compile_recur(c);
    resolve_jump(node3, compiler_pos(c));

    compile_call(c, &primitives[leave_loc]);
    PNODE* call = end_compilation(c, "call");

    resolve_stub(call_stub, call);
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
    ANODE* node4 = compile_cjump(c);
    compile_call(c, &primitives[compile_literal_loc]);
    ANODE* node5 = compile_jump(c);
    resolve_jump(node4, compiler_pos(c));
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[get_loc]);
    compile_call(c, &primitives[macro_qm_loc]);
    compile_call(c, &primitives[not_loc]);

    compile_call(c, &primitives[not_loc]);
    ANODE* node6 = compile_cjump(c);
    compile_call(c, &primitives[compile_call_loc]);
    ANODE* node7 = compile_jump(c);
    resolve_jump(node6, compiler_pos(c));
    compile_call(c, &primitives[get_loc]);
    compile_call(c, call);
    resolve_jump(node7, compiler_pos(c));

    resolve_jump(node5, compiler_pos(c));

    compile_call(c, &primitives[leave_loc]);
    PNODE* compile = end_compilation(c, "compile");

    register_macro(vm, compile, "compile", 0);


    // BEGIN DEFUN
    begin_compilation(c);
    compile_call(c, &primitives[program_read_loc]);
    compile_call(c, &primitives[begin_compilation_loc]);
    ANODE* node8 = compiler_pos(c);
    compile_call(c, &primitives[program_read_loc]);
    compile_call(c, &primitives[dup_loc]);
    compile_literal(c, symbol_value(vm, "end"));
    compile_call(c, &primitives[eq_loc]);
    compile_call(c, &primitives[not_loc]);
    compile_call(c, &primitives[not_loc]);
    ANODE* node9 = compile_cjump(c);
    compile_call(c, compile);
    ANODE* node10 = compile_jump(c);
    resolve_jump(node10, node8);
    resolve_jump(node9, compiler_pos(c));
    compile_call(c, &primitives[drop_loc]);
    compile_literal(c, symbol_value(vm, "leave"));
    compile_call(c, compile);
    compile_call(c, &primitives[end_compilation_loc]);
    compile_call(c, &primitives[leave_loc]);
    PNODE* defun = end_compilation(c, "defun");

    register_macro(vm, defun, "defun", 0);


    // BEGIN SETF
    begin_compilation(c);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[meta_loc]);
    compile_literal(c, symbol_value(vm, "setf"));
    compile_call(c, &primitives[swap_loc]);
    compile_call(c, getf);
    compile_call(c, call);
    compile_call(c, &primitives[leave_loc]);
    PNODE* setf = end_compilation(c, "setf");

    register_func(vm, setf, "setf", 0);


    // BEGIN EVAL
    begin_compilation(c);
    ANODE* node11 = compiler_pos(c);
    compile_call(c, &primitives[eol_qm_loc]);
    compile_call(c, &primitives[not_loc]);
    compile_call(c, &primitives[not_loc]);
    ANODE* node12 = compile_cjump(c);
    compile_call(c, &primitives[program_read_loc]);
    compile_call(c, &primitives[dup_loc]);
    compile_call(c, &primitives[type_loc]);
    compile_literal(c, symbol_value(vm, "symbol"));
    compile_call(c, &primitives[eq_loc]);

    compile_call(c, &primitives[not_loc]);
    ANODE* node13 = compile_cjump(c);
    compile_call(c, &primitives[get_loc]);
    compile_call(c, call);
    resolve_jump(node13, compiler_pos(c));

    ANODE* node14 = compile_jump(c);
    resolve_jump(node14, node11);
    resolve_jump(node12, compiler_pos(c));
    compile_call(c, &primitives[leave_loc]);
    PNODE* eval = end_compilation(c, "eval");

    register_func(vm, eval, "eval", 0);


    

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
