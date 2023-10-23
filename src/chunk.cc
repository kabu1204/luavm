//
// Created by 于承业 on 2023/10/17.
//
#include "chunk.h"

/*
 * Parse a binary chunk from data
 */
Chunk::Chunk(const char *data, size_t n) {
    ChunkReader reader((Slice(data, n)));
    CheckHeader(reader);
    sizeUpvalue_ = reader.ReadByte();
    mainFunc_ = reader.ReadProto("");
    Print(mainFunc_);
}

void Chunk::CheckHeader(ChunkReader& reader) {
    Slice s;
    if ((s = reader.ReadBytes(4)) != LUA_SIGNATURE) {
        throw std::runtime_error(std::string("Signature mismatch"));
    }
    memcpy(&header_.signature_, s.data(), sizeof(header_.signature_));

    if ((header_.version_ = reader.ReadByte()) != LUAC_VERSION) {
        throw std::runtime_error(std::string("Version mismatch"));
    }

    if ((header_.format_ = reader.ReadByte()) != LUAC_FORMAT) {
        throw std::runtime_error(std::string("Format mismatch"));
    }

    if ((s = reader.ReadBytes(6)) != LUAC_DATA) {
        throw std::runtime_error(std::string("LUAC_DATA mismatch"));
    }
    memcpy(&header_.luacData, s.data(), sizeof(header_.luacData));

    if ((header_.cintSize_ = reader.ReadByte()) != CINT_SIZE) {
        throw std::runtime_error(std::string("CINT_SIZE mismatch"));
    }

    if ((header_.sizetSize_ = reader.ReadByte()) != SIZET_SIZE) {
        throw std::runtime_error(std::string("SIZET_SIZE mismatch"));
    }

    if ((header_.instructionSize_ = reader.ReadByte()) != INSTRUCTION_SIZE) {
        throw std::runtime_error(std::string("INSTRUCTION_SIZE mismatch"));
    }

    if ((header_.luaIntegerSize_ = reader.ReadByte()) != LUA_INTEGER_SIZE) {
        throw std::runtime_error(std::string("LUA_INTEGER_SIZE mismatch"));
    }

    if ((header_.luaNumberSize_ = reader.ReadByte()) != LUA_NUMBER_SIZE) {
        throw std::runtime_error(std::string("LUA_NUMBER_SIZE mismatch"));
    }

    if ((header_.luacInt_ = reader.ReadLuaInteger()) != LUAC_INT) {
        throw std::runtime_error(std::string("Endianness mismatch"));
    }

    if ((header_.luacNum_ = reader.ReadLuaNumber()) != LUAC_NUM) {
        throw std::runtime_error(std::string("Float format mismatch"));
    }
}

void Chunk::PrintHeader(Prototype* f) {
    std::string funcType = "main";
    if(f->lineDefined_ > 0) {
        funcType.assign("function");
    }

    std::string varArgFlag = "";
    if(f->isVarArg_ > 0) {
        varArgFlag = "+";
    }

    printf("\n%s <%s:%d, %d> (%zu instructions)\n",
           funcType.c_str(), f->source_.c_str(),
           f->lineDefined_, f->lineDefined_, f->code_.size());

    printf("%d%s params, %d slots, %zu upvalues, ",
           f->numParams_, varArgFlag.c_str(), f->maxStackSize_, f->upvalues_.size());

    printf("%zu locals, %zu constants, %zu functions\n",
           f->locVars_.size(), f->constants_.size(), f->protos_.size());
}

void Chunk::PrintCode(Prototype *f) {
    static char buf[4096];
    for (int i = 0; i < f->code_.size(); ++i) {
        buf[0] = '-'; buf[1] = '\0';
        if (!f->lineInfo_.empty()) {
            snprintf(buf, sizeof(buf), "%d", f->lineInfo_[i]);
        }
        printf("\t%d\t[%s]\t0x%08X\n", i+1, buf, f->code_[i]);
    }
}

void Chunk::PrintDetail(Prototype *f) {
    int i;

    printf("constants (%zu):\n", f->constants_.size());
    i = 1;
    for (auto& k:f->constants_) {
        printf("\t%d\t%s\n", i++, k.String().c_str());
    }

    printf("locals (%zu):\n", f->locVars_.size());
    i = 0;
    for (auto& l:f->locVars_) {
        printf("\t%d\t%s\t%d\t%d\n", i++,
               l.varName_.c_str(),
               l.startPC_,
               l.endPC_);
    }

    auto upvalName = [this](Prototype* f_, byte_t idx) -> std::string {
        return f_->upvalueNames_.empty() ? "-" : f_->upvalueNames_[idx];
    };
    printf("upvalues (%zu):\n", f->upvalues_.size());
    i = 0;
    for (auto &v:f->upvalues_) {
        printf("\t%d\t%s\t%d\t%d\n",
               i, upvalName(f, i).c_str(), v.inStack_, v.idx_);
        i++;
    }
}

