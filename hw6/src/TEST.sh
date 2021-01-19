#!/bin/bash
cat MYMAIN.cpp $1 > TMP.cpp && g++ TMP.cpp && ./a.out > ANS
./run.sh $1 && qemu-riscv64-static a.out > RES
diff ANS RES
