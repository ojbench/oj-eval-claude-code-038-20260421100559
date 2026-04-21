#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include <climits>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

namespace sjtu {

template <typename T>
class vector {
public:
    class iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

    private:
        pointer ptr;
        vector* parent;

    public:
        iterator(pointer p = nullptr, vector* par = nullptr)
            : ptr(p), parent(par) {}

        iterator operator+(const int& n) const { return iterator(ptr + n, parent); }
        iterator operator-(const int& n) const { return iterator(ptr - n, parent); }
        int operator-(const iterator& rhs) const {
            if (parent != rhs.parent) throw std::invalid_argument("Iterators from different containers");
            return ptr - rhs.ptr;
        }
        iterator& operator+=(const int& n) { ptr += n; return *this; }
        iterator& operator-=(const int& n) { ptr -= n; return *this; }
        iterator operator++(int) { iterator tmp = *this; ptr++; return tmp; }
        iterator& operator++() { ptr++; return *this; }
        iterator operator--(int) { iterator tmp = *this; ptr--; return tmp; }
        iterator& operator--() { ptr--; return *this; }
        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }
        bool operator==(const iterator& rhs) const { return ptr == rhs.ptr; }
        bool operator!=(const iterator& rhs) const { return ptr != rhs.ptr; }
    };

    class const_iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;

    private:
        pointer ptr;
        const vector* parent;

    public:
        const_iterator(pointer p = nullptr, const vector* par = nullptr)
            : ptr(p), parent(par) {}

        const_iterator operator+(const int& n) const { return const_iterator(ptr + n, parent); }
        const_iterator operator-(const int& n) const { return const_iterator(ptr - n, parent); }
        int operator-(const const_iterator& rhs) const {
            if (parent != rhs.parent) throw std::invalid_argument("Iterators from different containers");
            return ptr - rhs.ptr;
        }
        const_iterator& operator+=(const int& n) { ptr += n; return *this; }
        const_iterator& operator-=(const int& n) { ptr -= n; return *this; }
        const_iterator operator++(int) { const_iterator tmp = *this; ptr++; return tmp; }
        const_iterator& operator++() { ptr++; return *this; }
        const_iterator operator--(int) { const_iterator tmp = *this; ptr--; return tmp; }
        const_iterator& operator--() { ptr--; return *this; }
        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }
        bool operator==(const const_iterator& rhs) const { return ptr == rhs.ptr; }
        bool operator!=(const const_iterator& rhs) const { return ptr != rhs.ptr; }
    };

private:
    T* data;
    size_t _size;
    size_t _capacity;
    std::allocator<T> alloc;

    void double_capacity() {
        size_t new_capacity = (_capacity == 0) ? 1 : (_capacity * 2);
        T* new_data = alloc.allocate(new_capacity);
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (data) {
                std::memcpy(new_data, data, _size * sizeof(T));
                alloc.deallocate(data, _capacity);
            }
        } else {
            for (size_t i = 0; i < _size; ++i) {
                std::allocator_traits<std::allocator<T>>::construct(alloc, new_data + i, std::move_if_noexcept(data[i]));
            }
            if (data) {
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    for (size_t i = 0; i < _size; ++i) {
                        std::allocator_traits<std::allocator<T>>::destroy(alloc, data + i);
                    }
                }
                alloc.deallocate(data, _capacity);
            }
        }
        data = new_data;
        _capacity = new_capacity;
    }

    void shrink_to_fit() {
        if (_size == _capacity) return;
        if (_size == 0) {
            if (data) alloc.deallocate(data, _capacity);
            data = nullptr;
            _capacity = 0;
            return;
        }
        T* new_data = alloc.allocate(_size);
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(new_data, data, _size * sizeof(T));
        } else {
            for (size_t i = 0; i < _size; ++i) {
                std::allocator_traits<std::allocator<T>>::construct(alloc, new_data + i, std::move_if_noexcept(data[i]));
            }
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (size_t i = 0; i < _size; ++i) {
                    std::allocator_traits<std::allocator<T>>::destroy(alloc, data + i);
                }
            }
        }
        alloc.deallocate(data, _capacity);
        data = new_data;
        _capacity = _size;
    }

    void reserve(size_t new_capacity) {
        if (new_capacity <= _capacity) return;
        T* new_data = alloc.allocate(new_capacity);
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (data) {
                std::memcpy(new_data, data, _size * sizeof(T));
                alloc.deallocate(data, _capacity);
            }
        } else {
            for (size_t i = 0; i < _size; ++i) {
                std::allocator_traits<std::allocator<T>>::construct(alloc, new_data + i, std::move_if_noexcept(data[i]));
            }
            if (data) {
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    for (size_t i = 0; i < _size; ++i) {
                        std::allocator_traits<std::allocator<T>>::destroy(alloc, data + i);
                    }
                }
                alloc.deallocate(data, _capacity);
            }
        }
        data = new_data;
        _capacity = new_capacity;
    }

