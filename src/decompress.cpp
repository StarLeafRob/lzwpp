/*
 *      Author: rdt
 */

#include "src/impl.hpp"

namespace lzw {

    void decompress(int input_fd, int output_fd) {
        Decompressor impl(input_fd, output_fd);
        impl.run();
    }

}


