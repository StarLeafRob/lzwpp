/*
 *      Author: rdt
 */

#pragma once

#include <array>
#include <stdint.h>

namespace lzw {

    class Writer {
    public:
        Writer(const int out_fd);
        void put(char c);
        void write(char* data, const size_t sz);
        void flush();
    private:
        const int out_fd_;
        std::array<char, 4096> buffer_;
        size_t bytes_;
    };

    class Decompressor {
    public:
        static constexpr size_t dict_size = (1 << 12) - 257;
        Decompressor(int in_fd, int out_fd);
        void run();
    private:
        void decode(const int16_t code);
        int16_t output_code(const int16_t code);

        struct entry {
            int16_t prev;
            uint8_t value;
        };
        const int input_fd_;
        Writer writer_;
        int16_t next_entry_ {0};
        int16_t previous_code_ = -1;
        std::array<uint8_t, dict_size> buffer_;
        std::array<entry, dict_size> dict_;
    };
}
