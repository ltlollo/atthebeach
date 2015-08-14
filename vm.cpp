#include "vm.h"

Machine::Machine(size_t size, const bool dirty) {
    mem.stack = (int8_t*)malloc(sizeof(int8_t)*(size+1));
    if (!dirty) {
        bzero(mem.stack, size);
    }
    if (mem.stack == nullptr) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
}

Machine::~Machine() {
    free(mem.stack);
}

int8_t Machine::exec(Instr* main_text, int entry) {
    uint32_t stack_size = 0;
    mem.ip = main_text+entry;
    mem.sp = mem.stack+1;
    while (true) {
        switch(mem.ip->op) {
NOINCR:
        case RET:
            if ((*(Instr**)mem.sp) == nullptr) {
                return *mem.stack;
            }
            mem.ip = *(Instr**)mem.sp;
            stack_size = *((uint32_t*)(mem.sp+4));
            mem.sp -= stack_size;
            // fall through
        case CALL: mem.sp += stack_size;
            *((Instr**)(mem.sp)) = mem.ip;
            *((uint32_t*)(mem.sp+4)) = stack_size;
            stack_size = 0;
            goto NOINCR;
        case JMP: mem.ip += mem.ip->rest;
            goto NOINCR;
        case JEQ:
            if (mem.reg[mem.ip->dst].ull == 0) {
                mem.ip += mem.ip->ival;
                goto NOINCR;
            } break;
        case JNE:
            if (mem.reg[mem.ip->dst].ull != 0) {
                mem.ip += mem.ip->ival;
                goto NOINCR;
            } break;
        case JGT:
            if (mem.reg[mem.ip->dst].ill > mem.reg[mem.ip->dst].ill) {
                mem.ip += mem.ip->ival;
                goto NOINCR;
            } break;
        case JGTU:
            if (mem.reg[mem.ip->dst].ull > mem.reg[mem.ip->dst].ull) {
                mem.ip += mem.ip->ival;
                goto NOINCR;
            } break;
        case JLT:
            if (mem.reg[mem.ip->dst].ill < mem.reg[mem.ip->dst].ill) {
                mem.ip += mem.ip->ival;
                goto NOINCR;
            } break;
        case JLTU:
            if (mem.reg[mem.ip->dst].ull < mem.reg[mem.ip->dst].ull) {
                mem.ip += mem.ip->ival;
                goto NOINCR;
            } break;
        case ADD:   mem.reg[mem.ip->dst].ill += mem.ip->ival;             break;
        case ADDU:  mem.reg[mem.ip->dst].ull += mem.ip->ival;             break;
        case SUB:   mem.reg[mem.ip->dst].ill -= mem.ip->ival;             break;
        case SUBU:  mem.reg[mem.ip->dst].ull -= mem.ip->ival;             break;
        case MUL:   mem.reg[mem.ip->dst].ill *= mem.ip->ival;             break;
        case MULU:  mem.reg[mem.ip->dst].ull *= mem.ip->ival;             break;
        case DIV:   mem.reg[mem.ip->dst].ill /= mem.ip->ival;             break;
        case DIVU:  mem.reg[mem.ip->dst].ull /= mem.ip->ival;             break;
        case MOD:   mem.reg[mem.ip->dst].ill %= mem.ip->ival;             break;
        case MODU:  mem.reg[mem.ip->dst].ull %= mem.ip->ival;             break;
        case ADDI:  mem.reg[mem.ip->dst].ill += mem.reg[mem.ip->src].ill; break;
        case ADDIU: mem.reg[mem.ip->dst].ull += mem.reg[mem.ip->src].ull; break;
        case SUBI:  mem.reg[mem.ip->dst].ill -= mem.reg[mem.ip->src].ill; break;
        case SUBIU: mem.reg[mem.ip->dst].ull -= mem.reg[mem.ip->src].ull; break;
        case MULI:  mem.reg[mem.ip->dst].ill *= mem.reg[mem.ip->src].ill; break;
        case MULIU: mem.reg[mem.ip->dst].ull *= mem.reg[mem.ip->src].ull; break;
        case DIVI:  mem.reg[mem.ip->dst].ill /= mem.reg[mem.ip->src].ill; break;
        case DIVIU: mem.reg[mem.ip->dst].ull /= mem.reg[mem.ip->src].ull; break;
        case MODI:  mem.reg[mem.ip->dst].ill %= mem.reg[mem.ip->src].ill; break;
        case MODIU: mem.reg[mem.ip->dst].ull %= mem.reg[mem.ip->src].ull; break;
        case XORI:  mem.reg[mem.ip->dst].ull ^= mem.reg[mem.ip->src].ull; break;
        case ANDI:  mem.reg[mem.ip->dst].ull &= mem.reg[mem.ip->src].ull; break;
        case ORI:   mem.reg[mem.ip->dst].ull |= mem.reg[mem.ip->src].ull; break;
        case NOT:   mem.reg[mem.ip->dst].ull =! mem.reg[mem.ip->dst].ull; break;
        case XOR:   mem.reg[mem.ip->dst].ill ^= mem.ip->ival;             break;
        case XORU:  mem.reg[mem.ip->dst].ull ^= mem.ip->uval;             break;
        case AND:   mem.reg[mem.ip->dst].ill &= mem.ip->ival;             break;
        case ANDU:  mem.reg[mem.ip->dst].ull &= mem.ip->uval;             break;
        case OR:    mem.reg[mem.ip->dst].ill |= mem.ip->ival;             break;
        case ORU:   mem.reg[mem.ip->dst].ull |= mem.ip->uval;             break;
        case LS:    mem.reg[mem.ip->dst].ill <<= mem.ip->nshift;          break;
        case LSU:   mem.reg[mem.ip->dst].ull <<= mem.ip->nshift;          break;
        case RS:    mem.reg[mem.ip->dst].ill >>= mem.ip->nshift;          break;
        case RSU:   mem.reg[mem.ip->dst].ull >>= mem.ip->nshift;          break;
        case CSR:
            switch(mem.ip->ss) {
            case 0: mem.reg[mem.ip->dst].ill = *(int8_t *)(mem.sp+mem.ip->boff);
                break;
            case 1: mem.reg[mem.ip->dst].ill = *(int16_t*)(mem.sp+mem.ip->boff);
                break;
            case 2: mem.reg[mem.ip->dst].ill = *(int32_t*)(mem.sp+mem.ip->boff);
                break;
            case 3: mem.reg[mem.ip->dst].ill = *(int64_t*)(mem.sp+mem.ip->boff);
                break;
            } break;
        case CSRU:
            switch(mem.ip->ss) {
            case 0: mem.reg[mem.ip->dst].ull = *(uint8_t *)(mem.sp+mem.ip->boff);
                break;
            case 1: mem.reg[mem.ip->dst].ull = *(uint16_t*)(mem.sp+mem.ip->boff);
                break;
            case 2: mem.reg[mem.ip->dst].ull = *(uint32_t*)(mem.sp+mem.ip->boff);
                break;
            case 3: mem.reg[mem.ip->dst].ull = *(uint64_t*)(mem.sp+mem.ip->boff);
                break;
            } break;
        case CRS:
            switch(mem.ip->ss) {
            case 0: *(uint8_t *)(mem.sp+stack_size) = mem.reg[mem.ip->dst].us;
                break;
            case 1: *(uint16_t*)(mem.sp+stack_size) = mem.reg[mem.ip->dst].u;
                break;
            case 2: *(uint32_t*)(mem.sp+stack_size) = mem.reg[mem.ip->dst].ul;
                break;
            case 3: *(uint64_t*)(mem.sp+stack_size) = mem.reg[mem.ip->dst].ull;
                break;
            }
            stack_size += (1<<(mem.ip->ss));
            break;
        case PUTI:
            switch(mem.ip->ss) {
            case 0: *(uint8_t *)(mem.reg[mem.ip->dst].ptr) =
                        mem.reg[mem.ip->from].us; break;
            case 1: *(uint16_t*)(mem.reg[mem.ip->dst].ptr) =
                        mem.reg[mem.ip->from].u; break;
            case 2: *(uint32_t*)(mem.reg[mem.ip->dst].ptr) =
                        mem.reg[mem.ip->from].ul; break;
            case 3: *(uint64_t*)(mem.reg[mem.ip->dst].ptr) =
                        mem.reg[mem.ip->from].ull; break;
            } break;
        case MV: stack_size -= mem.ip->ss;
            switch(mem.ip->ss) {
            case 0: mem.reg[mem.ip->dst].ill = *(int8_t *)(mem.sp+stack_size);
                break;
            case 1: mem.reg[mem.ip->dst].ill = *(int16_t*)(mem.sp+stack_size);
                break;
            case 2: mem.reg[mem.ip->dst].ill = *(int32_t*)(mem.sp+stack_size);
                break;
            case 3: mem.reg[mem.ip->dst].ill = *(int64_t*)(mem.sp+stack_size);
                break;
            } break;
        case MVU: stack_size -= mem.ip->ss;
            switch(mem.ip->ss) {
            case 0: mem.reg[mem.ip->dst].ill = *(int8_t *)(mem.sp+stack_size);
                break;
            case 1: mem.reg[mem.ip->dst].ill = *(int16_t*)(mem.sp+stack_size);
                break;
            case 2: mem.reg[mem.ip->dst].ill = *(int32_t*)(mem.sp+stack_size);
                break;
            case 3: mem.reg[mem.ip->dst].ill = *(int64_t*)(mem.sp+stack_size);
                break;
            } break;
        case BCP:
            memcpy(mem.reg[mem.ip->dst].ptr, mem.sp+mem.ip->soff, mem.ip->bs+1);
            break;
        case RES: stack_size += mem.ip->rest; break;
        case FRE: stack_size -= mem.ip->rest; break;
        case NOP: break;
        default:
            fprintf(stderr, "%c: unknown opcode\n", mem.ip->op);
            exit(EXIT_FAILURE);
        }
        ++mem.ip;
    }
}
