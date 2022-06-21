#include "Elfer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <MGK/Logger.h>
#include <MGK/Utils.h>

const Elf64_Word PHNum = 6;
const Elf64_Word SHNum = 9;
const Elf64_Addr PltSz = 0x30;

const Elf64_Addr StartOff   = 0x400000;
const Elf64_Addr EntryPoint = StartOff + 0x1000 + PltSz;
const size_t Offset_Mask    = ~0xffful;

const size_t ELF_INIT_SIZE = 0x4000;
const size_t ELF_BUF_ROUND = 0x1000;

const Elf64_Ehdr EHdr{
    .e_ident = {
        ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3, // 0x7f 'E' 'L' 'F'
        /*[EI_CLASS     ] = */ ELFCLASS64, 
        /*[EI_DATA      ] = */ ELFDATA2LSB, 
        /*[EI_VERSION   ] = */ EV_CURRENT,
        /*[EI_OSABI     ] = */ ELFOSABI_NONE,
        /*[EI_ABIVERSION] = */ 0,
        0,0,0,0, 0,0,0
    },
    .e_type    = ET_EXEC,
    .e_machine = EM_X86_64,
    .e_version = EV_CURRENT,

    .e_entry     = EntryPoint,

    .e_phoff     = sizeof(Elf64_Ehdr),
    .e_shoff     = 0x3500,

    .e_flags     = 0,
    .e_ehsize    = sizeof(Elf64_Ehdr),	

    .e_phentsize = sizeof(Elf64_Phdr), 
    .e_phnum     = PHNum,

    .e_shentsize = sizeof(Elf64_Shdr), 
    .e_shnum     = SHNum,
    .e_shstrndx  = 3,
};


static unsigned char PltSeg[PltSz] = {
    /*1000:*/	0xff, 0x35, 0x02, 0x20, 0x00, 0x00,    	// push    rip + 0x2002         # 3008 <_GLOBAL_OFFSET_TABLE_+0x8>
    /*1006:*/	0xff, 0x25, 0x04, 0x20, 0x00, 0x00,    	// jmp    [rip + 0x2004]        # 3010 <_GLOBAL_OFFSET_TABLE_+0x10>
    /*100c:*/	0x0f, 0x1f, 0x40, 0x00,          	    // nopl   rax + 0x0

// 0000000000001010 <ncc_in@plt>:
    /*1010:*/	0xff, 0x25, 0x02, 0x20, 0x00, 0x00,    	// jmp    [rip + 0x2002]        # 3018 <ncc_in>
    /*1016:*/	0x68, 0x00, 0x00, 0x00, 0x00,       	// push   0x0
    /*101b:*/	0xe9, 0xe0, 0xff, 0xff, 0xff,       	// jmp    1000 <ncc_in@plt-0x10>

// 0000000000001020 <ncc_out@plt>:
    /*1020:*/	0xff, 0x25, 0xfa, 0x1f, 0x00, 0x00,     // jmp    [rip + 0x1ffa]        # 3020 <ncc_out>
    /*1026:*/	0x68, 0x01, 0x00, 0x00, 0x00,       	// push   0x1
    /*102b:*/	0xe9, 0xd0, 0xff, 0xff, 0xff,           // jmp    1000 <ncc_in@plt-0x10>
};

static Elf64_Rela RelaSeg[2] = {
    {
        .r_offset = StartOff + 0x3018, 
        .r_info   = ELF64_R_INFO(1, R_X86_64_JUMP_SLOT),
        .r_addend = 0,
    },
    {
        .r_offset = StartOff + 0x3020,
        .r_info   = ELF64_R_INFO(2, R_X86_64_JUMP_SLOT),
        .r_addend = 0,
    }
};

//                       1       8        16
//                       v       v        v                  35v       44v      52v     60 v      68v          v  78
const char StrTab[] = "\0ncc_in\0ncc_out\0libncc.so\0.dynamic\0.got.plt\0.interp\0.dynsym\0.strtab\0.rela.plt\0.text";
const char Interp[] = "/lib64/ld-linux-x86-64.so.2";

static const Elf64_Sym SymTab[3] = {
    {
        .st_name  = 0,
        .st_info  = 0,
        .st_other = 0,
        .st_shndx = 0,
        .st_value = 0,
        .st_size  = 0,
    },
    {
        .st_name  = 1,  // ncc_in
        .st_info  = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE),
        .st_other = ELF64_ST_VISIBILITY(STV_DEFAULT),
        .st_shndx = STN_UNDEF,
        .st_value = 0,
        .st_size  = 0,
    },
    {
        .st_name  = 8,  // ncc_out
        .st_info  = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE),
        .st_other = ELF64_ST_VISIBILITY(STV_DEFAULT),
        .st_shndx = STN_UNDEF,
        .st_value = 0,
        .st_size  = 0,
    },
};

