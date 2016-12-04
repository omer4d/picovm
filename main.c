#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "vm.h"





int main() {
    VM* vm = create_vm();
    
    char str[256];
    
    while(1) {
        gets(str);
        eval_str(vm, str);
    }
    
    destroy_vm(vm);
    
    //eval_str("+");
    
    
    
    

    
    //run = register_func(vm, defun(2, dcall, exit), "run", 0);
    
    // |---->
    // key obj
    /*
    dup default_meta eq
    cjump mark
        dup meta [ index ] swap getf
        call
        leave
    resolve
        dgetf
    */
    

//    eval("          foo       1234 sd 132d324 31 ");
    
    
    
    //printf(dbl->
    
    /*
    PNODE* quad = defun(2, dbl, dbl);
    PNODE* main = defun(5, dcall, plus, drop, swap, dgetf);*/
    
    
    
    
    /*test111
    OBJECT* o = create_object();
    VALUE key = symbol_value("foo");
    VALUE val = num_value(123);
    //VALUE key2 = symbol_value("foo");
    map_put(&o->map, &key, &val);
    push(vm, key);
    push(vm, (VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)o});*/
    
    
    
    
    //push(vm, num_value(2));
    //push(vm, func_value(vm, dbl, 0));
    
    
    
    
    
    /*
    push(vm, num_value(2));
    push(vm, num_value(4));
    push(vm, func_value(vm, quad));*/
    
    /*
    VALUE v;
    v.type = BOOL_TYPE;
    v.data.boolean = 0;
    push(vm, num_value(2));
    push(vm, num_value(3));
    push(vm, v);*/
    
    
    /*
    OBJECT* o = create_object();
    VALUE key = symbol_value("index");
    VALUE val = num_value(123);
    map_put(&o->map, &key, &val);
    push(vm, key);
    push(vm, (VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)o});
    */
    
    
    
    //|---------->
    //key obj
        /*
    
    dup meta
    dup default_meta eq
    cjump mark
        [ index ] swap getf
        call
        leave
    resolve
        dgetf*/
    
    
    
    

    
    // *******
    // * etc *
    // *******
    
    
    
    
    //PNODE* main = defun(4, meta, swap, dgetf, f);
    
    /*                    
    PNODE* main = defun(1, call);
    
    
    
    
    
    
                        
    push(vm, func_value(vm, main, 0));
    
    vm->curr = run;
    next(vm);
    loop();*/
    
    getch();
    return 0;
}
