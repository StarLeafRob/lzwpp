/*
 *      Author: rdt
 */

#pragma once

#include <array>
#include <stdint.h>
#include <string>
#include <iosfwd>

namespace lzw {

    class Writer {
    public:
        static constexpr size_t write_buf_size = 4096 * 16;
        Writer(std::ostream& out);
        void put(char c);
        void write(char* data, const size_t sz);
        void flush();
    private:
        std::ostream& out_;
        std::array<char, write_buf_size> buffer_;
        size_t bytes_;
    };

    class Decompressor {
    public:
        static constexpr size_t dict_size = 4095 - 256;
        Decompressor(std::istream&, std::ostream&);
        void run();
    private:
        void decode(const int code);

        std::istream& in_;
        Writer writer_;
        int next_entry_;
        std::string previous_;
        std::array<std::string, dict_size> dict_;
    };

    inline void decompress(std::istream& i, std::ostream& o) {
        Decompressor comp(i, o);
        comp.run();
    }
}
