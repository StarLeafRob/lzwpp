/*
 *      Author: rdt
 */

#include "lib/iostream_impl.hpp"
#include "lib/decoder.hpp"
#include <fstream>
#include <stdint.h>

namespace lzw {

    using namespace std;

    bool decompress_loop(fstream& in, fstream& out) {
        int16_t code1;
        int16_t code2;
        uint8_t a, b, c;
        Decoder<fstream> codec(out);
        while(true) {
            if (not in.get((char&)a))
                return true;
            if (not in.get((char&)b))
                return false;
            if (not in.get((char&)c)) {
                code1 = (a << 8) + b;
                return codec.decode(code1);
            } else {
                code1 = (a << 4) + (b >> 4);
                code2 = ((b & 0xf) << 8) + c;
                if (not codec.decode(code1))
                    return false;
                if (not codec.decode(code2))
                    return false;
            }
        }
    }

    bool decompress (const char* input_p, const char* output_p) {
        fstream input(input_p, ios_base::binary | ios_base::in);
        fstream output(output_p ,ios_base::binary | ios_base::out);
        if (not input or not output)
            throw std::runtime_error("invalid files");
        bool result = true;
        try {
            decompress_loop(input, output);
        }
        catch(std::exception& e) {
            result = false;
        }
        input.close();
        output.close();
        return result;
    }

}


