#Submitted by Prajyoth Bhandary and Hrishikesh Deshmukh

#void thread_switch(struct thread * old, struct thread * new)
#                    rdi                 rsi
.globl thread_switch

thread_switch:

        #Step1:  push all callee-save registers onto the stack
        pushq %rbx
        pushq %rbp
        pushq %r12
        pushq %r13
        pushq %r14
        pushq %r15

        #Step2: Save the current stack pointer in old thread's control block
        movq %rsp, (%rdi)

        #Step3: Load stack pointer with new thread's control block
        movq (%rsi), %rsp

        #Step 4: Pop all the callee-save registers from the stack of new thhread
        popq %r15
        popq %r14
        popq %r13
        popq %r12
        popq %rbp
        popq %rbx
        #Step5: Return
        ret

#thread_start(struct thread * old, struct thread * new)
#                       rdi                     rsi


.globl thread_start

thread_start:

        #Step1:  push all callee-save registers onto the stack
        pushq %rbx
        pushq %rbp
        pushq %r12
        pushq %r13
        pushq %r14
        pushq %r15

        #Step2: Save the current stack pointer in old thread's control block
        movq %rsp, (%rdi)


        #Step3: Load stack pointer with new thread's control block
        movq (%rsi), %rsp

        #Step4: Load the new thread's initial argument into the first argument register (%rdi) and jump to the initial function
        #movq 16(%rsi), %rdi
        #jmp *%rdi
        jmp thread_wrap
