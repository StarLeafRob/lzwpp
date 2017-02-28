/*
 *      Author: rdt
 */

#include <iostream>
#include <fstream>
#include <chrono>

#include "src/decompress.hpp"

using namespace std;

int main (int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "usage = lzw_decompressor <input> <output>" << std::endl;
        return 1;
    }
    if (not lzw::iostream::decompress(argv[1], argv[2])) {
        std::cout << "failed" << std::endl;
        return 1;
    }
    return 0;
}
