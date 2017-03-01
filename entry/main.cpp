/*
 *      Author: rdt
 */

#include <array>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <cstdio>

using namespace std;

namespace lzw {

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

        static constexpr size_t dict_size = (1 << 12) - 257;

        Output& output_;
        int16_t next_entry_ {0};
        int16_t previous_code_ = -1;
        std::array<uint8_t, dict_size> buffer_;
        std::array<entry, dict_size> dict_;
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
            next_entry_ += 1;
            if (next_entry_ == dict_size) {
                next_entry_ = 0;
                previous_code_ = -1;
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

int main (int argc, char* argv[]) {

    int input_fd = 0;
    int output_fd = 0;

    if (argc == 1) {
        input_fd = STDIN_FILENO;
        output_fd = STDOUT_FILENO;
    } else if (argc == 3) {
        input_fd = open(argv[1], O_RDONLY);
        output_fd = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        if (input_fd == -1) {
            fprintf(stderr, "invalid input filename: %s\n", argv[1]);
            return 1;
        }
        if (output_fd == -1) {
            fprintf(stderr, "invalid output filename: %s\n", argv[2]);
            return 1;
        }
    } else {
        fprintf(stderr, "--------help---------\n"
                "./program <> to use stdin and stdout\n OR \n"
                "./program <input_file> <output_file>\n");

        return 1;
    }
    bool success = lzw::decompress(input_fd, output_fd);
    if (input_fd > STDERR_FILENO)
        close(input_fd);
    if (output_fd > STDERR_FILENO)
        close(output_fd);
    if (not success)
        fprintf(stderr, "decompressing failed\n");
    return success ? 0 : 1;
}
