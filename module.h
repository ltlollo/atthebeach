#ifndef MODULE_H
#define MODULE_H

#include "vm.h"

struct Offset {
     union {
         struct { Instr* iptr; };
         struct { int8_t* sptr; };
         struct { uint64_t off: 63, type :1; };
     };
 };

struct Module {
    int8_t* data;
    size_t dsize;

    Instr* main_text;
    Offset* funptr_region;
    Offset* symptr_region;
    Offset* dnload_funptr_region;
    Offset* dnload_symptr_region;

    int load(char* path);
    void unload();
    void populate_regions();
    void resolve_internal();
    uint32_t resolve_dyn_symbols();
    int init(Machine* m, uint32_t stack_size);
};

#endif // MODULE_H
