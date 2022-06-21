#ifndef NECROCC_BACKEND_X86_64_ELFER_H
#define NECROCC_BACKEND_X86_64_ELFER_H
#include <stdio.h>

struct ElfBuffer{
    void* buffer;
    size_t capacity;
};

ElfBuffer* createElfBuffer(size_t capacity);

void deleteElfBuffer(ElfBuffer* elf);

void expandElfBuffer(ElfBuffer* elf, size_t new_capacity);

void writeElfBuffer(ElfBuffer* elf, size_t offset, const void* data, size_t n);

void writeToFileElfBuffer(const ElfBuffer* elf, const char* filename);

void genElf(const char* filename, const char* binBuffer, size_t n);

void writeElfHeader (ElfBuffer* elf, size_t extra_size);

void writeElfPHTable(ElfBuffer* elf, size_t binSz);

void writeElfSHTable(ElfBuffer* elf, size_t binSz);

#endif