public:
    vector() : data(nullptr), _size(0), _capacity(0) {}
    vector(const vector& other) : _size(other._size), _capacity(other._size) {
        if (_capacity > 0) {
            data = alloc.allocate(_capacity);
            for (size_t i = 0; i < _size; ++i) {
                std::allocator_traits<std::allocator<T>>::construct(alloc, data + i, other.data[i]);
            }
        } else {
            data = nullptr;
        }
    }
    vector(vector&& other) noexcept : data(other.data), _size(other._size), _capacity(other._capacity) {
        other.data = nullptr;
        other._size = 0;
        other._capacity = 0;
    }
    ~vector() {
        clear();
        if (data) alloc.deallocate(data, _capacity);
    }
    vector& operator=(const vector& other) {
        if (this == &other) return *this;
        clear();
        if (data) alloc.deallocate(data, _capacity);
        _size = other._size;
        _capacity = other._size;
        if (_capacity > 0) {
            data = alloc.allocate(_capacity);
            for (size_t i = 0; i < _size; ++i) {
                std::allocator_traits<std::allocator<T>>::construct(alloc, data + i, other.data[i]);
            }
        } else {
            data = nullptr;
        }
        return *this;
    }
    vector& operator=(vector&& other) noexcept {
        if (this == &other) return *this;
        clear();
        if (data) alloc.deallocate(data, _capacity);
        data = other.data;
        _size = other._size;
        _capacity = other._capacity;
        other.data = nullptr;
        other._size = 0;
        other._capacity = 0;
        return *this;
    }

    T& at(const size_t& pos) {
        if (pos >= _size) throw std::out_of_range("vector::at");
        return data[pos];
    }
    const T& at(const size_t& pos) const {
        if (pos >= _size) throw std::out_of_range("vector::at");
        return data[pos];
    }
    T& operator[](const size_t& pos) { return data[pos]; }
    const T& operator[](const size_t& pos) const { return data[pos]; }
    const T& front() const {
        if (_size == 0) throw std::out_of_range("vector::front");
        return data[0];
    }
    const T& back() const {
        if (_size == 0) throw std::out_of_range("vector::back");
        return data[_size - 1];
    }
    iterator begin() { return iterator(data, this); }
    const_iterator cbegin() const { return const_iterator(data, this); }
    iterator end() { return iterator(data + _size, this); }
    const_iterator cend() const { return const_iterator(data + _size, this); }
    bool empty() const { return _size == 0; }
    size_t size() const { return _size; }
    void clear() {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < _size; ++i) {
                std::allocator_traits<std::allocator<T>>::destroy(alloc, data + i);
            }
        }
        _size = 0;
    }
    iterator insert(iterator pos, const T& value) {
        size_t index = pos - begin();
        if (_size == _capacity) {
            size_t new_capacity = (_capacity == 0) ? 1 : (_capacity * 2);
            reserve(new_capacity);
        }
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (_size > index) {
                std::memmove(data + index + 1, data + index, (_size - index) * sizeof(T));
            }
            std::allocator_traits<std::allocator<T>>::construct(alloc, data + index, value);
        } else {
            if (_size > index) {
                std::allocator_traits<std::allocator<T>>::construct(alloc, data + _size, std::move_if_noexcept(data[_size - 1]));
                for (size_t i = _size - 1; i > index; --i) {
                    data[i] = std::move_if_noexcept(data[i - 1]);
                }
                data[index] = value;
            } else {
                std::allocator_traits<std::allocator<T>>::construct(alloc, data + index, value);
            }
        }
        _size++;
        return begin() + index;
    }
    iterator insert(const size_t& ind, const T& value) {
        if (ind > _size) throw std::out_of_range("vector::insert");
        if (_size == _capacity) {
            size_t new_capacity = (_capacity == 0) ? 1 : (_capacity * 2);
            reserve(new_capacity);
        }
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (_size > ind) {
                std::memmove(data + ind + 1, data + ind, (_size - ind) * sizeof(T));
            }
            std::allocator_traits<std::allocator<T>>::construct(alloc, data + ind, value);
        } else {
            if (_size > ind) {
                std::allocator_traits<std::allocator<T>>::construct(alloc, data + _size, std::move_if_noexcept(data[_size - 1]));
                for (size_t i = _size - 1; i > ind; --i) {
                    data[i] = std::move_if_noexcept(data[i - 1]);
                }
                data[ind] = value;
            } else {
                std::allocator_traits<std::allocator<T>>::construct(alloc, data + ind, value);
            }
        }
        _size++;
        return begin() + ind;
    }
    iterator erase(iterator pos) {
        size_t index = pos - begin();
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (_size > index + 1) {
                std::memmove(data + index, data + index + 1, (_size - index - 1) * sizeof(T));
            }
        } else {
            for (size_t i = index; i < _size - 1; ++i) {
                data[i] = std::move_if_noexcept(data[i + 1]);
            }
            std::allocator_traits<std::allocator<T>>::destroy(alloc, data + _size - 1);
        }
        _size--;
        return begin() + index;
    }
    iterator erase(const size_t& ind) {
        if (ind >= _size) throw std::out_of_range("vector::erase");
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (_size > ind + 1) {
                std::memmove(data + ind, data + ind + 1, (_size - ind - 1) * sizeof(T));
            }
        } else {
            for (size_t i = ind; i < _size - 1; ++i) {
                data[i] = std::move_if_noexcept(data[i + 1]);
            }
            std::allocator_traits<std::allocator<T>>::destroy(alloc, data + _size - 1);
        }
        _size--;
        return begin() + ind;
    }
    void push_back(const T& value) {
        if (_size == _capacity) {
            size_t new_capacity = (_capacity == 0) ? 1 : (_capacity * 2);
            reserve(new_capacity);
        }
        std::allocator_traits<std::allocator<T>>::construct(alloc, data + _size, value);
        _size++;
    }
    void push_back(T&& value) {
        if (_size == _capacity) {
            size_t new_capacity = (_capacity == 0) ? 1 : (_capacity * 2);
            reserve(new_capacity);
        }
        std::allocator_traits<std::allocator<T>>::construct(alloc, data + _size, std::move(value));
        _size++;
    }
    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (_size == _capacity) {
            size_t new_capacity = (_capacity == 0) ? 1 : (_capacity * 2);
            reserve(new_capacity);
        }
        T* ptr = data + _size;
        std::allocator_traits<std::allocator<T>>::construct(alloc, ptr, std::forward<Args>(args)...);
        _size++;
        return *ptr;
    }
    void pop_back() {
        if (_size == 0) throw std::out_of_range("vector::pop_back");
        _size--;
        if constexpr (!std::is_trivially_destructible_v<T>) {
            std::allocator_traits<std::allocator<T>>::destroy(alloc, data + _size);
        }
    }
};

} // namespace sjtu

#endif
