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

typedef enum {
    REPL, RESUME, RESTART
}MODE;

VM* vm;
PNODE* n;

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

void loop() {
    MODE mode = REPL;
    
    while(!feof(vm->in)) {
        switch(mode) {
            case REPL:
                n = pvm_compile(vm);
                if(n) {
                    pvm_run(vm, n);
                    print_debug_info(vm);
                }
                break;
            case RESUME:
                pvm_resume(vm);
                print_debug_info(vm);
                break;
            case RESTART:
                break;
        }
        
        if(pvm_test_flags(vm, PVM_USER_HALT))
            mode = show_halt_menu();
        else {
            free(n);
            mode = REPL;
        }
    }
}

int main() {
    vm = create_vm();
    init_lib(vm);
    loop();
    destroy_vm(vm);
    return 0;
}
