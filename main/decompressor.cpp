/*
 *      Author: rdt
 */

#include "lib/decompress.hpp"
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;

int main (int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "usage = lzw_decompressor <input> <output>" << std::endl;
        return 1;
    }
    auto result = lzw::decompress(argv[1], argv[2]);
    if (result != lzw::Result::SUCCESS) {
        std::cout << "failed" << std::endl;
        return 1;
    }
    return 0;
}
