; Veil interrupt stubs (M4). Each vector pushes (err_code|0, vector_no),
; then the common path saves ALL registers, hands the stack frame to C,
; and returns with iretd. A single NASM table of handler addresses means
; idt.c can install all 32 gates with a loop — no 32 extern declarations.

section .text
extern isr_handler

isr_common:
    pusha                      ; edi..eax in a fixed order: struct regs matches
    mov eax, esp               ; esp now points at the register frame
    push eax
    call isr_handler
    add esp, 4                 ; drop the frame pointer arg
    popa                       ; restore all GPRs the fault interrupted
    add esp, 8                 ; discard vector + error code we pushed
    iretd                      ; pops eip, cs, eflags (ring 0 -> nothing more)

; Ring-0 interrupts do NOT push SS/ESP (no privilege change), so we always
; synthesize an error code to keep every frame the same size.
%macro ISR_NOERR 1
isr_%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro

; These vectors already push a CPU error code — forwarding it costs nothing.
%macro ISR_ERR 1
isr_%1:
    push dword %1
    jmp isr_common
%endmacro

%assign i 0
%rep 32
    %if (i == 8) || (i >= 10 && i <= 14) || (i == 17) || (i == 29) || (i == 30)
        ISR_ERR %[i]
    %else
        ISR_NOERR %[i]
    %endif
    %assign i i+1
%endrep

global isr_table
section .rodata
align 4
isr_table:
%assign i 0
%rep 32
    dd isr_%[i]
%assign i i+1
%endrep