void Chunk::Print(Prototype* f) {
    PrintHeader(f);
    PrintCode(f);
    PrintDetail(f);
    for (auto sub:f->protos_) {
        Print(sub);
    }
}

byte_t ChunkReader::ReadByte() {
    byte_t b = *reinterpret_cast<const byte_t*>(data_.data());
    data_.remove_prefix(sizeof(byte_t));
    return b;
}

uint32_t ChunkReader::ReadUint32() {
    uint32_t i = *reinterpret_cast<const uint32_t*>(data_.data());
    data_.remove_prefix(sizeof(uint32_t));
    return i;
}

uint64_t ChunkReader::ReadUint64() {
    uint64_t i = *reinterpret_cast<const uint64_t*>(data_.data());
    data_.remove_prefix(sizeof(uint64_t));
    return i;
}

LuaInteger ChunkReader::ReadLuaInteger() {
    return LuaInteger(ReadUint64());
}

LuaNumber ChunkReader::ReadLuaNumber() {
    double f = *reinterpret_cast<const double*>(data_.data());
    data_.remove_prefix(sizeof(double ));
    return f;
}

std::string ChunkReader::ReadLuaString() {
    uint64_t size = ReadByte();

    if (size == 0) {
        return "";
    } else if (size == 0xFF) {
        size = ReadUint64();
    }
    Slice bytes = ReadBytes(size - 1);
    return {bytes.data(), bytes.size()};
}

Slice ChunkReader::ReadBytes(uint32_t n) {
    Slice s(data_.data(), n);
    data_.remove_prefix(n);
    return s;
}

Prototype *ChunkReader::ReadProto(const std::string& parentSource) {
    auto proto = new Prototype();
    proto->source_ = ReadLuaString();
    if (proto->source_.empty()) {
        proto->source_ = parentSource; // copy
    }
    proto->lineDefined_ = ReadUint32();
    proto->lastLineDefined_ = ReadUint32();
    proto->numParams_ = ReadByte();
    proto->isVarArg_ = ReadByte();
    proto->maxStackSize_ = ReadByte();
    proto->code_ = ReadCode();
    proto->constants_ = ReadConstants();
    proto->upvalues_ = ReadUpvalues();
    proto->protos_ = ReadProtos(proto->source_);
    proto->lineInfo_ = ReadLineInfo();
    proto->locVars_ = ReadLocVars();
    proto->upvalueNames_ = ReadUpvalueNames();

    return proto;
}



std::vector<uint32_t> ChunkReader::ReadCode() {
    auto size = ReadUint32();
    std::vector<uint32_t> code(size);
    for (auto& c:code) {
        c = ReadUint32();
    }
    return code;
}

std::vector<Constant> ChunkReader::ReadConstants() {
    uint32_t size = ReadUint32();
    std::vector<Constant> v(size);
    for (auto& c:v) {
        c = std::move(ReadConstant());
    }
    return v;
}

Constant ChunkReader::ReadConstant() {
    auto tag = ConstantTag(ReadByte());
    Constant constant;
    switch (tag) {
        case ConstantTag::BOOLEAN:
            constant.SetBoolean(ReadByte() != 0);
            break;
        case ConstantTag::INTEGER:
            constant.SetInterger(ReadLuaInteger());
            break;
        case ConstantTag::NUMBER:
            constant.SetNumber(ReadLuaNumber());
            break;
        case ConstantTag::SSTRING:
        case ConstantTag::STRING:
            constant.SetString(ReadLuaString());
            break;
        case ConstantTag::NIL:
            break;
    }
    return constant;
}

std::vector<Upvalue> ChunkReader::ReadUpvalues() {
    auto size = ReadUint32();
    std::vector<Upvalue> v;
    v.reserve(size);
    for (int i = 0; i < size; ++i) {
        v.emplace_back(ReadByte(), ReadByte());
    }
    return v;
}

std::vector<uint32_t> ChunkReader::ReadLineInfo() {
    std::vector<uint32_t> v(ReadUint32());
    for (auto& n:v) {
        n = ReadUint32();
    }
    return v;
}

std::vector<LocalVar> ChunkReader::ReadLocVars() {
    auto size = ReadUint32();
    std::vector<LocalVar> v;
    v.reserve(size);
    for(int i = 0; i < size; ++i) {
        v.emplace_back(ReadLuaString(), ReadUint32(), ReadUint32());
    }
    return v;
}

std::vector<std::string> ChunkReader::ReadUpvalueNames() {
    auto size = ReadUint32();
    std::vector<std::string> v;
    v.reserve(size);
    for (int i = 0; i < size; ++i) {
        v.emplace_back(ReadLuaString());
    }
    return v;
}

std::vector<Prototype *> ChunkReader::ReadProtos(const std::string& parentSource) {
    auto size = ReadUint32();
    std::vector<Prototype*> v;
    v.reserve(size);
    for (int i = 0; i < size; ++i) {
        v.push_back(ReadProto(parentSource));
    }
    return v;
}

