#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// instruction bitsize
#define ISZ 32
// opcode bitsize
#define OSZ 6
// register addressing size: 2^(RSZ+1) is the number of registers
#define RSZ 4
// maximum shift bitsize
#define MSZ 6
// small move bitsize (0: 8b, 1: 16b, 2: 32b, 3: 64b)
#define SSZ 2
// big copyable size: max 1024b
#define BSZ 10

struct Instr {
    union {
        // jmp
        struct { uint32_t op :OSZ, rest :ISZ-OSZ; };
        // indirect ops
        struct { uint32_t :OSZ, dst :RSZ, src  :RSZ; };
        // copy stack <=> reg
        struct { uint32_t :OSZ, :RSZ, ss :SSZ; int32_t boff :ISZ-OSZ-RSZ-SSZ; };
        struct { uint32_t :OSZ, :RSZ, bs :BSZ; int32_t soff :ISZ-OSZ-RSZ-BSZ; };
        // indirect put
        struct { uint32_t :OSZ,     :RSZ,      :RSZ, from :RSZ; };
        // non indirect ops, conditional jmp
        struct { uint32_t :OSZ,     :RSZ; int32_t ival :ISZ-OSZ-RSZ; };
        struct { uint32_t :OSZ,     :RSZ,         uval :ISZ-OSZ-RSZ; };
        // shifts
        struct { uint32_t :OSZ,     :RSZ,         nshift :MSZ; };
        // plain value
        struct { uint32_t all; };
    };
};

enum Opcode : uint8_t {
    RET = 0, CALL, NOP,
    JMP, JEQ, JNE, JGT, JLT, JGTU, JLTU,
    ADD, SUB, DIV, MUL, ADDU, SUBU, DIVU, MULU, MOD, MODU,
    ADDI, SUBI, DIVI, MULI, ADDIU, SUBIU, DIVIU, MULIU, MODI, MODIU,
    XOR, AND, OR, XORU, ANDU, ORU, XORI, ANDI, ORI, NOT,
    LS, LSU, RS, RSU,
    CSR, CSRU, CRS, PUTI, MV, MVU, BCP, RES, FRE
};

struct Reg {
    union {
        void*    ptr;
        int8_t   is;
        int16_t  i;
        int32_t  il;
        int64_t  ill;
        uint8_t  us;
        uint16_t u;
        uint32_t ul;
        uint64_t ull;
    };
};

struct Machine {
    struct {
        union {
            struct { Reg reg[32]; };
            struct {
                Reg ureg[30];
                Instr* ip;
                int8_t* sp;
            };
        };
        int8_t* stack;
    } mem;
    Machine(size_t size, const bool dirty = true);
    ~Machine();
    int8_t exec(Instr* main_text, int entry = 0);
};

static_assert(sizeof(Reg) == 8, "Reg must be 64b");
static_assert(sizeof(Instr) == 4, "Instr must be 32b");
static_assert(sizeof(int8_t) == 1, "int8_t must be 8b");
static_assert(sizeof(void*) == 8, "ptr must be 64b");
static_assert(sizeof(Machine) == 33*sizeof(void*), "Machine must be 33 ptrs");

#endif // VM_H
