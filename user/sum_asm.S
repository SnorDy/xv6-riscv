.section .rodata 
input_pat: .string "|%s|\n"
output_pat: .string "%d\n"
err_msg: .string "error\n"


.section .bss
.lcomm buffer, 128 

.section .text             
.globl main
main:
    addi sp, sp, -16
    sd ra, 8(sp)
    
    la a0, buffer
    li a1, 128
    call gets
    beqz a0, error

    la a0, input_pat
    la a1, buffer
    call printf

    la t0, buffer

find:
    lb t1, 0(t0)
    beqz t1, error
    li t2, ' '
    beq t1, t2, found
    addi t0, t0, 1
    j find

found:
    sb zero, 0(t0)
    addi t2, t0, 1        
    
    la a0, buffer
    call atoi
    mv s0, a0
    mv a0, t2
    call atoi
    mv s1, a0

    mv a0, s0
    mv a1, s1
    call add
    mv a1, a0
    
    la a0, output_pat
    call printf
    
    li a0, 0
    j end

error:
    la a0, err_msg
    call printf
    li a0, 1

end:
    ld ra, 8(sp)
    addi sp, sp, 16
    ret
