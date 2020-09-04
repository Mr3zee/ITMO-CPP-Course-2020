#pragma once

#include <unordered_map>
#include "board.h"

using Node = std::pair<unsigned, Board>;
template<class Second>
using u_map = std::unordered_map<Node, Second>;
using v_board = std::vector<Board>;

class Solver
{
public:
    explicit Solver(const Board & board);

    Solver(const Solver & other) = default;

    Solver & operator = (const Solver & other) = default;

    std::size_t moves() const;

    auto begin() const
    { return m_moves.begin(); }

    auto end() const
    { return m_moves.end(); }

private:
    std::vector<Board> m_moves;

    static const std::vector<std::pair<int, int>> deltas;

    static int heuristic(const Board& board);

    static void neighbors(const Board& node, std::vector<Node> &retval);

    static v_board solve(const Board& board);

    static Node find_path(const Node& start, u_map<Node> &came_from);

    static v_board restore_path(const Node &node, Node& end, u_map<Node> &came_from);
};

namespace std {
    template <>
    struct hash<Node> {
        size_t operator ()(const Node& value) const {
            return std::hash<Board>()(value.second);
        }
    };
}
