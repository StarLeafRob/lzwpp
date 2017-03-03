/*
 *      Author: rdt
 */

#include "src/impl.hpp"
#include <array>
#include <stdint.h>
#include <string.h>
#include <iostream>

namespace lzw {

    Writer::Writer(std::ostream& out)
        : out_(out)
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
        out_.write(buffer_.data(), bytes_);
        bytes_ = 0;
    }

    Decompressor::Decompressor(std::istream& in, std::ostream& out)
        : in_(in)
        , writer_(out)
        , next_entry_(0)
        , dict_()
    {
    }

    void Decompressor::decode(const int code) {
        if (not previous_.empty()) {
            int shifted = code - 256;
            if (shifted > next_entry_) {
                throw std::runtime_error("corrupt input");
            }
            else if (shifted == next_entry_ ) {
                // code not in dictionary yet.
                // output previous code, output first letter of previous code, add combined to dict
                previous_.push_back(previous_[0]);
                dict_[next_entry_++] = previous_;
                writer_.write((char*)previous_.data(), previous_.size());
            }
            else {
                dict_[next_entry_] = previous_;
                if (code < 256) {
                    dict_[next_entry_++].push_back(code);
                    writer_.put(char(code));
                    previous_.clear();
                    previous_.push_back(char(code));
                } else {
                    dict_[next_entry_++].push_back(dict_[shifted][0]);
                    previous_ = dict_[shifted];
                    writer_.write((char*)previous_.data(), previous_.size());
                }
            }
            if (next_entry_ == dict_size) {
                next_entry_ = 0;
                previous_.clear();
            }
        }
        else {
            writer_.put(char(code));
            previous_.push_back(code);
        }
    }

    void Decompressor::run() {
        std::array<uint8_t, 4096 * 3> raw;
        uint8_t* pos;
        uint8_t* end;
        size_t bytes;
        while (in_) {
            in_.read((char*)raw.data(), raw.size());
            bytes = in_.gcount();
            pos = raw.begin();
            end = raw.begin() + bytes;
            for(; pos < end; pos += 3) {
                if (pos + 2 == end and in_.eof()) {
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
