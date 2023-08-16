# LLVM-EXEGESIS-DEFREG RBX 20000
movq (%rbx), %rax
movq %rbx, %rdx
addq $8192, %rdx
movq (%rdx), %rax