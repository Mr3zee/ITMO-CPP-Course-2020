#include "solver.h"
#include <queue>
#include <algorithm>
#include <map>
#include <set>

Solver::Solver(const Board & board) {
    m_moves = solve(board);
}

std::size_t Solver::moves() const {
    return m_moves.empty() ? 0 : m_moves.size() - 1;
}

const std::vector<std::pair<int, int>> Solver::deltas = {
        {-1, 0}, {0, -1}, {1, 0}, {0, 1}
};

int Solver::heuristic(const Board& board) {
    return board.manhattan() + board.linear_conflict();
}

void Solver::neighbors(const Board& node, std::vector<Node> &retval) {
    retval.clear();
    for (auto [dx, dy] : Solver::deltas) {
        Board new_board = node;
        int new_x = new_board.get_blank().first + dx;
        int new_y = new_board.get_blank().second + dy;
        if (new_board.in_bounds(new_x, new_y)) {
            new_board.swap_blank(new_x, new_y);
            retval.emplace_back(0, new_board);
        }
    }
}

v_board Solver::solve(const Board& board) {
    if (board.size() == 1 || board.size() == 0) return v_board(1, board);
    if (!board.is_solvable()) return v_board();
    Node start = {0, board};
    u_map<Node> came_from;
    Node end = find_path(start, came_from);
    return restore_path(start, end, came_from);
}

Node Solver::find_path(const Node &start, u_map<Node> &came_from) {
    std::set<Node> open_list;
    open_list.insert(start);
    u_map<int> close_list;
    close_list[start] = 1;
    came_from[start] = start;
    std::vector<Node> neighbors;
    Node current;
    while (!open_list.empty()) {
        current = *open_list.begin();
        open_list.erase(open_list.begin());
        if (current.second.is_goal()) {
            return current;
        }
        Solver::neighbors(current.second, neighbors);
        for (auto & next : neighbors) {
            int new_level = close_list[current] + 1;
            int cur_level = close_list[next];
            if (!cur_level || new_level < cur_level) {
                close_list[next] = new_level;
                next.first = new_level + heuristic(next.second);
                open_list.erase(next);
                open_list.insert(next);
                came_from[next] = current;
            }
        }
    }
    return {start};
}

v_board Solver::restore_path(const Node &start, Node& end, u_map<Node> &came_from) {
    v_board retval;
    while (end != start) {
        retval.push_back(end.second);
        end = came_from[end];
    }
    retval.push_back(end.second);
    std::reverse(retval.begin(), retval.end());
    return retval;
}
