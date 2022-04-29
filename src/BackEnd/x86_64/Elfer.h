#ifndef NECROCC_BACKEND_X86_64_ELFER_H
#define NECROCC_BACKEND_X86_64_ELFER_H
#include <stdio.h>

void genElf(const char* filename, const char* binBuffer, size_t n);

void writeElfHeader(FILE* file);

void writeElfPHTable(FILE* file, size_t n);
void writeElfSHTable(FILE* file, size_t n);

#endif