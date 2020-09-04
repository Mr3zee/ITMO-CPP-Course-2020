#include "randomized_queue.h"

std::mt19937_64 detail::get_random_generator() {
    static std::random_device random_device;
    return std::mt19937_64(random_device());
}

int detail::generate_index(int n, std::mt19937_64 &generator) {
    auto distribution = std::uniform_int_distribution<>(0, n - 1);
    int rev = distribution(generator);
    return rev;
}

std::vector<size_t> detail::generate_random_permutation(size_t size, std::mt19937_64 &generator) {
    std::vector<size_t> vector(size);
    for (size_t i = 0; i < vector.size(); ++i) {
        vector[i] = i;
    }
    std::shuffle(vector.begin(), vector.end() - 1, generator);
    return vector;
}