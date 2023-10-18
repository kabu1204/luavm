//
// Created by 于承业 on 2023/10/17.
//

#ifndef LUAVM_CHUNK_H
#define LUAVM_CHUNK_H
#include "typedefs.h"
#include "string"
#include "vector"
#include "slice.h"

#define LUA_SIGNATURE       0x1b4c7561
#define LUAC_VERSION        0x53
#define LUAC_FORMAT         0
#define LUAC_DATA           "\x19\x93\x0d\x0a\x1a\x0a"
#define CINT_SIZE           sizeof(int)
#define SIZET_SIZE          sizeof(size_t)
#define INSTRUCTION_SIZE    4
#define LUA_INTEGER_SIZE    8
#define LUA_NUMBER_SIZE     8
#define LUAC_INT            0x5678
#define LUAC_NUM            370.5

enum ConstantTag {
    NIL = 0x00,
    BOOLEAN = 0x01,
    NUMBER = 0x03,
    INTEGER = 0x13,
    SSTRING = 0x04, // short string
    STRING = 0x14   // long string
};

class LuaNil {
public:
    LuaNil();
};

class LuaBoolean {

};

typedef int64_t LuaInteger;
typedef double LuaNumber;

class LuaString {
public:
    LuaString();
    void Put(char *p) {
        if (str_.size() == 0) {
            (*p) = 0x00;
            return;
        } else if (str_.size() >= 254) {
            (*p) = 0xFF;
            ++p;
            *(size_t*)(p) = str_.size() + 1;
            p += sizeof(size_t)
        } else {
            (*p) = byte_t(str_.size() + 1);
            ++p;
        }
        memcpy(p, str_.data(), str_.size());
    }
    void DecodeFrom(const Slice& data) {
        // TODO
    }
private:
    std::string str_;
};

class Constant {
public:
    Constant();
private:
    ConstantTag tag_;
    union {
        LuaNil nil_;
        LuaNumber number_;
        LuaInteger integer_;
        LuaString string_;
    };
};

class LocalVar {
public:
    LocalVar();
private:
    std::string varName_;
    uint32_t startPC_;
    uint32_t endPC_;
};

class Upvalue {
public:
    Upvalue();
private:
    byte_t inStack_;
    byte_t idx_;
};

class Prototype {
public:
    Prototype();
private:
    std::string source_;
    uint32_t lineDefined_;
    uint32_t lastLineDefined_;
    byte_t numParams_;
    byte_t isVarArg_;
    byte_t maxStackSize_;
    std::vector<uint32_t> code_;
    std::vector<void*> constants_;
    std::vector<Upvalue> upvalues_;
    std::vector<Prototype*> protos_;
    std::vector<uint32_t> lineInfo_;
    std::vector<LocalVar> locVars_;
    std::vector<std::string> upvalueNames_;
};

class ChunkHeader {
public:
    ChunkHeader();
private:
    byte_t signature_[4];   // default 0x1b4c7561
    byte_t version_;        // major * 16 + minor
    byte_t format_;         // default 0
    byte_t luacData[6];     // default 0x19930d0a1a0a
    byte_t cintSize_;       // sizeof(cint) default 4
    byte_t sizetSize_;      // sizeof(size_t) default 8
    byte_t instructionSize_;    // default 4
    byte_t luaIntegerSize_;     // default 8
    byte_t luaNumberSize_;      // default 8
    int64_t luacInt_;           // 0x5678
    double luacNum_;            // 370.5
};

class Chunk {
public:
    explicit Chunk(const char* data);
private:
    ChunkHeader header_;
    byte_t sizeUpvalue_;
    Prototype* mainFunc_;
};

class ChunkReader {
public:
    ChunkReader(const Slice& data): data_(data) {}
    ChunkReader(const std::string& data): data_(data) {}
    byte_t ReadByte();
    Slice ReadBytes(uint32_t n);
    uint32_t ReadUint32();
    uint64_t ReadUint64();
    LuaInteger ReadLuaInteger();
    LuaNumber ReadLuaNumber();
    std::string ReadLuaString();    // TODO: LuaString
private:
    Slice data_;
};

#endif //LUAVM_CHUNK_H
