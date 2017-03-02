/*
 *      Author: rdt
 */

#include "src/impl.hpp"
#include <array>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

namespace lzw {

    Writer::Writer(const int out_fd)
        : out_fd_(out_fd)
        , bytes_(0)
    {}

    void Writer::put(char c) {
        buffer_[bytes_++] = c;
        if (bytes_ == buffer_.size())
            flush();
    }

    void Writer::write(char* data, const size_t sz) {
        size_t free_space = buffer_.size() - bytes_;
        size_t t = (sz <= free_space) ? sz : free_space;
        memcpy(buffer_.data() + bytes_, data, t);
        bytes_ += t;
        if (bytes_ == buffer_.size())
            flush();
        if (t < sz)
            write(data + t, sz - t);
    }
    void Writer::flush() {
        size_t t;
        char* pos = buffer_.data();
        while(bytes_ != 0) {
            t = ::write(out_fd_, pos, bytes_);
            if (t < 0)
                throw std::runtime_error("write syscall returned error");
            bytes_ -= t;
            pos += t;
        }
    }

    Decompressor::Decompressor(int in_fd, int out_fd)
        : input_fd_(in_fd)
        , writer_(out_fd)
    {}

    void Decompressor::decode(const int16_t code) {
        if (previous_code_ != -1) {
            int16_t first;
            auto next_offset = next_entry_ + 256;
            if (code > next_offset)
                throw std::runtime_error("corrupt input");
            else if (code == next_offset) {
                // code not in dictionary yet.
                // output previous code, output first letter of previous code, add combined to dict
                first = output_code(previous_code_);
                writer_.put(char(first));
            }
            else {
                first = output_code(code);
            }
            dict_[next_entry_].value = first;
            dict_[next_entry_].prev = previous_code_;
            previous_code_ = code;
            next_entry_ += 1;
            if (next_entry_ == dict_size) {
                next_entry_ = 0;
                previous_code_ = -1;
            }
        }
        else {
            writer_.put(char(code));
            previous_code_ = code;
        }
    }

    int16_t Decompressor::output_code(const int16_t code) {
        auto copy = code;
        auto pos = buffer_.rbegin();
        while (copy > 255) {
            const auto& entry = dict_[copy - 256];
            *pos = entry.value;
            copy = entry.prev;
            pos++;
        }
        *pos = uint8_t(copy);
        uint8_t* start = &(*pos);
        size_t bytes = std::distance(start, buffer_.data() + buffer_.size());
        writer_.write((char*)start, bytes);
        return copy;
    }

    void Decompressor::run() {
        std::array<uint8_t, 4096 * 3> raw;
        uint8_t* pos;
        uint8_t* end;
        size_t bytes;
        bool eof = false;
        while (not eof) {
            bytes = read(input_fd_, raw.data(), raw.size());
            if (bytes < 0) {
                throw std::runtime_error("read syscall returned error");
            } else if (bytes != raw.size()) {
                eof = true;
            }
            pos = raw.begin();
            end = raw.begin() + bytes;
            for(; pos < end; pos += 3) {
                if (pos + 2 == end and eof) {
                    decode((pos[0] << 8) + (pos[1]));
                    break;
                }
                decode((pos[0] << 4) + (pos[1] >> 4));
                decode(((pos[1] & 0xf) << 8) + pos[2]);
            }
        }
        writer_.flush();
    }
}
