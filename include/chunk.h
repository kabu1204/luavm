//
// Created by 于承业 on 2023/10/17.
//

#ifndef LUAVM_CHUNK_H
#define LUAVM_CHUNK_H
#include "typedefs.h"
#include "string"
#include "vector"
#include "slice.h"

#define LUA_SIGNATURE       "\x1b\x4c\x75\x61"
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
    LuaNil(){}
};

typedef int64_t LuaInteger;
typedef double LuaNumber;
typedef bool LuaBoolean;

class LuaString {
public:
    LuaString(): str_("") {}
    LuaString(const std::string& str): str_(str) {}
    LuaString(std::string&& str): str_(std::move(str)) {}
    LuaString& operator=(std::string&& str) {
        str_ = std::move(str);
        return *this;
    }
    LuaString& operator=(LuaString&& lstr) {
        str_ = std::move(lstr.str_);
        return *this;
    }
    size_t size() const {
        return str_.size();
    }
    void Encode(char *p) {
        if (str_.size() == 0) {
            (*p) = 0x00;
            return;
        } else if (str_.size() >= 254) {
            (*p) = 0xFF;
            ++p;
            *(size_t*)(p) = str_.size() + 1;
            p += sizeof(size_t);
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
    friend class Constant;
    std::string str_;
};

class Constant {
public:
    Constant() {
        tag_ = ConstantTag::NIL;
    }
    Constant(Constant&& from) {
        *this = std::move(from);
    }
    Constant& operator=(Constant&& from) {
        tag_ = from.tag_;
        switch (tag_) {
            case ConstantTag::NIL:
                break;
            case ConstantTag::NUMBER:
                number_ = from.number_;
                break;
            case ConstantTag::INTEGER:
                integer_ = from.integer_;
                break;
            case ConstantTag::BOOLEAN:
                bool_ = from.bool_;
                break;
            case ConstantTag::STRING:
            case ConstantTag::SSTRING:
                string_ = std::move(from.string_);
                break;
        }
        return *this;
    }
    ~Constant() {
        if (tag_ == ConstantTag::SSTRING || tag_ == ConstantTag::STRING) {
            string_.~LuaString();
        }
    }
    void SetShortString(LuaString&& str) {
        tag_ = ConstantTag::SSTRING;
        string_ = std::move(str);
    }
    void SetLongString(LuaString&& str) {
        tag_ = ConstantTag::STRING;
        string_ = std::move(str);
    }
    void SetString(LuaString&& str) {
        tag_ = str.size() >= 255 ? ConstantTag::STRING : ConstantTag::SSTRING;
        string_ = std::move(str);
    }
    void SetNil() {
        tag_ = ConstantTag::NIL;
    }
    void SetBoolean(LuaBoolean b) {
        tag_ = ConstantTag::BOOLEAN;
        bool_ = b;
    }
    void SetNumber(LuaNumber n) {
        tag_ = ConstantTag::NUMBER;
        number_ = n;
    }
    void SetInterger(LuaInteger n) {
        tag_ = ConstantTag::INTEGER;
        integer_ = n;
    }
    [[nodiscard]] std::string String() const {
        std::string s("UNKNOWN CONSTANT TYPE");
        char *buf = nullptr;
        switch (tag_) {
            case ConstantTag::NIL:
                s.assign("<nil>");
                break;
            case ConstantTag::BOOLEAN:
                s = "<boolean>(" + std::string(bool(bool_)? "true" : "false") + ")";
                break;
            case ConstantTag::NUMBER:
                s = "<number>(" + std::to_string(number_) + ")";
                break;
            case ConstantTag::INTEGER:
                s = "<integer>(" + std::to_string(integer_) + ")";
                break;
            case ConstantTag::SSTRING:
            case ConstantTag::STRING:
                s = "<string>(\"" + string_.str_ + "\")";
                break;
        }
        return s;
    }
private:
    ConstantTag tag_;
    union {
        LuaBoolean bool_;
        LuaNumber number_;
        LuaInteger integer_;
        LuaString string_;
    };
};

class LocalVar {
public:
    LocalVar(){}
    LocalVar(std::string&& varName, uint32_t startPC, uint32_t endPC)
        : varName_(std::move(varName)),
            startPC_(startPC),
            endPC_(endPC) {}
    LocalVar(LocalVar&& from) {
        *this = std::move(from);
    }
    LocalVar& operator=(LocalVar&& from) {
        varName_ = std::move(from.varName_);
        startPC_ = from.startPC_;
        endPC_ = from.endPC_;
        return *this;
    }
private:
    friend class Chunk;
    std::string varName_;
    uint32_t startPC_;
    uint32_t endPC_;
};

class Upvalue {
public:
    Upvalue(byte_t inStack, byte_t idx)
        : inStack_(inStack), idx_(idx)
    {}
private:
    friend class Chunk;
    byte_t inStack_;
    byte_t idx_;
};

class Prototype {
public:
    Prototype(){}

    Prototype(Prototype&& from) {
        *this = std::move(from);
    }

    Prototype& operator=(Prototype&& from) {
        // TODO: use memcpy for first 5 members
        lineDefined_ = from.lineDefined_;
        lastLineDefined_ = from.lastLineDefined_;
        numParams_ = from.numParams_;
        isVarArg_ = from.isVarArg_;
        maxStackSize_ = from.maxStackSize_;
        source_ = std::move(from.source_);
        code_ = std::move(from.code_);
        constants_ = std::move(from.constants_);
        upvalues_ = std::move(from.upvalues_);
        protos_ = std::move(from.protos_);
        lineInfo_ = std::move(from.lineInfo_);
        locVars_ = std::move(from.locVars_);
        upvalueNames_ = std::move(from.upvalueNames_);

        return *this;
    }

private:
    friend class ChunkReader;
    friend class Chunk;
    uint32_t lineDefined_;
    uint32_t lastLineDefined_;
    byte_t numParams_;
    byte_t isVarArg_;
    byte_t maxStackSize_;
    std::string source_;
    std::vector<uint32_t> code_;
    std::vector<Constant> constants_;
    std::vector<Upvalue> upvalues_;
    std::vector<Prototype*> protos_;
    std::vector<uint32_t> lineInfo_;
    std::vector<LocalVar> locVars_;
    std::vector<std::string> upvalueNames_;
};

class ChunkHeader {
public:
    ChunkHeader(){}
private:
    friend class Chunk;
    byte_t signature_[4];   // default 0x1b4c7561
    byte_t version_;        // major * 16 + minor
    byte_t format_;         // default 0
    byte_t luacData[6];     // default 0x19930d0a1a0a
    byte_t cintSize_;       // sizeof(cint) default 4
    byte_t sizetSize_;      // sizeof(size_t) default 8
    byte_t instructionSize_;    // default 4
    byte_t luaIntegerSize_;     // default 8
    byte_t luaNumberSize_;      // default 8
    LuaInteger luacInt_;           // 0x5678
    LuaNumber luacNum_;            // 370.5
};

class ChunkReader;

class Chunk {
public:
    explicit Chunk(const char* data, size_t n);
    void Print(Prototype* f);
private:
    void PrintHeader(Prototype* f);
    void PrintDetail(Prototype* f);
    void PrintCode(Prototype* f);
    void CheckHeader(ChunkReader& reader);
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
    std::vector<uint32_t> ReadCode();
    std::vector<Constant> ReadConstants();
    Constant ReadConstant();
    std::vector<Upvalue> ReadUpvalues();
    std::vector<uint32_t> ReadLineInfo();
    std::vector<LocalVar> ReadLocVars();
    std::vector<std::string> ReadUpvalueNames();
    Prototype* ReadProto(const std::string& parentSource);
    std::vector<Prototype*> ReadProtos(const std::string& parentSource);
private:
    Slice data_;
};

#endif //LUAVM_CHUNK_H
