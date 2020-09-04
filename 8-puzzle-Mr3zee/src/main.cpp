#include "solver.h"

#include <iostream>
#include <chrono>

using time_ = std::chrono::steady_clock;

int main()
{
    int res = 0;
    int dimension = 6;
    int num = 1;
    for (int i = 0; i < num; ++i) {
        Board board(dimension);
        if (!board.is_solvable()) {
            i--;
            continue;
        }
        auto a = time_::now().time_since_epoch();
        Solver solver(board);
        auto b = time_::now().time_since_epoch();
        res += std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
    }
    std::cout << "For " << num << ' ' << dimension << "x" << dimension << " board(s) ";
    std::cout << "solved in " << res << " ms";
}
