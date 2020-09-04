#include <iostream>
#include <algorithm>
#include "searcher.h"

template <class T>
T intersect(const T &first, const T &second);

bool is_separator(char c);

std::vector<Searcher::Word> parse_document(const std::string &line);

void take_word(std::vector<std::string> &words, const std::string &line, size_t &position);

void Searcher::Index::insert(const Word &word, const Filename &filename, int position) {
    index[word][filename].insert(position);
}

void Searcher::Index::remove(const Filename &filename) {
    for (auto &pair : index) {
        index[pair.first].erase(filename);
    }
    files.erase(filename);
}

void Searcher::Index::find(const Word &word, Info& retval) const {
    if (index.count(word)) {
        retval = index.at(word);
    } else {
        retval = Info();
    }
}

bool Searcher::Index::contains_file(const Filename &filename) const {
    return files.count(filename);
}

std::vector<Searcher::Info> Searcher::Index::find_all(const SeparateQueries &queries) const {
    std::vector<Info> retval;
    for (const auto& query : queries) {
        Info found;
        find(query, found);
        if (!found.empty()) retval.push_back(found);
    }
    return retval;
}

[[maybe_unused]] void Searcher::remove_document(const Filename &filename) {
    index.remove(filename);
}

void Searcher::add_document(const Filename &filename, std::istream &strm) {
    if (index.contains_file(filename)) index.remove(filename);
    int position = 0;
    std::string line;
    while (std::getline(strm, line)) {
        auto words = parse_document(line);
        for (const auto &word : words) {
            index.insert(word, filename, position++);
        }
    }
}

std::vector<Searcher::Word> parse_document(const std::string &line) {
    std::vector<std::string> words;
    size_t position = 0;
    while (position < line.size()) {
        while (position < line.size() && is_separator(line[position])) {position++;}
        if (position >= line.size()) break;
        take_word(words, line, position);
    }
    return words;
}

void take_word(std::vector<std::string> &words, const std::string &line, size_t &position) {
    size_t start = position;
    bool was_char = false;
    while (position < line.size() && !is_separator(line[position])) {
        was_char |= line[position] != '_';
        position++;
    }
    if (was_char) words.push_back(line.substr(start, position - start));
}

std::pair<Searcher::DocIterator, Searcher::DocIterator> Searcher::search(const std::string &query) {
    Query parsed_query = parse_query(query);
    FileSet separate_queries_files = intersection(index.find_all(parsed_query.first));
    if (parsed_query.second.empty()) {
        // separate_queries_files
        std::shared_ptr<FileSet> files = std::make_shared<FileSet>(separate_queries_files);
        return {DocIterator(files, files->begin()), DocIterator(files, files->end())};
    } else {
        ExactQueries exact_queries = parsed_query.second;
        FileSet exact_queries_files = search_phrase(exact_queries[0]);
        for (size_t i = 1; i < exact_queries.size(); ++i) {
            exact_queries_files = intersect<FileSet>(exact_queries_files, search_phrase(exact_queries[i]));
        }
        if (!separate_queries_files.empty()) {
            exact_queries_files = intersect<FileSet>(exact_queries_files, separate_queries_files);
        }
        // exact_queries_files
        std::shared_ptr<FileSet> files = std::make_shared<FileSet>(exact_queries_files);
        return {DocIterator(files, files->begin()), DocIterator(files, files->end())};
    }
}

Searcher::Query Searcher::parse_query(const std::string &line) {
    ExactQueries exact_queries;
    SeparateQueries separate_queries;
    std::vector<Word> lexemes;
    size_t position = 0;
    int count_quotes = 0;
    while (position < line.size()) {
        while (position < line.size() && is_separator(line[position])) {
            if (line[position] == '\"') {
                lexemes.emplace_back("\"");
                count_quotes++;
            }
            position++;
        }
        if (position >= line.size()) break;
        take_word(lexemes, line, position);
    }
    if (lexemes.empty()) throw BadQuery("Empty query", line);
    if (count_quotes % 2) throw BadQuery("Expected closing quote", line);
    size_t i = 0;
    while (i < lexemes.size()) {
        if (lexemes[i] == "\"") {
            i++;
            SeparateQueries inner;
            while (i < lexemes.size() && lexemes[i] != "\"") {
                inner.push_back(lexemes[i++]);
            }
            i++;
            exact_queries.push_back(inner);
            continue;
        }
        separate_queries.push_back(lexemes[i++]);
    }
    return {separate_queries, exact_queries};
}

Searcher::FileSet Searcher::intersection(const std::vector<Info> &files) {
    if (files.empty()) return Searcher::FileSet();
    FileSet retval = to_filename_set(files[0]);
    for (size_t i = 1; i < files.size(); ++i) {
        retval = intersect<FileSet>(retval, to_filename_set(files[i]));
    }
    return retval;
}

template<class T>
T intersect(const T &first, const T &second) {
    T retval;
    std::set_intersection(first.begin(), first.end(),
                          second.begin(), second.end(),
                          std::inserter(retval, retval.begin()));
    return retval;
}

Searcher::FileSet Searcher::to_filename_set(const Searcher::Info &info) {
    FileSet retval;
    for (const auto& item : info) {
        retval.insert(item.first);
    }
    return retval;
}

Searcher::FileSet Searcher::search_phrase(const Searcher::SeparateQueries &query) {
    std::vector<Info> words_info = index.find_all(query);
    if (words_info.size() != query.size()) return Searcher::FileSet();
    FileSet files = intersection(words_info), retval;
    if (files.empty()) return Searcher::FileSet();
    for (const auto& file : files) {
        Entries candidates = increment(words_info[0][file]);
        for (size_t i = 1; i < words_info.size(); ++i) {
            candidates = increment(intersect<Entries>(candidates, words_info[i][file]));
        }
        if (!candidates.empty()) retval.insert(file);
    }
    return retval;
}

Searcher::Entries Searcher::increment(const Entries &entries) {
    Entries retval;
    for (auto entry : entries) {
        retval.insert(entry + 1);
    }
    return retval;
}

bool is_separator(char c) {
    return std::isspace(c) || (std::ispunct(c) && c != '_');
}