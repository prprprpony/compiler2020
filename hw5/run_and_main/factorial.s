.data
_g_n: .word 0
.text
.text
_start_fact:
sd ra,0(sp)
sd fp,-8(sp)
add fp,sp,-8
add sp,sp,-16
la ra,_frameSize_fact
lw ra,0(ra)
sub sp,sp,ra
sd t0,8(sp)
sd t1,16(sp)
sd t2,24(sp)
sd t3,32(sp)
sd t4,40(sp)
sd t5,48(sp)
sd t6,56(sp)
sd s2,64(sp)
sd s3,72(sp)
sd s4,80(sp)
sd s5,88(sp)
sd s6,96(sp)
sd s7,104(sp)
sd s8,112(sp)
sd s9,120(sp)
sd s10,128(sp)
sd s11,136(sp)
sd fp,144(sp)
fsw ft0,152(sp)
fsw ft1,156(sp)
fsw ft2,160(sp)
fsw ft3,164(sp)
fsw ft4,168(sp)
fsw ft5,172(sp)
fsw ft6,176(sp)
fsw ft7,180(sp)
la t5, _g_n
lw t0,0(t5)
.data
_CONSTANT_1: .word 1
.align 3
.text
lw t1, _CONSTANT_1
sub t0, t0, t1
seqz t0, t0
beqz t0, _elseLabel_0
la t5, _g_n
lw t0,0(t5)
mv a0,t0
j _end_fact
j _ifExitLabel_0
_elseLabel_0:
la t5, _g_n
lw t0,0(t5)
.data
_CONSTANT_2: .word 1
.align 3
.text
lw t1, _CONSTANT_2
subw t0, t0, t1
la t1, _g_n
sw t0,0(t1)
la t5, _g_n
lw t0,0(t5)
jal _start_fact
mv t1, a0
mulw t0, t0, t1
mv a0,t0
j _end_fact
_ifExitLabel_0:
_end_fact:
ld t0,8(sp)
ld t1,16(sp)
ld t2,24(sp)
ld t3,32(sp)
ld t4,40(sp)
ld t5,48(sp)
ld t6,56(sp)
ld s2,64(sp)
ld s3,72(sp)
ld s4,80(sp)
ld s5,88(sp)
ld s6,96(sp)
ld s7,104(sp)
ld s8,112(sp)
ld s9,120(sp)
ld s10,128(sp)
ld s11,136(sp)
ld fp,144(sp)
flw ft0,152(sp)
flw ft1,156(sp)
flw ft2,160(sp)
flw ft3,164(sp)
flw ft4,168(sp)
flw ft5,172(sp)
flw ft6,176(sp)
flw ft7,180(sp)
ld ra,8(fp)
mv sp,fp
add sp,sp,8
ld fp,0(fp)
jr ra
.data
_frameSize_fact: .word 184
.text
_start_MAIN:
sd ra,0(sp)
sd fp,-8(sp)
add fp,sp,-8
add sp,sp,-16
la ra,_frameSize_MAIN
lw ra,0(ra)
sub sp,sp,ra
sd t0,8(sp)
sd t1,16(sp)
sd t2,24(sp)
sd t3,32(sp)
sd t4,40(sp)
sd t5,48(sp)
sd t6,56(sp)
sd s2,64(sp)
sd s3,72(sp)
sd s4,80(sp)
sd s5,88(sp)
sd s6,96(sp)
sd s7,104(sp)
sd s8,112(sp)
sd s9,120(sp)
sd s10,128(sp)
sd s11,136(sp)
sd fp,144(sp)
fsw ft0,152(sp)
fsw ft1,156(sp)
fsw ft2,160(sp)
fsw ft3,164(sp)
fsw ft4,168(sp)
fsw ft5,172(sp)
fsw ft6,176(sp)
fsw ft7,180(sp)
.data
_CONSTANT_3: .ascii "Enter a number:\000"
.align 3
.text
la t0, _CONSTANT_3
mv a0,t0
jal _write_str
jal _read_int
mv t0, a0
la t1, _g_n
sw t0,0(t1)
la t5, _g_n
lw t0,0(t5)
.data
_CONSTANT_4: .word 1
.align 3
.text
lw t1, _CONSTANT_4
addw t0, t0, t1
la t1, _g_n
sw t0,0(t1)
la t5, _g_n
lw t0,0(t5)
.data
_CONSTANT_6: .word 1
.align 3
.text
lw t1, _CONSTANT_6
sgt t0, t0, t1
beqz t0, _elseLabel_5
jal _start_fact
mv t0, a0
sw t0,-4(fp)
j _ifExitLabel_5
_elseLabel_5:
.data
_CONSTANT_7: .word 1
.align 3
.text
lw t0, _CONSTANT_7
sw t0,-4(fp)
_ifExitLabel_5:
.data
_CONSTANT_8: .ascii "The factorial is \000"
.align 3
.text
la t0, _CONSTANT_8
mv a0,t0
jal _write_str
lw t0,-4(fp)
mv a0,t0
jal _write_int
.data
_CONSTANT_9: .ascii "\n\000"
.align 3
.text
la t0, _CONSTANT_9
mv a0,t0
jal _write_str
_end_MAIN:
ld t0,8(sp)
ld t1,16(sp)
ld t2,24(sp)
ld t3,32(sp)
ld t4,40(sp)
ld t5,48(sp)
ld t6,56(sp)
ld s2,64(sp)
ld s3,72(sp)
ld s4,80(sp)
ld s5,88(sp)
ld s6,96(sp)
ld s7,104(sp)
ld s8,112(sp)
ld s9,120(sp)
ld s10,128(sp)
ld s11,136(sp)
ld fp,144(sp)
flw ft0,152(sp)
flw ft1,156(sp)
flw ft2,160(sp)
flw ft3,164(sp)
flw ft4,168(sp)
flw ft5,172(sp)
flw ft6,176(sp)
flw ft7,180(sp)
ld ra,8(fp)
mv sp,fp
add sp,sp,8
ld fp,0(fp)
jr ra
.data
_frameSize_MAIN: .word 192
