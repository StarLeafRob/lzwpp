/*
 *      Author: rdt
 */

#include "src/decoder.hpp"
#include "src/decompress.hpp"

#include <array>
#include <memory>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <iostream>

namespace lzw { namespace iostream {

    using namespace std;

    class Writer {
    public:
        Writer(fstream& out)
            : out_(out)
            , bytes_(0)
        {}

        void put(char c) {
            buffer_[bytes_++] = c;
            if (bytes_ == buffer_.size())
                flush();
        }

        void write(char* data, const size_t sz) {
            size_t free_space = buffer_.size() - bytes_;
            size_t t = (sz <= free_space) ? sz : free_space;
            memcpy(buffer_.data() + bytes_, data, t);
            bytes_ += t;
            if (bytes_ == buffer_.size())
                flush();
            if (t < sz)
                write(data + t, sz - t);
        }
        void flush() {
            out_.write(buffer_.data(), bytes_);
            bytes_ = 0;
        }

    private:
        fstream& out_;
        std::array<char, 4096> buffer_;
        size_t bytes_;
    };

    constexpr size_t read_buf_size = 4096 * 3;

    bool decompress_loop(fstream& in, Writer& writer) {
        uint8_t a, b, c;
        array<uint8_t, read_buf_size> buffer;
        Decoder<Writer> codec(writer);
        while(in) {
            in.read((char*)buffer.data(), buffer.size());
            auto pos = buffer.begin();
            const auto end = pos + in.gcount();
            bool eof = in.eof();
            while(pos != end) {
                a = *pos++;
                b = *pos++;
                if (pos == end and eof) {
                    return codec.decode((a << 8) + b);
                }
                c = *pos++;
                if (not codec.decode((a << 4) + (b >> 4)))
                    return false;
                if (not codec.decode(((b & 0xf) << 8) + c))
                    return false;
            }
            if (eof)
                return true;
        }
        return false;
    }

    bool decompress (const char* input_p, const char* output_p) {
        fstream input(input_p, ios_base::binary | ios_base::in);
        fstream output(output_p ,ios_base::binary | ios_base::out);
        Writer writer(output);
        if (not input or not output)
            throw std::runtime_error("invalid files");
        bool result = decompress_loop(input, writer);
        writer.flush();
        input.close();
        output.close();
        return result;
    }
}}


