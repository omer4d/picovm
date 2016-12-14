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



int main() {
    
    VM* vm = create_vm();
    //COMPILER* c = &vm->compiler;
    init_lib(vm);
    
    //FILE* f = fopen("test.txt", "r");
    //vm->in = f;
    
    VM_EXECUTION_CONTEXT xc;
    
    while(!feof(vm->in)) {
        pvm_eval(vm);
        if(pvm_test_flags(vm, PVM_RUNTIME_ERROR | PVM_COMPILE_TIME_ERROR)) {
            printf("ERROR HALT!");
            getchar();
        }
        
        else if(pvm_test_flags(vm, PVM_USER_HALT)) {
            printf("USER HALT!");
            getchar();
        }
    }

    //getchar();
    //fclose(f);
    destroy_vm(vm);
    
    return 0;
}
