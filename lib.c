#include "lib.h"

#include "func.h"
#include "primitives.h"

//void init_lib(VM* vm) {
//    COMPILER* c = &vm->compiler;
//    
//    PNODE const* dcall = &primitives[dcall_loc];
//    
//    // **** methods ****
//    
//    FUNC* dcall_func = create_func(dcall, vm->primitive_func_meta);
//    FUNC* pcall_func = create_func(pcall, vm->primitive_func_meta);
//    
//    PNODE* dgetf = fcons_deb(vm, dgetf_impl, "dgetf");
//    FUNC* dgetf_func = create_func(dgetf, vm->primitive_func_meta);
//    
//    // **** call decl ****
//    
//    PNODE* call = fcons(vm, enter_impl);
//    register_func(vm, call, "call", 0);
//    ncons(vm, jump);
//    PNODE* call_stub = ncons(vm, NULL);
//    
//    // **** getf ****
//    
//    PNODE* getf = fcons(vm, enter_impl);
//    register_func(vm, getf, "getf", 0);
//
//    ncons(vm, dup);
//    ncons(vm, meta);
//    
//    ncons(vm, dup);
//    ncons(vm, fpush);
//    PNODE* tmp = ncons(vm, NULL);
//    tmp->value.type = OBJECT_TYPE;
//    tmp->value.data.obj = (OBJECT_BASE*)vm->default_meta;
//    ncons(vm, eq);
//    
//    ncons(vm, cjump);
//    mark_helper(vm);
//        ncons(vm, fpush);
//        tmp = ncons(vm, NULL);
//        tmp->value = symbol_value(vm, "index");
//        ncons(vm, swap);
//        ncons(vm, getf);
//        ncons(vm, call);
//        ncons(vm, jump);
//    mark_helper(vm);
//    resolve_helper(vm, 2);
//        ncons(vm, drop);
//        ncons(vm, dgetf);
//    resolve_helper(vm, 1);
//    drop_marks_helper(vm, 2);
//    
//    ncons(vm, leave);
//    
//    // **** call def ****
//    
//    PNODE* call_rest = ncons(vm, dup);
//    call_stub->into = call_rest;
//    
//    ncons(vm, fpush);
//    tmp = ncons(vm, NULL);
//    tmp->value = (VALUE){.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)pcall_func}; //lookup(vm, "pcall");
//    ncons(vm, eq);
//    
//    ncons(vm, cjump);
//    mark_helper(vm);
//        ncons(vm, dup);
//        ncons(vm, meta);
//        
//        ncons(vm, fpush);
//        tmp = ncons(vm, NULL);
//        tmp->value = symbol_value(vm, "call");
//        ncons(vm, swap);
//        ncons(vm, getf);
//        ncons(vm, call);
//        ncons(vm, jump);
//    mark_helper(vm);
//    resolve_helper(vm, 2);
//        ncons(vm, pcall);
//    resolve_helper(vm, 1);
//    drop_marks_helper(vm, 2);
//    
//    ncons(vm, leave);
//    
//    
//    
//    
//    
////    
////    // **** compile ****
////    
////    PNODE* compile = fcons(vm, enter_impl);
////    register_macro(vm, compile, "compile", 1);
////    
////    ncons(vm, dup);
////    ncons(vm, type);
////    compile_literal_helper(vm, symbol_value(vm, "symbol"));
////    ncons(vm, eq);
////    ncons(vm, cjump); mark_helper(vm);
////        //not symbol
////        ncons(vm, compile_literal);
////        ncons(vm, jump); mark_helper(vm);
////    
////        //is symbol
////        resolve_helper(vm, 2);
////        ncons(vm, dup);
////        ncons(vm, get);
////        ncons(vm, macro_qm);
////        ncons(vm, cjump); mark_helper(vm);
////            // not macro
////            ncons(vm, compile_call);
////            ncons(vm, jump); mark_helper(vm);
////    
////            // is macro
////            resolve_helper(vm, 2);
////            ncons(vm, get);
////            ncons(vm, call);
////    
////    resolve_helper(vm, 1);
////    drop_marks_helper(vm, 2);
////    
////    resolve_helper(vm, 1);
////    drop_marks_helper(vm, 2);
////    ncons(vm, leave);
////    
////    // **** defun ****
////    
////    PNODE* start_defun = fcons(vm, start_defun_impl);
////    set_debug_info(vm, start_defun, "start_defun");
////    
////    PNODE* end_defun = fcons(vm, end_defun_impl);
////    set_debug_info(vm, end_defun, "end_defun");
////    
////    PNODE* loop;
////    PNODE* defn = fcons(vm, enter_impl);
////    register_macro(vm, defn, "defun", 0);
////    
////    ncons(vm, program_read);
////    ncons(vm, start_defun);
////    loop = ncons(vm, program_read);
////    ncons(vm, dup);
////    compile_literal_helper(vm, symbol_value(vm, "end"));
////    ncons(vm, eq);
////    ncons(vm, cjump);
////    mark_helper(vm);
////    // if false
////        ncons(vm, compile);
////        ncons(vm, jump);
////        ncons(vm, loop);
////    // if true
////        resolve_helper(vm, 1);
////        ncons(vm, drop);
////        ncons(vm, end_defun);
////        ncons(vm, leave);
////    drop_marks_helper(vm, 1);
////    
////    // **** run ****
////    
////    register_func(vm, defun(vm, 2, call, exit), "run", 0);
////    
////    // **** init meta methods ****
////    
////    set_method(vm, vm->default_meta, "index", dgetf_func);
////    set_method(vm, vm->func_meta, "call", dcall_func);
////    set_method(vm, vm->primitive_func_meta, "call", pcall_func);
////    
////    //program_flush(vm);
//}

