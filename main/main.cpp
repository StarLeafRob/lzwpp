/*
 *      Author: rdt
 */

#include "lib/lzw.hpp"

int main (int argc, char* argv[]) {
    return lzw::decompress("hello", "world") ? 0 : 1;
}
