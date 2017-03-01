/*
 *      Author: rdt
 */

#include "src/decoder.hpp"
#include "src/decompress.hpp"

#include <array>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

namespace lzw {

    using namespace std;

    class Writer {
    public:
        Writer(const int out_fd)
            : out_fd_(out_fd)
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
            size_t t;
            char* pos = buffer_.data();
            while(bytes_ != 0) {
                t = ::write(out_fd_, pos, bytes_);
                if (t < 0)
                    exit(1);
                bytes_ -= t;
                pos += t;
            }
        }

    private:
        const int out_fd_;
        array<char, 4096> buffer_;
        size_t bytes_;
    };

    constexpr size_t read_buf_size = 4096 * 3;

    bool decompress_loop(int in_fd, Writer& writer) {
        uint8_t a, b, c;
        array<uint8_t, read_buf_size> buffer;
        Decoder<Writer> codec(writer);
        uint8_t* pos;
        uint8_t* end;
        size_t bytes;
        bool eol = false;
        while(true) {
            bytes = read(in_fd, buffer.data(), buffer.size());
            if (bytes == 0) {
                return true;
            } else if (bytes < 0) {
                return false;
            } else if (bytes != read_buf_size) {
                eol = true;
            }
            pos = buffer.begin();
            end = buffer.begin() + bytes;
            while(pos != end) {
                a = *pos++;
                b = *pos++;
                if (pos == end and eol)
                    return codec.decode((a << 8) + b);
                c = *pos++;
                if (not codec.decode((a << 4) + (b >> 4)))
                    return false;
                if (not codec.decode(((b & 0xf) << 8) + c))
                    return false;
            }
        };
        return false;
    }

    bool decompress (const int in_fd, const int out_fd) {
        Writer writer(out_fd);
        bool result = decompress_loop(in_fd, writer);
        writer.flush();
        return result;
    }
}


