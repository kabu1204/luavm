//
// Created by 于承业 on 2023/10/17.
//
#include "chunk.h"

/*
 * Parse a binary chunk from data
 */
Chunk::Chunk(const char *data) {
    ChunkReader reader((Slice(data)));
    if (reader.ReadUint32() != LUA_SIGNATURE) {
        throw std::runtime_error(std::string("Signature mismatch"));
    }

    if (reader.ReadByte() != LUAC_VERSION) {
        throw std::runtime_error(std::string("Version mismatch"));
    }

    if (reader.ReadByte() != LUAC_FORMAT) {
        throw std::runtime_error(std::string("Format mismatch"));
    }

    if (reader.ReadBytes(6) != LUAC_DATA) {
        throw std::runtime_error(std::string("LUAC_DATA mismatch"));
    }

    if (reader.ReadByte() != CINT_SIZE) {
        throw std::runtime_error(std::string("CINT_SIZE mismatch"));
    }

    if (reader.ReadByte() != SIZET_SIZE) {
        throw std::runtime_error(std::string("SIZET_SIZE mismatch"));
    }

    if (reader.ReadByte() != INSTRUCTION_SIZE) {
        throw std::runtime_error(std::string("INSTRUCTION_SIZE mismatch"));
    }

    if (reader.ReadByte() != LUA_INTEGER_SIZE) {
        throw std::runtime_error(std::string("LUA_INTEGER_SIZE mismatch"));
    }

    if (reader.ReadByte() != LUA_NUMBER_SIZE) {
        throw std::runtime_error(std::string("LUA_NUMBER_SIZE mismatch"));
    }

    if (reader.ReadLuaInteger() != LUAC_INT) {
        throw std::runtime_error(std::string("Endianness mismatch"));
    }

    if (reader.ReadLuaNumber() != LUAC_NUM) {
        throw std::runtime_error(std::string("Float format mismatch"));
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
