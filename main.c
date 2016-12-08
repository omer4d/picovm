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
        //gets(str);
        eval_str(vm, stdin);
    }
    
    destroy_vm(vm);
    
    getch();
    return 0;
}