const size_t DTNum = 10;
static Elf64_Dyn DynTable[DTNum] = {
    {
        .d_tag = DT_NEEDED,                         // [0]
        .d_un  = {.d_val = 16},
    },
    {
        .d_tag = DT_STRTAB,                         // [1]
        .d_un  = {.d_ptr = StartOff + 0x400},
    },
    {
        .d_tag = DT_SYMTAB,                         // [2]
        .d_un  = {.d_ptr = StartOff + 0x300},
    },
    {
        .d_tag = DT_STRSZ,                          // [3]
        .d_un  = {.d_val = sizeof(StrTab)},
    },
    {
        .d_tag = DT_SYMENT,                         // [4]
        .d_un  = {.d_val = sizeof(Elf64_Sym)},
    },
    {
        .d_tag = DT_PLTGOT,                         // [5]
        .d_un  = {.d_ptr = StartOff + 0x3000},
    },
    {
        .d_tag = DT_PLTRELSZ,                       // [6]
        .d_un  = {.d_val = sizeof(RelaSeg)},
    },
    {
        .d_tag = DT_PLTREL,                         // [7]
        .d_un  = {.d_val = DT_RELA},
    },
    {
        .d_tag = DT_JMPREL,                         // [8]
        .d_un  = {.d_ptr = StartOff + 0x500},
    },
    {
        .d_tag = DT_NULL,                           // [9]
        .d_un  = {},
    },
};


static Elf64_Shdr SHTable[SHNum] = {
    {   
        .sh_name      = 0,                  //[0]: Null
        .sh_type      = 0,
        .sh_flags     = 0,
        .sh_addr      = 0,
        .sh_offset    = 0,
        .sh_size      = 0,
        .sh_link      = 0,
        .sh_info      = 0,
        .sh_addralign = 0,
        .sh_entsize   = 0,
    },
    {   
        .sh_name      = 44,                  //[1]: Interp
        .sh_type      = SHT_PROGBITS,
        .sh_flags     = SHF_ALLOC,
        .sh_addr      = StartOff + 0x200,
        .sh_offset    = 0x200,
        .sh_size      = sizeof(Interp),
        .sh_link      = 0,
        .sh_info      = 0,
        .sh_addralign = 0x8,
        .sh_entsize   = 0,
    },
    {   
        .sh_name      = 52,                  //[2]: Dynsym
        .sh_type      = SHT_DYNSYM,
        .sh_flags     = SHF_ALLOC,
        .sh_addr      = StartOff + 0x300,
        .sh_offset    = 0x300,
        .sh_size      = sizeof(SymTab),
        .sh_link      = 3,
        .sh_info      = 1,
        .sh_addralign = 0x8,
        .sh_entsize   = sizeof(Elf64_Sym),
    },
    {   
        .sh_name      = 60,                  //[3]: StrTab
        .sh_type      = SHT_STRTAB,
        .sh_flags     = SHF_ALLOC,
        .sh_addr      = StartOff + 0x400,
        .sh_offset    = 0x400,
        .sh_size      = sizeof(StrTab),
        .sh_link      = 0,
        .sh_info      = 0,
        .sh_addralign = 0x1,
        .sh_entsize   = 0,
    },
    {   
        .sh_name      = 68,                  //[4]: .rela.plt
        .sh_type      = SHT_RELA,
        .sh_flags     = SHF_ALLOC | SHF_INFO_LINK,
        .sh_addr      = StartOff + 0x500,
        .sh_offset    = 0x500,
        .sh_size      = sizeof(RelaSeg),
        .sh_link      = 2,
        .sh_info      = 8,
        .sh_addralign = 0x8,
        .sh_entsize   = sizeof(Elf64_Rela),
    },
    {   
        .sh_name      = 74,                  //[5]: .plt
        .sh_type      = SHT_PROGBITS,
        .sh_flags     = SHF_ALLOC | SHF_EXECINSTR,
        .sh_addr      = StartOff + 0x1000,
        .sh_offset    = 0x1000,
        .sh_size      = sizeof(PltSeg),
        .sh_link      = 0,
        .sh_info      = 0,
        .sh_addralign = 0x10,
        .sh_entsize   = 0x10,
    },
    {   
        .sh_name      = 78,                  //[6]: .text
        .sh_type      = SHT_PROGBITS,
        .sh_flags     = SHF_ALLOC | SHF_EXECINSTR,
        .sh_addr      = EntryPoint,
        .sh_offset    = 0x1030,
        .sh_size      = 0,
        .sh_link      = 0,
        .sh_info      = 0,
        .sh_addralign = 0x8,
        .sh_entsize   = 0,
    },
    {   
        .sh_name      = 26,                  //[7]: .dynamic
        .sh_type      = SHT_DYNAMIC,
        .sh_flags     = SHF_ALLOC | SHF_WRITE,
        .sh_addr      = StartOff + 0x2ee0,
        .sh_offset    =            0x2ee0,
        .sh_size      = sizeof(Elf64_Dyn) * DTNum,
        .sh_link      = 3,
        .sh_info      = 0,
        .sh_addralign = 0x8,
        .sh_entsize   = sizeof(Elf64_Dyn),
    },
    {   
        .sh_name      = 35,                  //[8]: .got.plt
        .sh_type      = SHT_PROGBITS,
        .sh_flags     = SHF_ALLOC | SHF_WRITE,
        .sh_addr      = StartOff + 0x3000,
        .sh_offset    =            0x3000,
        .sh_size      = 0x80,
        .sh_link      = 0,
        .sh_info      = 0,
        .sh_addralign = 0x8,
        .sh_entsize   = 0x8,
    },
};

