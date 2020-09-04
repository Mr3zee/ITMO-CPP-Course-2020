#include <iostream>
#include <fstream>
#include "sort.h"

void sort_console(const parameters& input) {
    sort(std::cin, input);
}

void sort_file(const std::string& file, const parameters& input) {
    std::ifstream fin;
    fin.open(file);
    sort(fin, input);
    fin.close();
}

int main(int argc, char ** argv)
{
    parameters input;
    if (argc < 3) {
        if (argc == 1) sort_console(input); else sort_file(argv[1], input);
        return 0;
    }
    std::string file = argv[1];
    int i = (file == "-k" || file == "--key") ? 1 : 2;
    input.key_field1 = std::strtoul(argv[i + 1], nullptr, 10);
    i += 2;
    if (i < argc) {
        if (argv[i][0] != '-') {
            input.key_field2 = std::strtoul(argv[i++], nullptr, 10);
        }
        input.column_separator = i < argc ? argv[i + 1] : "\n\t\r ";
    }
    if (file[0] == '-') sort_console(input); else sort_file(file, input);
    return 0;
}