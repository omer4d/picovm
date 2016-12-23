#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "vm.h"
#include "symbol.h"
#include "primitives.h"
#include "lib.h"

#include "value.h"
#include "func.h"

typedef enum {
    REPL, RESUME, RESTART
}MODE;

VM* vm;

MODE show_halt_menu() {
    printf("[1] Debug\n");
    printf("[other] Resume\n");
    printf("Selection: ");

    char c = getchar();

    switch(c) {
        case '1':
            return RESUME;
        default:
            return RESUME;
    }
}

void repl() {
    MODE mode = REPL;
    
    while(!chs_eof(vm->in)) {
        switch(mode) {
            case REPL:
                vm_log(vm, ">");
                pvm_exec(vm, lookup_by_name(vm, "eval"));
                pvm_trace(vm);
                break;
            case RESUME:
                pvm_resume(vm);
                pvm_trace(vm);
                break;
            case RESTART:
                break;
        }
        
        if(pvm_test_flags(vm, PVM_RUNTIME_ERROR | PVM_COMPILE_TIME_ERROR)) {
            chs_drop(vm->in);
            mode = REPL;
        }
        else if(pvm_test_flags(vm, PVM_USER_HALT))
            mode = show_halt_menu();
        else {
            mode = REPL;
        }
    }
}

void load(char const* fn) {
    MODE mode = REPL;
    FILE* f = fopen(fn, "r");
    CHARSTREAM* chs = create_file_charstream(f);
    CHARSTREAM* old_in = vm->in;
    vm->in = chs;
    
    while(!chs_eof(vm->in) && !pvm_test_flags(vm, PVM_RUNTIME_ERROR | PVM_COMPILE_TIME_ERROR)) {
        switch(mode) {
            case REPL:
                pvm_exec(vm, lookup_by_name(vm, "eval"));
                break;
            case RESUME:
                pvm_resume(vm);
                break;
            case RESTART:
                break;
        }

        if(pvm_test_flags(vm, PVM_USER_HALT))
            mode = show_halt_menu();
        else {
            mode = REPL;
        }
    }
    
    destroy_charstream(chs);
    fclose(f);
    vm->in = old_in;
}

int main() {
    vm = create_vm();
    init_lib(vm);
    load("test.txt");
    repl();
    destroy_vm(vm);
    return 0;
}
