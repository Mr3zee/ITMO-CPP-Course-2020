#include <random>
#include <chrono>
#include <algorithm>
#include "board.h"

namespace detail {

    std::mt19937_64 get_random_generator() {
        // TODO поменять генератор
        return std::mt19937_64(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    std::vector<unsigned> generate_random_permutation(size_t size) {
        std::vector<unsigned> vector(size);
        std::iota(vector.begin(), vector.end(), 0);
        std::shuffle(vector.begin(), vector.end(), get_random_generator());
        return vector;
    }

    unsigned permutation_parity(const std::vector<unsigned> &permutation) {
        std::vector<int> visited(permutation.size(), 0);
        int parity = 0;
        for (size_t i = 0; i < permutation.size(); i++) {
            if (!visited[i]) {
                visited[i] = 1;
                size_t j = i;
                do {
                    parity++;
                    visited[j] = 1;
                    j = permutation[j] - 1;
                } while (j != i);
                parity--;
            }
        }
        return parity % 2;
    }
}

Board Board::create_goal(const unsigned size) {
    std::vector<std::vector<unsigned>> data(size, std::vector<unsigned>(size));
    for (unsigned i = 0; i < size; ++i) {
        for (unsigned j = 0; j < size; ++j) {
            data[i][j] = i * size + j + 1;
        }
    }
    data[size - 1][size - 1] = 0;
    Board board(std::move(data));
    return board;
}

Board::Board(unsigned size) {
    size_ = size;
    std::vector<unsigned> permutation(size * size);
    permutation = detail::generate_random_permutation(permutation.size());
    std::vector<std::vector<unsigned>> new_data(size);
    for (unsigned i = 0; i < size_; ++i) {
        std::vector<unsigned> temp(size_);
        for (unsigned j = 0; j < size_; ++j) {
            temp[j] = permutation[size * i + j];
            if (!temp[j]) {
                blank_ = {i, j};
            }
        }
        new_data[i] = temp;
    }
    data_ = new_data;
    set_fields(permutation);
}

Board::Board(const std::vector<std::vector<unsigned>> &data) :
        size_(data.size()),
        data_(data) {
    set_defaults_from_data();
}

Board::Board(std::vector<std::vector<unsigned>> &&data) :
        size_(data.size()),
        data_(std::move(data)) {
    set_defaults_from_data();
}

void Board::set_defaults_from_data() {
    std::vector<unsigned> permutation;
    for (unsigned i = 0; i < size_; ++i) {
        for (unsigned j = 0; j < size_; ++j) {
            if (!data_[i][j]) {
                blank_ = {i, j};
            }
            permutation.push_back(data_[i][j]);
        }
    }
    set_fields(permutation);
}


void Board::set_fields(std::vector<unsigned> &permutation) {
    for (auto &i : permutation) {
        if (!i) {
            i = permutation.size();
            break;
        }
    }
    solvable_ = !size_ || detail::permutation_parity(permutation) == (manhattan(blank_.first, blank_.second) % 2);
    hamming_ = calc_hamming();
    manhattan_ = !size_ ? 0 : calc_manhattan();
    setup_linear_conflict();
}

std::size_t Board::size() const {
    return size_;
}

bool Board::is_goal() const {
    return hamming_ == 0;
}

unsigned Board::calc_hamming() const {
    if (!size_) return 0;
    unsigned retval = 0, count = 1;
    for (unsigned i = 0; i < size_; ++i) {
        for (unsigned j = 0; j < size_; ++j) {
            retval += (count++ != actual_value(i, j));
        }
    }
    return retval;
}

unsigned Board::hamming() const {
    return hamming_;
}

void Board::set_hamming_field(unsigned x) {
    hamming_ = x;
}

unsigned Board::calc_manhattan() const {
    if (!size_) return 0;
    unsigned retval = 0;
    for (unsigned i = 0; i < size_; ++i) {
        for (unsigned j = 0; j < size_; ++j) {
            retval += manhattan(i, j);
        }
    }
    return retval - manhattan(blank_.first, blank_.second);
}

unsigned Board::manhattan(unsigned x, unsigned y) const {
    unsigned d = actual_value(x, y);
    unsigned dx = abs(x - (d - 1) / size_);
    unsigned dy = abs(y - (d - 1) % size_);
    return dx + dy;
}

unsigned Board::manhattan() const {
    return manhattan_;
}

void Board::set_manhattan_field(unsigned x) {
    manhattan_ = x;
}

unsigned Board::count_set_lc(unsigned x) {
    unsigned total = 0;
    for (size_t i = 0; i < size_; ++i) {
        for (size_t j = i + 1; j < size_; ++j) {
            total += check_lc(x, x, i, j) + check_lc(i, j, x, x);
        }
    }
    return total * 2;
}

unsigned Board::check_lc(unsigned int x, unsigned int y, unsigned int i, unsigned int j) {
    int first = tiles_goals[x][i].first;
    int second = tiles_goals[y][j].first;
    if (first > second && first != -1 && second != -1) {
        tiles_lc[x][i].insert_row(x, j);
        tiles_lc[y][j].insert_row(y, i);
        return 1;
    }
    return 0;
}

void Board::setup_linear_conflict() {
    set_goals();
    tiles_lc = std::vector(size_, std::vector(size_, in_lc_with()));
    if (!size_) return;
    unsigned total = 0;
    for (size_t i = 0; i < size_; ++i) {
        total += count_set_lc(i);
    }
    set_linear_conflict_field(total);
}

void Board::set_linear_conflict_field(unsigned int x) {
    linear_conflict_ = x;
}

unsigned Board::linear_conflict() const {
    return linear_conflict_;
}

void Board::set_goals() {
    tiles_goals = std::vector(size_, std::vector<std::pair<int, int>>(size_, {-1, -1}));
    for (size_t i = 0; i < size_; ++i) {
        for (size_t j = 0; j < size_; ++j) {
            set_goal(i, j);
        }
    }
}

void Board::set_goal(unsigned i, unsigned j) {
    if (i == blank_.first && j == blank_.second) return;
    unsigned value = actual_value(i, j);
    unsigned dest_x = (value - 1) / size_;
    unsigned dest_y = (value - 1) % size_;
    if (dest_x == i) {
        tiles_goals[i][j].first = dest_y;
    }
    if (dest_y == j) {
        tiles_goals[i][j].second = dest_x;
    }
}

bool Board::in_bounds(int x, int y) const {
    int s = static_cast<int>(size_);
    return (0 <= x && x < s) && (0 <= y && y < s);
}

void Board::swap_blank(unsigned x, unsigned y) {
    unsigned manhattan_x1 = manhattan(x, y);
    unsigned new_x = blank_.first;
    unsigned new_y = blank_.second;
    unsigned hamming_x1 = on_place(x, y);
    unsigned hamming_x2 = on_place(new_x, new_y);
    std::swap(data_[x][y], data_[blank_.first][blank_.second]);
    blank_.first = x;
    blank_.second = y;
    set_manhattan_field(manhattan() - manhattan_x1 + manhattan(new_x, new_y));
    set_hamming_field(
            hamming()
            + (hamming_x1 ? 1 : on_place(new_x, new_y) ? -1 : 0)
            + (hamming_x2 ? 1 : on_place(x, y) ? -1 : 0)
    );
    unsigned deleted = 0;
    unsigned added = 0;
    if (new_x == x) {
        if (tiles_goals[x][y].second != -1) {
            deleted = tiles_lc[x][y].in_column.size();
            for (size_t i = 0; i < size_; ++i) {
                tiles_lc[x][y].in_column.erase({i, y});
                tiles_lc[i][y].in_column.erase({x, y});
            }
            deleted -= tiles_lc[x][y].in_column.size();
            tiles_goals[x][y] = {-1, -1};
            set_goal(x, new_y);
        } else {
            tiles_goals[x][y] = {-1, -1};
            set_goal(x, new_y);
            auto shifted = tiles_goals[x][new_y].second;
            for (size_t i = 0; i < x; ++i) {
                auto enemy = tiles_goals[i][new_y].second;
                if (enemy > shifted && enemy != -1 && shifted != -1) {
                    tiles_lc[x][new_y].in_column.insert({i, new_y});
                    tiles_lc[i][new_y].in_column.insert({x, new_y});
                    added += 2;
                }
            }
            for (size_t i = x + 1; i < size_; ++i) {
                auto enemy = tiles_goals[i][new_y].second;
                if (enemy < shifted && enemy != -1 && shifted != -1) {
                    tiles_lc[x][new_y].in_column.insert({i, new_y});
                    tiles_lc[i][new_y].in_column.insert({x, new_y});
                    added += 2;
                }
            }
        }
    } else {
        if (tiles_goals[x][y].first != -1) {
            deleted = tiles_lc[x][y].in_row.size();
            for (size_t i = 0; i < size_; ++i) {
                tiles_lc[x][y].in_row.erase({x, i});
                tiles_lc[x][i].in_row.erase({x, y});
            }
            deleted -= tiles_lc[x][y].in_row.size();
            tiles_goals[x][y] = {-1, -1};
            set_goal(new_x, y);
        } else {
            tiles_goals[x][y] = {-1, -1};
            set_goal(new_x, y);
            auto shifted = tiles_goals[new_x][y].first;
            for (size_t i = 0; i < y; ++i) {
                auto enemy = tiles_goals[new_x][i].first;
                if (enemy > shifted && enemy != -1 && shifted != -1) {
                    tiles_lc[new_x][y].in_row.insert({new_x, i});
                    tiles_lc[new_x][i].in_row.insert({new_x, y});
                    added += 2;
                }
            }
            for (size_t i = y + 1; i < size_; ++i) {
                auto enemy = tiles_goals[new_x][i].first;
                if (enemy < shifted && enemy != -1 && shifted != -1) {
                    tiles_lc[new_x][y].in_row.insert({new_x, i});
                    tiles_lc[new_x][i].in_row.insert({new_x, y});
                    added += 2;
                }
            }
        }
    }
    set_linear_conflict_field(linear_conflict() - deleted * 2 + added);
}


std::pair<unsigned, unsigned> Board::get_blank() const {
    return blank_;
}

unsigned Board::on_place(unsigned x, unsigned y) const {
    return actual_value(x, y) == x * size_ + y + 1;
}

bool Board::is_solvable() const {
    return solvable_;
}

unsigned Board::actual_value(unsigned x, unsigned y) const {
    return !data_[x][y] ? size_ * size_ : data_[x][y];
}

std::string Board::to_string() const {
    std::string str;
    for (unsigned i = 0; i < size_; ++i) {
        for (unsigned j = 0; j < size_; ++j) {
            str += (!data_[i][j] ? " " : std::to_string(data_[i][j])) + ' ';
        }
        str += '\n';
    }
    return str;
}

const std::vector<unsigned> &Board::operator[](const std::size_t i) const {
    return data_[i];
}

bool operator==(const Board &lhs, const Board &rhs) {
    if (lhs.size() != rhs.size()) return false;
    unsigned size = lhs.size();
    for (unsigned i = 0; i < size; ++i) {
        for (unsigned j = 0; j < size; ++j) {
            if (lhs.data_[i][j] != rhs.data_[i][j]) return false;
        }
    }
    return true;
}

bool operator!=(const Board &lhs, const Board &rhs) {
    return !(lhs == rhs);
}

bool operator<(const Board &lhs, const Board &rhs) {
    return lhs.data_ < rhs.data_;
}

bool operator>(const Board &lhs, const Board &rhs) {
    return lhs.data_ > rhs.data_;
}