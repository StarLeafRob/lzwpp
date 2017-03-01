/*
 *      Author: rdt
 */

#include "src/decompress.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

using namespace std;

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
