/*
 *      Author: rdt
 */

#include "lib/decompress.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>
#include <stdint.h>
namespace lzw {

    using namespace std;

    struct entry {
        int16_t prev;
        uint8_t value;
    };

    struct Dictionary {
        static constexpr size_t dict_size = (1 << 12) - 256;

        int16_t output_code_with_buf(fstream& out, const int16_t code) {
            // return -1 on error, return first first character of dictionary entry on succcess
            auto copy = code;
            auto pos = output_buf_.rbegin();
            while (copy > 255) {
                if (pos == output_buf_.rend())
                    return -1;
                const auto& entry = impl_[copy - 256];
                *pos = entry.value;
                copy = entry.prev;
                pos++;
            }
            *pos = uint8_t(copy);
            uint8_t* start = &(*pos);
            size_t bytes = std::distance(start, output_buf_.data() + output_buf_.size());
            out.write((char*)start, bytes);
            return copy;
        }

        int16_t output_code_with_vector(fstream& out, const int16_t code) {
            // return first character of dictionary entry
            std::vector<uint8_t> output_buf;
            auto copy = code;
            while (true) {
                if (copy > 255) {
                    output_buf.push_back(impl_[copy - 256].value);
                    copy = impl_[copy - 256].prev;
                } else {
                    output_buf.push_back(uint8_t(copy));
                    break;
                }
            }
            std::reverse(output_buf.begin(), output_buf.end());
            out.write((char*)output_buf.data(), output_buf.size());
            return copy;
        }

        Result lookup(fstream& out, const int16_t code) {
            if (previous_code_ != -1) {
                int16_t first;
                if (code < next_entry_ + 256) {
                    first = output_code_with_buf(out, code);
                    if (first == -1)
                        first = output_code_with_vector(out, code);
                }
                else if (code == next_entry_ + 256) {
                    // code not in dictionary yet.
                    // output previous code, output first letter of previous code, add combined to dict
                    first = output_code_with_buf(out, previous_code_);
                    if (first == -1)
                        first = output_code_with_vector(out, previous_code_);
                    out.put((char&)first);
                }
                else
                    return Result::CORRUPT_INPUT;

                impl_[next_entry_].value = first;
                impl_[next_entry_].prev = previous_code_;
                previous_code_ = code;
                next_entry_++;
                if (next_entry_ == dict_size) {
                    previous_code_ = first;
                    next_entry_ = 0;
                }
            }
            else {
                out.put(char(code));
                previous_code_ = code;
            }
            return out ? Result::SUCCESS : Result::IO_ERROR;
        }
        std::array<uint8_t, 1 << 12> output_buf_;
        std::array<entry, dict_size> impl_;
        int16_t next_entry_ {0}; // index of the next value in the dictionary
        int16_t previous_code_ = -1;
    };

#define RETURN_ON_ERROR(x) do { if (x != Result::SUCCESS) return x;} while(false)

    Result decompress_loop(fstream& in, fstream& out) {
        int16_t code1;
        int16_t code2;
        uint8_t a, b, c;
        Dictionary dict;
        Result r;
        while(true) {
            if (not in.get((char&)a))
                break;
            if (not in.get((char&)b))
                return Result::IO_ERROR;
            if (not in.get((char&)c)) {
                code1 = (a << 8) + b;
                r = dict.lookup(out, code1);
                return r;
            } else {
                code1 = (a << 4) + (b >> 4);
                code2 = ((b & 0xf) << 8) + c;
                r = dict.lookup(out, code1);
                RETURN_ON_ERROR(r);
                r = dict.lookup(out, code2);
                RETURN_ON_ERROR(r);
            }
        }
        return r;
    }

    Result decompress (const char* input_p, const char* output_p) {
        std::fstream input(input_p, ios_base::binary | ios_base::in);
        std::fstream output(output_p ,ios_base::binary | ios_base::out);
        if (not input or not output)
            return Result::IO_ERROR;
        auto r = decompress_loop(input, output);
        input.close();
        output.close();
        return r;
    }

}


