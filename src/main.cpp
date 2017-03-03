/*
 *      Author: rdt
 */

#include "src/impl.hpp"
#include <fstream>
#include <iostream>
#include <exception>

using namespace std;
using namespace lzw;

int main (int argc, char* argv[]) {
    std::istream* in;
    std::ostream* out;
    std::fstream fin;
    std::fstream fout;
    if (argc == 1) {
        in = &std::cin;
        out = &std::cout;
    }
    else if (argc == 3) {
        fin.open(argv[1], ios_base::binary | ios_base::in);
        if (not fin)
            std::cerr << "invalid input filename " << argv[1] << std::endl;
        fout.open(argv[2], ios_base::binary | ios_base::out);
        if (not fout)
            std::cerr << "invalid output filename " << argv[1] << std::endl;
        in = &fin;
        out = &fout;
    }
    else {
        std::cerr << "--------help---------" << std::endl
                  << "./program <> to use stdin and stdout" << std::endl
                  << " OR " << std::endl
                  << "./program <input_file> <output_file>" << std::endl;

        return 1;
    }
    try {
        decompress(*in, *out);
    } catch(std::exception& e) {
        std::cerr << "failed due to exception: " << e.what() << std::endl;
    }
    fin.close();
    fout.close();
    return 0;
}
