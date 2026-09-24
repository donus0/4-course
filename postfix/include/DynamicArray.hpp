#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

// Динамический массив (аналог std::vector, реализованный вручную), хранящий
// элементы отсортированными по возрастанию. Используется как хранилище
// операндов (переменных/чисел длиной более одного символа), встречающихся
// во входном инфиксном выражении, и как основа для дихотомического поиска.
template <typename T>
class DynamicArray {
private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    void ensureCapacity(std::size_t minCapacity) {
        if (capacity_ >= minCapacity) {
            return;
        }
        std::size_t newCapacity = capacity_ == 0 ? 4 : capacity_ * 2;
        while (newCapacity < minCapacity) {
            newCapacity *= 2;
        }
        T* newData = new T[newCapacity];
        for (std::size_t i = 0; i < size_; ++i) {
            newData[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }

public:
    DynamicArray() : data_(nullptr), size_(0), capacity_(0) {}

    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    ~DynamicArray() {
        delete[] data_;
    }

    void pushBack(const T& value) {
        ensureCapacity(size_ + 1);
        data_[size_++] = value;
    }

    T& operator[](std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("DynamicArray::operator[]: индекс вне диапазона");
        }
        return data_[index];
    }

    const T& operator[](std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("DynamicArray::operator[]: индекс вне диапазона");
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
    long binarySearch(const T& value) const {
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
    bool insertSorted(const T& value) {
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

        ensureCapacity(size_ + 1);
        for (std::size_t i = size_; i > lo; --i) {
            data_[i] = std::move(data_[i - 1]);
        }
        data_[lo] = value;
        ++size_;
        return true;
    }
};
