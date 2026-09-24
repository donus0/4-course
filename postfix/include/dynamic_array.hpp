#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

// Динамический массив (аналог std::vector, реализованный вручную), хранящий
// элементы отсортированными по возрастанию. Используется как хранилище
// операндов (переменных/чисел длиной более одного символа), встречающихся
// во входном инфиксном выражении, и как основа для дихотомического поиска.
template <typename T>
class dynamic_array {
private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    void ensure_capacity(std::size_t min_capacity) {
        if (capacity_ >= min_capacity) {
            return;
        }
        std::size_t new_capacity = capacity_ == 0 ? 4 : capacity_ * 2;
        while (new_capacity < min_capacity) {
            new_capacity *= 2;
        }
        T* new_data = new T[new_capacity];
        for (std::size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }

public:
    dynamic_array() : data_(nullptr), size_(0), capacity_(0) {}

    dynamic_array(const dynamic_array&) = delete;
    dynamic_array& operator=(const dynamic_array&) = delete;

    ~dynamic_array() {
        delete[] data_;
    }

    void push_back(const T& value) {
        ensure_capacity(size_ + 1);
        data_[size_++] = value;
    }

    T& operator[](std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("dynamic_array::operator[]: индекс вне диапазона");
        }
        return data_[index];
    }

    const T& operator[](std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("dynamic_array::operator[]: индекс вне диапазона");
        }
        return data_[index];
    }

    std::size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    // Дихотомический (бинарный) поиск значения в отсортированном массиве.
    // Возвращает индекс элемента или -1, если значение не найдено.
    long binary_search(const T& value) const {
        long lo = 0;
        long hi = static_cast<long>(size_) - 1;

        while (lo <= hi) {
            long mid = lo + (hi - lo) / 2;
            if (data_[mid] < value) {
                lo = mid + 1;
            } else if (value < data_[mid]) {
                hi = mid - 1;
            } else {
                return mid;
            }
        }

        return -1;
    }

    // Вставляет значение, сохраняя массив отсортированным (позиция ищется
    // тем же дихотомическим способом). Если значение уже есть в массиве,
    // ничего не делает и возвращает false.
    bool insert_sorted(const T& value) {
        std::size_t lo = 0;
        std::size_t hi = size_;

        while (lo < hi) {
            std::size_t mid = lo + (hi - lo) / 2;
            if (data_[mid] < value) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }

        if (lo < size_ && !(data_[lo] < value) && !(value < data_[lo])) {
            return false;
        }

        ensure_capacity(size_ + 1);
        for (std::size_t i = size_; i > lo; --i) {
            data_[i] = std::move(data_[i - 1]);
        }
        data_[lo] = value;
        ++size_;
        return true;
    }
};
