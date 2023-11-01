//
// Created by 于承业 on 2023/10/23.
//

#ifndef LUAVM_OPCODES_H
#define LUAVM_OPCODES_H
#include "stdint.h"
#include "typedefs.h"
#include "string"
#include "vector"

enum OpMode {
    IABC = 0,
    IABx,
    IAsBx,
    IAx,
};

enum {
    OP_MOVE = 0,
    OP_LOADK,
    OP_LOADKX,
    OP_LOADBOOL,
    OP_LOADNIL,
    OP_GETUPVAL,
    OP_GETTABUP,
    OP_GETTABLE,
    OP_SETTABUP,
    OP_SETUPVAL,
    OP_SETTABLE,
    OP_NEWTABLE,
    OP_SELF,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_MOD,
    OP_POW,
    OP_DIV,
    OP_IDIV,
    OP_BAND,
    OP_BOR,
    OP_BXOR,
    OP_SHL,
    OP_SHR,
    OP_UNM,
    OP_BNOT,
    OP_NOT,
    OP_LEN,
    OP_CONCAT,
    OP_JMP,
    OP_EQ,
    OP_LT,
    OP_LE,
    OP_TEST,
    OP_TESTSET,
    OP_CALL,
    OP_TAILCALL,
    OP_RETURN,
    OP_FORLOOP,
    OP_FORPREP,
    OP_TFORCALL,
    OP_TFORLOOP,
    OP_SETLIST,
    OP_CLOSURE,
    OP_VARARG,
    OP_EXTRAARG
};

enum OprandType {
    OpArgN = 0, // argument is not used
    OpArgU,     // argument is used
    OpArgR,     // argument is a register or a jump offset
    OpArgK      // argument is a constant or register/constant
};

class Opcode {
public:
    Opcode(byte_t testFlag,
           byte_t setAFlag,
           byte_t argBMode,
           byte_t argCMode,
           OpMode opMode,
           std::string&& name)
       :testFlag_(testFlag),
       setAFlag_(setAFlag),
       argBMode_(argBMode),
       argCMode_(argCMode),
       opMode_(opMode),
       name_(std::move(name))
    {}
    byte_t testFlag_; // operator is a test (next instruction must be a jump)
    byte_t setAFlag_; // instruction set register A
    byte_t argBMode_; // B arg mode
    byte_t argCMode_; // C arg mode
    OpMode opMode_; // op mode
    std::string name_;
};

extern std::vector<Opcode> opcodes;

#endif //LUAVM_OPCODES_H
