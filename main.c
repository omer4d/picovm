#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "vm.h"
//#include "symbol.h"
//#include "primitives.h"
//#include "lib.h"



#include "value.h"



int main() {
    VALUE v = parse_num("123");
    printf("%f", v.data.num);
    
    /*
    VM* vm = create_vm();
    COMPILER* c = &vm->compiler;
    init_lib(vm);
    
    while(1) {
        pvm_eval(vm);
   }
    
    destroy_vm(vm);*/
    
    getch();
    return 0;
}
