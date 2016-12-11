#include "lib.h"

#include "value.h"
#include "func.h"
#include "primitives.h"
#include "symbol.h"

//#define CC(name) compile_call(c, &primitives[name ## _loc])
//#define CL(name) compile_call(c, (name))
//#define CO(v) compile_literal(c, (VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)(v)});
//#define CS(name) compile_literal(c, symbol_value(vm, (name)))
//
//void init_lib(VM* vm) {
//    COMPILER* c = &vm->compiler;
//    
//    PNODE const* dcall = &primitives[dcall_loc];
//    PNODE const* pcall = &primitives[pcall_loc];
//    
//    // **** methods ****
//    
//    FUNC* dcall_func = create_func(dcall, vm->primitive_func_meta);
//    FUNC* pcall_func = create_func(pcall, vm->primitive_func_meta);
//    
//    PNODE const* dgetf = &primitives[dgetf_loc];
//    FUNC* dgetf_func = create_func(dgetf, vm->primitive_func_meta);
//    
//    // **** call decl ****
//    
//    begin_compilation(c);
//    compile_func_enter(c);
//    compile_stub(c);
//    
//    register_func(vm, end_compilation(c), "call", 0);
//    
//    // **** getf ****
//    
//    DEFUN
//    (dgetf
//        I(dup)
//        I(meta)
//        I(dup)
//        OBJ(vm->default_meta)
//        I(eq)
//        CIF
//        (
//            SYM("index")
//            I(swap)
//            C(getf)
//            I(call)
//        )
//        (
//            I(drop),
//            L(dgetf),
//            DROP_MARKS(2),
//            END_DEFUN(dgetf)
//        )
//    )
//    
//    
//    
//    //CC(leave);
//    
//    //PNODE* getf = end_compilation(c);
//    //register_func(vm, getf, "getf", 0);
//    
//    // **** call def ****
//    
//    PNODE* call_rest = CC(dup);
//    call_stub->into = call_rest;
//    
//    CC(fpush);
//    tmp = CC(NULL);
//    tmp->value = (VALUE){.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)pcall_func}; //lookup(vm, "pcall");
//    CC(eq);
//    
//    CC(cjump);
//    mark_helper(vm);
//        CC(dup);
//        CC(meta);
//        
//        CC(fpush);
//        tmp = CC(NULL);
//        tmp->value = symbol_value(vm, "call");
//        CC(swap);
//        CC(getf);
//        CC(call);
//        CC(jump);
//    mark_helper(vm);
//    resolve_helper(vm, 2);
//        CC(pcall);
//    resolve_helper(vm, 1);
//    drop_marks_helper(vm, 2);
//    
//    CC(leave);
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
////    CC(dup);
////    CC(type);
////    compile_literal_helper(vm, symbol_value(vm, "symbol"));
////    CC(eq);
////    CC(cjump); mark_helper(vm);
////        //not symbol
////        CC(compile_literal);
////        CC(jump); mark_helper(vm);
////    
////        //is symbol
////        resolve_helper(vm, 2);
////        CC(dup);
////        CC(get);
////        CC(macro_qm);
////        CC(cjump); mark_helper(vm);
////            // not macro
////            CC(compile_call);
////            CC(jump); mark_helper(vm);
////    
////            // is macro
////            resolve_helper(vm, 2);
////            CC(get);
////            CC(call);
////    
////    resolve_helper(vm, 1);
////    drop_marks_helper(vm, 2);
////    
////    resolve_helper(vm, 1);
////    drop_marks_helper(vm, 2);
////    CC(leave);
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
////    CC(program_read);
////    CC(start_defun);
////    loop = CC(program_read);
////    CC(dup);
////    compile_literal_helper(vm, symbol_value(vm, "end"));
////    CC(eq);
////    CC(cjump);
////    mark_helper(vm);
////    // if false
////        CC(compile);
////        CC(jump);
////        CC(loop);
////    // if true
////        resolve_helper(vm, 1);
////        CC(drop);
////        CC(end_defun);
////        CC(leave);
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
//
