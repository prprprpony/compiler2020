#!/bin/bash
sed  's/main/MAIN/g' $1 > /tmp/__temp.c
./parser /tmp/__temp.c
riscv64-linux-gnu-gcc -O0 -static main.S
