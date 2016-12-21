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

int main() {
    VM* vm = create_vm();
    //COMPILER* c = &vm->compiler;
    init_lib(vm);
    
    //FILE* f = fopen("test.txt", "r");
    //vm->in = f;
    
    //VM_EXECUTION_CONTEXT xc;
    
    VM_CONTINUATION_DATA cont;
    int cont_used = 0;
    
    MODE mode = REPL;
    PNODE* n;
    
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
                break;
            case RESTART:
                break;
        }
        
        if(pvm_test_flags(vm, PVM_RUNTIME_ERROR) && !pvm_test_flags(vm, PVM_COMPILE_TIME_ERROR) && !cont_used) {
            printf("[1] Stash continuation and fix the problem\n");
            printf("[other] Restart\n: ");
            char c = getchar();
            if(c == '1') {
                mode = REPL;
                pvm_get_cc(&cont, vm);
                cont_used = 1;
            }
            else {
                free(n);
                mode = REPL;
            }
        }else if(pvm_test_flags(vm, PVM_USER_HALT)) {
            if(cont_used) {
                printf("[1] Call stashed continuation\n");
                printf("[2] Resume\n: ");
            }else{
                
            }
            
            printf("press any key to resume...");
            getchar();
            mode = RESUME;
        }else {
            free(n);
            mode = REPL;
        }
    }

    printf("the end!");
    //getchar();
    //fclose(f);
    destroy_vm(vm);
    
    return 0;
}
