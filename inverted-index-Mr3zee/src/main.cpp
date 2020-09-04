#include "searcher.h"

#include <fstream>
#include <iostream>

int main()
{
    Searcher::Filename singleton_file("a.txt");
    // file: "a_single_word"
    std::ifstream singleton_stream(singleton_file);

    Searcher s;
    s.add_document(singleton_file, singleton_stream);


    auto [begin1, end1] = s.search("a_single_word");
    auto begin2 = begin1;
    std::cout << "true: " << (begin1 != end1) << '\n';
    std::cout << "true: " << (singleton_file == *begin1) << '\n';
    while (begin1 != end1) {
        s.add_document(singleton_file, singleton_stream);
        std::cout << *begin1 << ' ';
        begin1++;
    }
    std::cout << '\n';
    std::cout << "true: " << (1 == std::distance(begin2, end1)) << '\n';
    s.add_document(singleton_file, singleton_stream);
}
