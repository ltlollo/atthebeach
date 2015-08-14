#include "module.h"

int Module::load(char* path) {
    long size;
    FILE* infile = fopen(path, "rb");
    if (infile == nullptr) {
        perror("fopen");
        return -1;
    }
    if (fseek(infile, 0, SEEK_END) == -1) {
        perror("fseek");
        goto ERR;
    }
    if ((size = ftell(infile)) == -1) {
        perror("ftell");
        goto ERR;
    }
    dsize = size;
    rewind(infile);
    data = (int8_t*)malloc((dsize+1)*sizeof(int8_t)+sizeof(uint64_t));
    if (data == nullptr) {
        perror("malloc");
        goto ERR;
    }
    *(data+dsize) = 0;
    *((uint64_t*)(data+dsize+1)) = 0;
    if (fread(data, dsize, 1, infile) != dsize) {
        perror("fread");
        goto ERR;
    }
    if (dsize < sizeof(int8_t)+2*sizeof(uint64_t)+sizeof(Instr)) {
        fprintf(stderr, "unsupported format");
    }
    ver = *(uint8_t*)data++;
    return 0;
ERR:
    fclose(infile);
    exit(EXIT_FAILURE);
}

void Module::unload() {
    free(data);
}

uint32_t Module::resolve_dyn_symbols() {
    fprintf(stderr, "unimplemented\n");
    return 0;
}

void Module::populate_regions() {
    Offset* region = (Offset*)data;
    funptr_region = region;
    while (region->type == 1) {
        ++region;
    }
    if (region >= (Offset*)data+dsize) {
        fprintf(stderr, "missing function region\n");
        exit(EXIT_FAILURE);
    }
    dnload_funptr_region = region;
    while (*(uint64_t*)region != 0) {
        ++region;
    }
    if (region >= (Offset*)data+dsize) {
        fprintf(stderr, "missing end of function region\n");
        exit(EXIT_FAILURE);
    }
    (region++)->iptr = nullptr;
    if (region >= (Offset*)data+dsize) {
        fprintf(stderr, "missing symbol region\n");
        exit(EXIT_FAILURE);
    }
    symptr_region = region;
    while (region->type == 1) {
        ++region;
    }
    if (region >= (Offset*)data+dsize) {
        fprintf(stderr, "missing symbol region\n");
        exit(EXIT_FAILURE);
    }
    dnload_funptr_region = region;

    while (*(uint64_t*)region != 0) {
        ++region;
    }
    if (region >= (Offset*)data+dsize) {
        fprintf(stderr, "missing end of symbol region\n");
        exit(EXIT_FAILURE);
    }
    (region++)->iptr = nullptr;
    if (region >= (Offset*)data+dsize) {
        fprintf(stderr, "missing main text region\n");
        exit(EXIT_FAILURE);
    }
    Instr* currfn = (Instr*)region;
    main_text = currfn;
#ifdef NDEBUG
    if (funptr_region == dnload_funptr_region) {
        fprintf(stderr, "no funptr region\n");
    }
    if (dnload_funptr_region->iptr == nullptr) {
        fprintf(stderr, "no dnload funptr region\n");
    }
    if (symptr_region == dnload_symptr_region) {
        fprintf(stderr, "no symptr region\n");
    }
    if (dnload_symptr_region->sptr == nullptr) {
        fprintf(stderr, "no dnload symptr region\n");
    }
#endif
    size_t funptr_region_size = dnload_funptr_region - funptr_region,
            symptr_region_size = dnload_symptr_region - symptr_region;
    if (funptr_region_size < symptr_region_size) {
        fprintf(stderr, "symbol region doesn't include all the function"
                        " signatures\n");
        exit(EXIT_FAILURE);
    }
}

void Module::resolve_internal() {
    Offset* region = funptr_region;
    for (; region != dnload_funptr_region; ++region) {
        region->iptr = (Instr*)(data+region->off);
    }
    region = symptr_region;
    for (; region != dnload_symptr_region; ++region) {
        region->sptr = (int8_t*)(data+region->off);
    }
}

int Module::init(Machine* pm = nullptr, uint32_t stack_size = 2048) {
    return pm ? pm->exec(main_text) : Machine(stack_size).exec(main_text);
}
