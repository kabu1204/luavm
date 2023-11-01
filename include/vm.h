//
// Created by 于承业 on 2023/10/23.
//

#ifndef LUAVM_VM_H
#define LUAVM_VM_H
#include "typedefs.h"
#include "opcodes.h"
#include "string"

#define MAXARG_Bx (1<<18-1)        // 262143
#define MAXARG_sBx (MAXARG_Bx >> 1) // 131071


class Instruction {
public:
    [[nodiscard]] uint32_t Opcode() const {
        return instruction_ & 0x3f;
    }

    void ABC(uint32_t* a, uint32_t* b, uint32_t* c) const {
        *a = (instruction_ >> 6 & 0xff);
        *b = (instruction_ >> 14 & 0x1ff);
        *c = (instruction_ >> 23 & 0x1ff);
    }

    void ABx(uint32_t* a, uint32_t* bx) const {
        *a = (instruction_ >> 6 & 0xff);
        *bx = (instruction_ >> 14);
    }

    void AsBx(uint32_t* a, uint32_t* sbx) const {
        ABx(a, sbx);
        *sbx -= MAXARG_sBx;
    }

    void Ax(uint32_t* a) const {
        *a = instruction_ >> 6;
    }

    std::string OpName() const {
        return opcodes[Opcode()].name_;
    }

    OpMode OpMode() const {
        return opcodes[Opcode()].opMode_;
    }

    byte_t BMode() const {
        return opcodes[Opcode()].argBMode_;
    }

    byte_t CMode() const {
        return opcodes[Opcode()].argCMode_;
    }
private:
    uint32_t instruction_;
};

#endif //LUAVM_VM_H
