// SPDX-License-Identifier: MIT
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
namespace kastle2::talking {
constexpr size_t kFlashCapacity = 7680 * 1024;
constexpr size_t kHeaderSize = 32, kBankSize = 16, kPhraseSize = 80;
inline uint32_t Read32(const uint8_t *p) {
    return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}
inline uint32_t Crc32(const uint8_t *p, size_t size) {
    uint32_t crc = 0xffffffffu;
    while (size--) {
        crc ^= *p++;
        for (int i = 0; i < 8; ++i) crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1)));
    }
    return ~crc;
}
struct Phrase { char text[64]; uint8_t speed, mouth, throat, sing; uint8_t reserved[12]; };
static_assert(sizeof(Phrase) == kPhraseSize);
class PresetFile {
    const uint8_t *data_ = nullptr;
    size_t banks_offset_ = 0;
public:
    uint8_t num_banks = 0, num_samples = 0, num_scales = 0, num_rhythms = 0, sequencer_length = 16;
    bool Load(const uint8_t *data, size_t capacity) {
        data_ = nullptr;
        if (!data || capacity < 36 || memcmp(data, "k2ts", 4)) return false;
        const size_t size = Read32(data + 4);
        if (size < 36 || size > capacity || size > kFlashCapacity || data[12] != 1 || data[13]) return false;
        num_banks=data[14]; num_samples=data[15]; num_scales=data[16]; num_rhythms=data[17]; sequencer_length=data[18];
        if (!num_banks || num_banks>32 || !num_samples || num_samples>32 || !num_scales || num_scales>32 || !num_rhythms || num_rhythms>64 || sequencer_length<2 || sequencer_length>64) return false;
        banks_offset_ = kHeaderSize + 4 * (num_scales + num_rhythms);
        if (size != banks_offset_ + num_banks * (kBankSize + num_samples * kPhraseSize) + 4) return false;
        if (memcmp(data+size-4,"ahoj",4) || Crc32(data+kHeaderSize,size-kHeaderSize)!=Read32(data+8)) return false;
        for (size_t i=0; i<num_scales; ++i) if (!Read32(data+kHeaderSize+i*4) || Read32(data+kHeaderSize+i*4)>0xfff) return false;
        for (size_t b=0; b<num_banks; ++b) for (size_t p=0; p<num_samples; ++p) {
            const uint8_t *phrase=data+banks_offset_+b*(kBankSize+num_samples*kPhraseSize)+kBankSize+p*kPhraseSize;
            const void *zero=memchr(phrase,0,64);
            if (!zero || !phrase[0] || !phrase[64] || !phrase[65] || !phrase[66] || phrase[67]>1) return false;
            for (const uint8_t *c=phrase; c<static_cast<const uint8_t *>(zero); ++c) if (*c<32 || *c>126 || *c=='[' || *c==']') return false;
        }
        data_=data; return true;
    }
    uint32_t Scale(size_t i) const { return Read32(data_+kHeaderSize+4*i); }
    uint32_t Rhythm(size_t i) const { return Read32(data_+kHeaderSize+4*(num_scales+i)); }
    const uint8_t *Bank(size_t b) const { return data_+banks_offset_+b*(kBankSize+num_samples*kPhraseSize); }
    Phrase GetPhrase(size_t b, size_t p) const {
        Phrase result{};
        if (data_ && b<num_banks && p<num_samples) memcpy(&result,Bank(b)+kBankSize+p*kPhraseSize,sizeof result);
        return result;
    }
    uint32_t Color(size_t b) const { const auto *p=Bank(b)+8; return uint32_t(p[0])<<16|uint32_t(p[1])<<8|p[2]; }
};
}
