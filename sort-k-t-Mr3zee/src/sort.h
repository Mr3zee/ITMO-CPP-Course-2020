#pragma once
#include <istream>
#include <ostream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>

using ll = long long;
using pss = std::pair<size_t, size_t>;

struct parameters {
    ll key_field1 = -1;
    ll key_field2 = -1;
    std::string column_separator = "\n\t\r ";
};

struct sort_block {
    std::string origin_string;
    // first sorting index
    size_t fsi = std::string::npos;
    // number of characters
    size_t num = std::string::npos;
};

pss find_part_of(const std::string& line, size_t pos, size_t count, const std::string& separator) {
    size_t first = pos, last = pos;
    for (size_t i = 0; i < count; ++i) {
        first = line.find_first_not_of(separator, last);
        last = line.find_first_of(separator, first);
    }
    return {(first == std::string::npos ? line.length() : first),
            (last == std::string::npos ? line.length() : last)};
}

sort_block split(const std::string& line, const parameters& args) {
    if (args.key_field1 < 1) return {line, 0, line.length()};
    pss retval = find_part_of(line, 0, args.key_field1, args.column_separator);
    if (args.key_field2 == -1) {
        return {line, retval.first, retval.second - retval.first};
    }
    size_t first = retval.first;
    retval = find_part_of(line, retval.second, args.key_field2 - args.key_field1, args.column_separator);
    return {line, first, retval.second - first};
}

bool compare(const sort_block& block1, const sort_block& block2) {
    return block1.origin_string.compare(block1.fsi, block1.num, block2.origin_string, block2.fsi, block2.num) < 0;
}

void sort(std::istream & in, const parameters& args) {
    std::vector<sort_block> lines;
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(split(line, args));
    }
    std::sort(lines.begin(), lines.end(), compare);
    for (auto & str : lines) {
        std::cout << str.origin_string << '\n';
    }
}
