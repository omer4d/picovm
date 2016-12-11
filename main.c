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
    
    while(1) {
        pvm_eval(vm);
   }
    
    destroy_vm(vm);
    
    return 0;
}
