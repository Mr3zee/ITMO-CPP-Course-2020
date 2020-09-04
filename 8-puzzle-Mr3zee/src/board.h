#pragma once

#include <string>
#include <vector>
#include <random>
#include <unordered_set>

class Board;

class Solver;

namespace std {
    template<>
    struct hash<Board>;

    template<>
    struct hash<std::pair<int, int>> {
        size_t operator()(const std::pair<int, int> &value) const {
            return value.first * 31 + value.second;
        }
    };
}

class Board {
    friend struct std::hash<Board>;

    friend class Solver;

public:
    static Board create_goal(unsigned size);

    Board() :
            size_(0),
            solvable_(true),
            manhattan_(0),
            hamming_(0),
            blank_({0, 0}) {}

    Board(const Board &other) = default;

    Board &operator=(const Board &other) = default;

    Board(unsigned size);

    Board(const std::vector<std::vector<unsigned>> &data);

    Board(std::vector<std::vector<unsigned>> &&data);

    std::size_t size() const;

    bool is_goal() const;

    unsigned hamming() const;

    unsigned manhattan() const;

    unsigned linear_conflict() const;

    bool in_bounds(int x, int y) const;

    std::pair<unsigned, unsigned> get_blank() const;

    unsigned on_place(unsigned x, unsigned y) const;

    bool is_solvable() const;

    std::string to_string() const;

    const std::vector<unsigned> &operator[](std::size_t i) const;

    friend bool operator==(const Board &lhs, const Board &rhs);

    friend bool operator!=(const Board &lhs, const Board &rhs);

    friend bool operator<(const Board &lhs, const Board &rhs);

    friend bool operator>(const Board &lhs, const Board &rhs);

    friend std::ostream &operator<<(std::ostream &out, const Board &board) { return out << board.to_string(); }

    void swap_blank(unsigned x, unsigned y);

private:
    struct in_lc_with {
        std::unordered_set<std::pair<int, int>> in_row;
        std::unordered_set<std::pair<int, int>> in_column;

        in_lc_with() : in_row(std::unordered_set<std::pair<int, int>>()),
                       in_column(std::unordered_set<std::pair<int, int>>()) {}

        void insert_row(int i, int j) {
            in_row.insert({i, j});
        }

        void insert_column(int i, int j) {
            in_column.insert({i, j});
        }
    };

    unsigned size_;
    std::vector<std::vector<unsigned>> data_;
    bool solvable_;
    unsigned manhattan_;
    unsigned hamming_;
    unsigned linear_conflict_;
    std::pair<unsigned, unsigned> blank_;
    std::vector<std::vector<std::pair<int, int>>> tiles_goals;
    std::vector<std::vector<in_lc_with>> tiles_lc;

    void set_goals();

    void set_goal(unsigned i, unsigned j);

    void set_fields(std::vector<unsigned> &permutation);

    unsigned actual_value(unsigned x, unsigned y) const;

    void set_hamming_field(unsigned x);

    unsigned calc_hamming() const;

    void set_manhattan_field(unsigned x);

    unsigned calc_manhattan() const;

    unsigned manhattan(unsigned x, unsigned y) const;

    void set_linear_conflict_field(unsigned x);

    void setup_linear_conflict();

    unsigned count_set_lc(unsigned x);

    unsigned check_lc(unsigned x, unsigned y, unsigned i, unsigned j);

    void set_defaults_from_data();
};

namespace std {
    template<>
    struct hash<Board> {
        size_t operator()(const Board &value) const {
            size_t seed = value.size();
            for (unsigned i = 0; i < value.data_.size(); ++i) {
                for (unsigned j = 0; j < value.data_.size(); ++j) {
                    seed = ((61 * i + 127 * j + 31 + seed) * value.data_[i][j] + 631) % 112909;
                }
            }
            return seed;
        }
    };
}