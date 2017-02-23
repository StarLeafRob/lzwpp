/*
 *      Author: rdt
 */

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <string.h>
#include <unistd.h>
#include "src/iostream_impl.hpp"

using namespace std;
using namespace lzw::iostream;

using chrono::duration_cast;
using chrono::milliseconds;

const char* test_path = "test_data/";
constexpr size_t bufsz = 8096;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "./iostream_test <test_data_path>" << endl;
        return 1;
    }

    std::vector<std::string> test_data = {
        "compressed4", "compressed3", "compressed2", "compressed1"
    };

    std::string test_path(argv[1]);
    if (test_path.back() != '/')
        test_path.append("/");

    for (const auto& test : test_data) {
        const std::string input = test_path + test;
        const std::string output = test_path + "test-de" + test;
        auto t_start = chrono::steady_clock::now();
        decompress(input.c_str(), output.c_str());
        auto t_end = chrono::steady_clock::now();
        cout << test << " took " << duration_cast<milliseconds>(t_end - t_start).count() << " ms" << endl;
        const std::string reference_file = test_path + "de" + test;

        fstream result(output, ios_base::binary | ios_base::in | ios_base::ate);
        fstream reference(reference_file, ios_base::binary | ios_base::in | ios_base::ate);

        if(not result or not reference) {
            cout << "cant open result or reference file" << endl;
            return 1;
        }
        bool match = result.tellg() == reference.tellg();
        result.seekg(0);
        reference.seekg(0);
        if (match) {
            char buf1[bufsz];
            char buf2[bufsz];
            memset(buf1, 0, bufsz);
            memset(buf2, 0, bufsz);
            do {
                result.read(buf1, bufsz);
                reference.read(buf2, bufsz);
                if (memcmp(buf1, buf2, bufsz)) {
                    match = false;
                    break;
                }
            } while(result and reference);
        }
        std::remove(output.c_str());
        if (not match) {
            cout << "results don't match for " << test << endl;
            return 1;
        }
    }
}
