clang -emit-llvm -S -c original_code.c -o original.ll
clang++  -g main.cpp -o main `llvm-config --cxxflags --ldflags --system-libs --libs orcjit native mcjit` -fexceptions 
