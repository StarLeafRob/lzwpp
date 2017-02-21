/*
 *      Author: rdt
 */

#pragma once
#include <string>
#include <iosfwd>

namespace lzw {

    enum class Result: uint8_t {
        SUCCESS,
        IO_ERROR,
        CORRUPT_INPUT
    };

    Result decompress (const char* input, const char* output);

}
