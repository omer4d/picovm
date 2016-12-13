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
    
    while(!feof(vm->in)) {
        pvm_eval(vm);
    }

    //getchar();
    //fclose(f);
    destroy_vm(vm);
    
    return 0;
}