void genElf(const char* filename, const char* binBuffer, size_t bufSize){
    LOG_ASSERT(bufSize != 0);
    LOG_INFO("File size: %zu", bufSize);
    size_t extra_size = bufSize & ELF_BUF_ROUND;

    ElfBuffer* elf = createElfBuffer(ELF_INIT_SIZE + extra_size);
    
    LOG_ASSERT(elf != NULL);
    
    writeElfHeader (elf, extra_size);
    writeElfPHTable(elf, bufSize);

    writeElfBuffer(elf, 0x200, Interp, sizeof(Interp));

    writeElfBuffer(elf, 0x300, SymTab, sizeof(SymTab));

    writeElfBuffer(elf, 0x400, StrTab, sizeof(StrTab));


    RelaSeg[0].r_offset += extra_size;          //Patching extra size offsets in Rels segment
    RelaSeg[1].r_offset += extra_size;

    writeElfBuffer(elf, 0x500, RelaSeg, sizeof(RelaSeg));

    RelaSeg[0].r_offset -= extra_size;          // Return to default.
    RelaSeg[1].r_offset -= extra_size;

    *(unsigned*)&(PltSeg[0x02]) += (unsigned)extra_size;      //Patching extra size offsets in Plt segment
    *(unsigned*)&(PltSeg[0x08]) += (unsigned)extra_size;
    *(unsigned*)&(PltSeg[0x12]) += (unsigned)extra_size;
    *(unsigned*)&(PltSeg[0x22]) += (unsigned)extra_size;

    writeElfBuffer(elf, 0x1000, PltSeg, PltSz);

    *(unsigned*)&(PltSeg[0x02]) -= (unsigned)extra_size;
    *(unsigned*)&(PltSeg[0x08]) -= (unsigned)extra_size;
    *(unsigned*)&(PltSeg[0x12]) -= (unsigned)extra_size;
    *(unsigned*)&(PltSeg[0x22]) -= (unsigned)extra_size;  
    
    writeElfBuffer(elf, 0x1000 + PltSz, binBuffer, bufSize);

    
    DynTable[6].d_un.d_ptr += extra_size;

    writeElfBuffer(elf, 0x2ee0 + extra_size, DynTable, sizeof(Elf64_Dyn) * DTNum);

    DynTable[6].d_un.d_ptr -= extra_size;

    Elf64_Addr addr = 0x401016;
    
    writeElfBuffer(elf, 0x3018 + extra_size, &addr, sizeof(Elf64_Addr));

    addr += 0x10;

    writeElfBuffer(elf, 0x3018 + extra_size + sizeof(Elf64_Addr),
                         &addr, sizeof(Elf64_Addr));

    writeElfSHTable(elf, bufSize);

    writeToFileElfBuffer(elf, filename);
    deleteElfBuffer(elf);
}



void writeElfHeader (ElfBuffer* elf, size_t extra_size){
    Elf64_Ehdr hdr = EHdr;

    hdr.e_shoff += extra_size;

    writeElfBuffer(elf, 0, &hdr, sizeof(Elf64_Ehdr));
}

