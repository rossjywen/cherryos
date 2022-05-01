# introduction

CherryOS is a homemade unix-like kernel, I wrote it as I referrence linux-0.11 kernel.

# new features

Compared with linux0.11 kernel, there are such features added

1. all .s files are re-written into `nasm assembly format` and use `nasm` as assembler
2. kernel using pages are changed from a big amount of 4K-pages into 4M-pages
3. I use software instead of IA32 task switch mechanism to implement task switch