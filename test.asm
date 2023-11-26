# LLVM-EXEGESIS-DEFREG RBX 20000
movq (%rbx), %rax
addq $0x2000, %rbx
movq (%rbx), %rax
addq $0x3000, %rbx
movq (%rbx), %rax
subq $0x5000, %rbx
movq $0x40000, %rdi
movq (%rdi), %rax