void writeElfPHTable(ElfBuffer* elf, size_t binSz){
    size_t extra_size = binSz & ELF_BUF_ROUND;

    Elf64_Phdr hdr_phd = {
        .p_type   = PT_PHDR,	
        .p_flags  = PF_R,		
        .p_offset = sizeof(Elf64_Ehdr),		
        .p_vaddr  = StartOff + sizeof(Elf64_Ehdr),		
        .p_paddr  = StartOff + sizeof(Elf64_Ehdr),		
        .p_filesz = sizeof(Elf64_Phdr) * PHNum,		
        .p_memsz  = sizeof(Elf64_Phdr) * PHNum,		
        .p_align  = 0x8,
    };

    Elf64_Phdr hdr_interp = {
        .p_type   = PT_INTERP,	
        .p_flags  = PF_R,		
        .p_offset = 0x200,		
        .p_vaddr  = StartOff + 0x200,		
        .p_paddr  = StartOff + 0x200,		
        .p_filesz = sizeof(Interp),		
        .p_memsz  = sizeof(Interp),		
        .p_align  = 0x1,
    };

    Elf64_Phdr hdr_filler = {
        .p_type   = PT_LOAD,	
        .p_flags  = PF_R,		
        .p_offset = 0,		
        .p_vaddr  = StartOff + 0x0000,		
        .p_paddr  = StartOff + 0x0000,		
        .p_filesz = 0x1000,		
        .p_memsz  = 0x1000,		
        .p_align  = 0x1000,
    };

    Elf64_Phdr hdr_text = {
        .p_type   = PT_LOAD,	
        .p_flags  = PF_R | PF_X,		
        .p_offset = 0x1000,		
        .p_vaddr  = StartOff + 0x1000,	
        .p_paddr  = StartOff + 0x1000,		
        .p_filesz = binSz + PltSz,		
        .p_memsz  = binSz + PltSz,		
        .p_align  = 0x1000,
    };

    Elf64_Phdr hdr_dynld = {
        .p_type   = PT_LOAD,	
        .p_flags  = PF_R | PF_W,		
        .p_offset =            0x2ee0 + extra_size,		
        .p_vaddr  = StartOff + 0x2ee0 + extra_size,		
        .p_paddr  = StartOff + 0x2ee0 + extra_size,		
        .p_filesz = 0x200,		
        .p_memsz  = 0x1200,		
        .p_align  = 0x1000,
    };

    Elf64_Phdr hdr_dyn = {
        .p_type   = PT_DYNAMIC,	
        .p_flags  = PF_R | PF_W,		
        .p_offset =            0x2ee0 + extra_size,		
        .p_vaddr  = StartOff + 0x2ee0 + extra_size,		
        .p_paddr  = StartOff + 0x2ee0 + extra_size,		
        .p_filesz = 0x120,		
        .p_memsz  = 0x120,		
        .p_align  = 0x1000,
    };


    writeElfBuffer(elf, sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * 0, &hdr_phd,    sizeof(Elf64_Phdr));
    writeElfBuffer(elf, sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * 1, &hdr_interp, sizeof(Elf64_Phdr));
    writeElfBuffer(elf, sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * 2, &hdr_filler, sizeof(Elf64_Phdr));
    writeElfBuffer(elf, sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * 3, &hdr_text,   sizeof(Elf64_Phdr));
    writeElfBuffer(elf, sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * 4, &hdr_dynld,  sizeof(Elf64_Phdr));
    writeElfBuffer(elf, sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * 5, &hdr_dyn,    sizeof(Elf64_Phdr));
}



void writeElfSHTable(ElfBuffer* elf, size_t binSz){
    size_t extra_size = binSz & ELF_BUF_ROUND;
    
    SHTable[6].sh_size    = binSz;
    SHTable[7].sh_addr   += extra_size;
    SHTable[7].sh_offset += extra_size;
    SHTable[8].sh_addr   += extra_size;
    SHTable[8].sh_offset += extra_size;

    writeElfBuffer(elf, 0x3500 + extra_size, SHTable ,sizeof(SHTable));

    SHTable[6].sh_size    = 0;
    SHTable[7].sh_addr   -= extra_size;
    SHTable[7].sh_offset -= extra_size;
    SHTable[8].sh_addr   -= extra_size;
    SHTable[8].sh_offset -= extra_size;
}

ElfBuffer* createElfBuffer(size_t capacity){
    ElfBuffer* elf = (ElfBuffer*)mgk_calloc(1, sizeof(ElfBuffer));
    elf->buffer = mgk_calloc(capacity, 1);
    elf->capacity = capacity;
    return elf;
}

void deleteElfBuffer(ElfBuffer* elf){
    if(elf == NULL) return;
    free(elf->buffer);
    free(elf);
}

void expandElfBuffer(ElfBuffer* elf, size_t new_capacity){
    elf->buffer = mgk_realloc(elf->buffer, new_capacity, 1);
    elf->capacity = new_capacity;
}


void writeElfBuffer(ElfBuffer* elf, size_t offset, const void* data, size_t n){
    LOG_ASSERT(elf != NULL);
    if(offset + n > elf->capacity) expandElfBuffer(elf, ((offset + n) & ~(ELF_BUF_ROUND - 1)) + ELF_BUF_ROUND);
    memcpy((char*)elf->buffer + offset, data, n);
}

void writeToFileElfBuffer(const ElfBuffer* elf, const char* filename){
    LOG_ASSERT(elf != NULL);
    LOG_ASSERT(filename != NULL);
    
    FILE* file = fopen(filename, "w");
    
    LOG_ASSERT(file != NULL);

    fwrite(elf->buffer, elf->capacity, 1, file);
    fclose(file);
}