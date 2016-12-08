#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

struct VM_t;

void next(struct VM_t* vm);

void true_impl(struct VM_t* vm);
void false_impl(struct VM_t* vm);
void nil_impl(struct VM_t* vm);


void dup_impl(struct VM_t* vm);
void swap_impl(struct VM_t* vm);
void drop_impl(struct VM_t* vm);
void push_impl(struct VM_t* vm);


void plus_impl(struct VM_t* vm);
void minus_impl(struct VM_t* vm);
void mul_impl(struct VM_t* vm);
void div_impl(struct VM_t* vm);
void mod_impl(struct VM_t* vm);


void eq_impl(struct VM_t* vm);
void not_eq_impl(struct VM_t* vm);
void gt_impl(struct VM_t* vm);
void lt_impl(struct VM_t* vm);
void gte_impl(struct VM_t* vm);
void lte_impl(struct VM_t* vm);


void and_impl(struct VM_t* vm);
void or_impl(struct VM_t* vm);
void not_impl(struct VM_t* vm);


void exit_impl(struct VM_t* vm);
void jump_impl(struct VM_t* vm);
void cjump_impl(struct VM_t* vm);


void enter_impl(struct VM_t* vm);
void leave_impl(struct VM_t* vm);
void pcall_impl(struct VM_t* vm);
void dcall_impl(struct VM_t* vm);


void meta_impl(struct VM_t* vm);
void dgetf_impl(struct VM_t* vm);


void get_impl(struct VM_t* vm);
void set_impl(struct VM_t* vm);
void setmac_impl(struct VM_t* vm);
void macro_qm_impl(struct VM_t* vm);
void type_impl(struct VM_t* vm);

#endif
