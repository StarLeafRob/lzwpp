/*
 *      Author: rdt
 */

#pragma once

#include <array>
#include <stdint.h>
#include <string>

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
        static constexpr size_t dict_size = 4095 - 256;
        Decompressor(int in_fd, int out_fd);
        void run();
    private:
        void decode(const int code);

        const int input_fd_;
        Writer writer_;
        int next_entry_;
        std::string previous_;
        std::array<std::string, dict_size> dict_;
    };
}
