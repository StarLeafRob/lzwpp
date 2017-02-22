/*
 *      Author: rdt
 */

#pragma once

#include <array>
#include <stdint.h>
#include <assert.h>

namespace lzw {

    // output provides 2 functions
    // void put(char);
    // void write(char*, size_t);
    template<typename Output>
    class Decoder {
    public:
        Decoder(Output& output);
        bool decode(const int16_t code);
    private:
        int16_t output_code(const int16_t code);

        struct entry {
            int16_t prev;
            uint8_t value;
        };

        static constexpr size_t dict_size = (1 << 12) - 256;

        Output& output_;
        std::array<uint8_t, 1 << 12> buffer_;
        std::array<entry, dict_size> dict_;
        int16_t next_entry_ {0};
        int16_t previous_code_ = -1;
    };

    template <typename Output>
    Decoder<Output>::Decoder(Output& output)
        : output_(output)
    {}

    template<typename Output>
    bool Decoder<Output>::decode(const int16_t code) {
        if (previous_code_ != -1) {
            int16_t first;
            if (code < next_entry_ + 256) {
                first = output_code(code);
            }
            else if (code == next_entry_ + 256) {
                // code not in dictionary yet.
                // output previous code, output first letter of previous code, add combined to dict
                first = output_code(previous_code_);
                output_.put(char(first));
            }
            else {
                return false;
            }
            dict_[next_entry_].value = first;
            dict_[next_entry_].prev = previous_code_;
            previous_code_ = code;
            next_entry_++;
            if (next_entry_ == dict_size) {
                previous_code_ = first;
                next_entry_ = 0;
            }
        }
        else {
            output_.put(char(code));
            previous_code_ = code;
        }
        return true;
    }

    // returns the first letter of the word coded for
    template <typename Output>
    int16_t Decoder<Output>::output_code(const int16_t code) {
        auto copy = code;
        auto pos = buffer_.rbegin();
        while (copy > 255) {
            assert(pos != buffer_.rend());
            const auto& entry = dict_[copy - 256];
            *pos = entry.value;
            copy = entry.prev;
            pos++;
        }
        *pos = uint8_t(copy);
        uint8_t* start = &(*pos);
        size_t bytes = std::distance(start, buffer_.data() + buffer_.size());
        output_.write((char*)start, bytes);
        return copy;
    }
}


