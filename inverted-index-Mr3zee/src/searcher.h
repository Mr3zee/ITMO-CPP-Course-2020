#pragma once

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
#include <queue>
#include <exception>
#include <memory>

class Searcher {
public:
    using Word = std::string;
    using Entries = std::set<long long>;
    using Members = std::unordered_set<Word>;
    using Filename = std::string;
    using Info = std::map<Filename, Entries>;
    using SeparateQueries = std::vector<Word>;
    using ExactQueries = std::vector<SeparateQueries>;
    using Query = std::pair<SeparateQueries, ExactQueries>;
    using FileSet = std::set<Filename>;

    Searcher() : index(Index()) {}

    void add_document(const Filename &filename, std::istream &strm);

    [[maybe_unused]] void remove_document(const Filename &filename);

    class BadQuery : public std::exception {
    private:
        std::string m_message;
    public:
        explicit BadQuery(const std::string &message, const std::string &query) :
                m_message("Search query syntax error: " + message + " || Query: " + query) {}

        const char *what() const noexcept override {
            return m_message.c_str();
        }
    };

    struct DocIterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const Filename ;
        using difference_type = long long;
        using pointer = value_type *;
        using reference = value_type &;
    private:
        std::shared_ptr<FileSet> _files;
        FileSet::iterator _ptr;
    public:
        explicit DocIterator(std::shared_ptr<std::set<std::string>> files, const std::set<std::string>::iterator ptr)
                : _files(std::move(files)), _ptr(ptr) {}

        bool operator==(const DocIterator &other) const {
            return _ptr == other._ptr;
        }

        bool operator!=(const DocIterator &other) const {
            return _ptr != other._ptr;
        }

        reference operator*() const {
            return _ptr.operator*();
        }

        pointer operator->() const {
            return _ptr.operator->();
        }

        DocIterator &operator++() {
            _ptr++;
            return *this;
        }

        DocIterator operator++(int) {
            auto retval = *this;
            _ptr++;
            return retval;
        }

        DocIterator &operator--() {
            _ptr--;
            return *this;
        }

        DocIterator operator--(int) {
            auto retval = *this;
            _ptr--;
            return retval;
        }
    };

    std::pair<DocIterator, DocIterator> search(const std::string &query);

    class Index {
    public:
        explicit Index() :
                index(std::unordered_map<Word, Info>()) {}

        void insert(const Word &word, const Filename &filename, int position);

        void remove(const Filename &filename);

        void find(const Word &word, Info& retval) const;

        std::vector<Info> find_all(const SeparateQueries &queries) const;

        bool contains_file(const Filename &filename) const;

    private:
        std::unordered_map<Word, Info> index;

        std::unordered_set<Filename> files;
    };

private:
    Index index;

    static Query parse_query(const std::string &line);

    static FileSet intersection(const std::vector<Info> &files);

    static FileSet to_filename_set(const Info &info);

    FileSet search_phrase(const SeparateQueries &query);

    static Entries increment(const Entries &entries);

};