#pragma once

#include <iostream>
#include <vector>
#include <iterator>

#include <random>
#include <algorithm>

namespace detail {
    std::mt19937_64 get_random_generator();

    int generate_index(int n, std::mt19937_64 &generator);

    std::vector<size_t> generate_random_permutation(size_t size, std::mt19937_64 &generator);
}

template<class T>
class Iterator {
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = long long;
    using pointer = value_type *;
    using reference = value_type &;
private:
    pointer _ptr;
    std::vector<size_t> _permutation;
    size_t _index;
public:
    explicit Iterator(const pointer ptr, size_t size, size_t index, std::mt19937_64 &generator) :
            _ptr(ptr),
            _permutation(detail::generate_random_permutation(size + 1, generator)),
            _index(index) {}

    pointer current() const {
        return _ptr + _permutation[_index];
    }

    bool operator==(const Iterator &other) const {
        return current() == other.current();
    }

    bool operator!=(const Iterator &other) const {
        return current() != other.current();
    }

    reference operator*() const {
        return *current();
    }

    value_type *operator->() const {
        return current();
    }

    Iterator &operator++() {
        _index++;
        return *this;
    }

    Iterator operator++(int) {
        auto retval = *this;
        _index++;
        return retval;
    }

    Iterator &operator--() {
        _index--;
        return *this;
    }

    Iterator operator--(int) {
        auto retval = *this;
        _index--;
        return retval;
    }
};

template<class T>
class randomized_queue {
private:
    std::vector<T> elements;
    mutable std::mt19937_64 generator;

    // generate random index from [0, size)
    int generate_index() const {
        return detail::generate_index(elements.size(), generator);
    }

public:
    // default constructor
    randomized_queue() {
        elements = std::vector<T>();
        generator = detail::get_random_generator();
    }

    // copy constructor
    randomized_queue(const randomized_queue<T> &other) {
        elements = other.elements;
        generator = detail::get_random_generator();
    }

    // move constructor
    randomized_queue(randomized_queue<T> &&other) noexcept {
        elements = std::move(other.elements);
        generator = detail::get_random_generator();
    }

    // copy operator
    randomized_queue<T> &operator=(const randomized_queue<T> &other) {
        if (this == &other) {
            return *this;
        }
        elements = other.elements;
        generator = detail::get_random_generator();
        return *this;
    }

    // move operator
    randomized_queue<T> &operator=(randomized_queue<T> &&other) noexcept {
        elements = std::move(other.elements);
        generator = detail::get_random_generator();
        return *this;
    }

    // return current queue size
    size_t size() const {
        return elements.size();
    }

    // return true if queue is empty
    bool empty() const {
        return elements.empty();
    }

    // insert x into queue
    void enqueue(const T &x) {
        elements.push_back(x);
    }

    // move x into queue
    void enqueue(T &&x) {
        elements.push_back(std::move(x));
    }

    // return and when delete random element from queue
    T dequeue() {
        size_t index = generate_index();
        T retval = std::move(elements[index]);
        std::swap(elements[index], elements.back());
        elements.pop_back();
        return retval;
    }

    // return random value from queue
    const T &sample() const {
        return elements[generate_index()];
    }

    T &sample() {
        return elements[generate_index()];
    }

//    ITERATOR(iterator, T)
//
//    ITERATOR(const_iterator, const T)

    Iterator<T> begin() {
        return iterator(elements.begin(), elements.size(), 0, generator);
    }

    Iterator<const T> begin() const {
        return const_iterator(elements.begin(), elements.size(), 0, generator);
    }

    Iterator<T> end() {
        return iterator(elements.begin(), elements.size(), elements.size(), generator);
    }

    Iterator<const T> end() const {
        return const_iterator(elements.begin(), elements.size(), elements.size(), generator);
    }

    Iterator<const T> cbegin() const {
        return const_iterator(elements.begin(), elements.size(), 0, generator);
    }

    Iterator<const T> cend() const {
        return const_iterator(elements.begin(), elements.size(), elements.size(), generator);
    }
};
