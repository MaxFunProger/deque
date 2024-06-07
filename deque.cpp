#include <string.h>

#include <deque>
#include <iostream>
#include <string>
#include <vector>

template <typename T>
class Deque {
  public:
    Deque();
    Deque(const Deque& other);
    Deque(const int& size);
    Deque(const int& size, const T& value);

    ~Deque();

    Deque& operator=(const Deque& other);

    T& operator[](size_t pos);
    const T& operator[](size_t pos) const;
    T& at(size_t pos);
    const T& at(size_t pos) const;

    using pointer = T*;
    using reference = T&;

    template <bool Const>
    struct Iterator;

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template <bool Const>
    struct Iterator {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = int;
        using value_type = std::conditional_t<Const, const T, T>;
        using pointer = std::conditional_t<Const, const T*, T*>;
        using reference = std::conditional_t<Const, T const&, T&>;
        using Self = Iterator<Const>;
        Iterator() : chunkP(nullptr), curP(nullptr) {}
        Iterator(const typename std::vector<T*>::iterator& chunkIter,
                 T* eltPointer)
            : chunkP(*chunkIter), curP(eltPointer), map_iterator(chunkIter) {}

        template <bool OtherConst>
            requires(Const || !OtherConst)
        Iterator(const Deque<T>::Iterator<OtherConst>& iter) {
            chunkP = iter.chunkP;
            curP = iter.curP;
            map_iterator = iter.map_iterator;
        }

        ~Iterator() {}

        template <bool OtherConst>
            requires(Const || !OtherConst)
        Self& operator=(const Iterator<OtherConst>& iter) {
            curP = iter.curP;
            chunkP = iter.chunkP;
            map_iterator = iter.map_iterator;
            return *this;
        }

        reference operator*() const {
            return *curP;
        }

        pointer operator->() const {
            return curP;
        }

        Self& operator++() {
            ++curP;
            if (curP == chunkP + chunkSize_) {
                ++map_iterator;
                curP = chunkP = *map_iterator;
            }
            return *this;
        }

        Self operator++(int) {
            Self tmp = *this;
            ++*this;
            return tmp;
        }

        Self& operator--() {
            if (curP == chunkP) {
                --map_iterator;
                chunkP = *map_iterator;
                if (chunkP == nullptr) {
                    curP = nullptr;
                    return *this;
                }
                curP = chunkP + chunkSize_ - 1;
                return *this;
            } else {
                --curP;
            }
            return *this;
        }

        Self operator--(int) {
            Self tmp = *this;
            --*this;
            return tmp;
        }

        Self operator+(int number) const {
            if (number >= 0) {
                if (curP - chunkP + number < chunkSize_) {
                    return Self(map_iterator, curP + number);
                }
                typename std::vector<T*>::iterator returnMapIter =
                    map_iterator + number / chunkSize_;
                if (curP - chunkP + number % chunkSize_ < chunkSize_) {
                    return Self(
                        returnMapIter,
                        *returnMapIter + (curP - chunkP) + number % chunkSize_);
                }
                ++returnMapIter;
                number -= chunkP + chunkSize_ - curP;
                return Self(returnMapIter, *returnMapIter + number);
            } else {
                int positive = -number;
                if (curP == chunkP && chunkP == nullptr) {
                    typename std::vector<T*>::iterator returnMapIter =
                        map_iterator - 1;
                    --positive;
                    returnMapIter -= positive / chunkSize_;
                    return Self(returnMapIter, *returnMapIter + chunkSize_ - 1 -
                                                   positive % chunkSize_);
                }
                if (curP - chunkP >= positive) {
                    return Self(map_iterator, curP - positive);
                }
                positive -= curP - chunkP + 1;
                typename std::vector<T*>::iterator returnMapIter =
                    map_iterator - 1;
                returnMapIter -= positive / chunkSize_;
                return Self(returnMapIter, *returnMapIter + chunkSize_ - 1 -
                                               positive % chunkSize_);
            }
        }

        Self& operator+=(const int& number) {
            *this = *this + number;
            return *this;
        }

        Self operator-(int number) const {
            return *this + (-number);
        }

        int operator-(const const_iterator& iter) const {
            int delta_chunks = (map_iterator - iter.map_iterator) * chunkSize_;
            delta_chunks -= iter.curP - iter.chunkP;
            delta_chunks += curP - chunkP;
            return delta_chunks;
        }

        Self& operator-=(const int& number) {
            *this = *this - number;
            return *this;
        }

        bool operator<(const const_iterator& iter) const {
            if (map_iterator < iter.map_iterator) {
                return true;
            }
            if (map_iterator == iter.map_iterator) {
                return curP < iter.curP;
            }
            return false;
        }

        bool operator<=(const const_iterator& iter) const {
            return (*this < iter || *this == iter);
        }

        bool operator>=(const const_iterator& iter) const {
            return !(*this < iter);
        }

        template <typename IterType>
        bool operator==(const IterType& iter) const {
            return (chunkP == iter.chunkP && curP == iter.curP &&
                    map_iterator == iter.map_iterator);
        }

        bool operator!=(const const_iterator& iter) const {
            return !(chunkP == iter.chunkP && curP == iter.curP &&
                     map_iterator == iter.map_iterator);
        }

        bool operator>(const const_iterator& iter) const {
            return !(*this <= iter);
        }

        T *chunkP, *curP;
        typename std::vector<T*>::iterator map_iterator;
    };

    size_t size() const;

    iterator begin();
    const_iterator begin() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    const_iterator cbegin() const;
    const_reverse_iterator crbegin() const;
    iterator end();
    const_iterator end() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;
    const_iterator cend() const;
    const_reverse_iterator crend() const;

    void insert(const iterator& /*iter*/, const T& /*val*/);
    void erase(const iterator& /*iter*/);

    void push_back(const T& value);
    void push_front(const T& value);
    void pop_back();
    void pop_front();

  private:
    void realloc();
    void alloc(pointer& /*p*/);
    void dealloc(pointer& /*p*/);
    void construct(pointer /*p*/, const T& /*value*/);
    void destroy(pointer /*p*/);
    size_t sz_;
    iterator beginIterator, endIterator;
    std::vector<T*> chunkMap_;
    static constexpr int chunkSize_ = 32;
    int chunkMapSize_;
};

template <typename T>
Deque<T>::Deque() : sz_(0), chunkMapSize_(3) {
    chunkMap_.resize(chunkMapSize_, nullptr);
    try {
        alloc(chunkMap_[1]);
        beginIterator = endIterator =
            iterator(chunkMap_.begin() + 1, chunkMap_[1]);
    } catch (...) {
        chunkMap_.clear();
        throw;
    }
}

template <typename T>
Deque<T>::Deque(const Deque<T>& other)
    : sz_(other.sz_), chunkMapSize_(other.chunkMapSize_) {
    chunkMap_.resize(chunkMapSize_, nullptr);
    int revert_index = -1;
    try {
        for (int it = 0; it < chunkMapSize_; ++it) {
            if (other.chunkMap_[it] != nullptr) {
                alloc(chunkMap_[it]);
                revert_index = it;
            }
        }
    } catch (...) {
        for (int i = 0; i <= revert_index; ++i) {
            if (chunkMap_[i] != nullptr) {
                dealloc(chunkMap_[i]);
            }
        }
        chunkMap_.clear();
        throw;
    }
    iterator revert_iterator;
    try {
        pointer tmp = nullptr;
        const_iterator it;
        for (it = other.cbegin(); it != other.cend(); ++it) {
            construct(chunkMap_[it.map_iterator - other.chunkMap_.begin()] +
                          (it.curP - it.chunkP),
                      *(it.curP));
            tmp = chunkMap_[it.map_iterator - other.chunkMap_.begin()] +
                  (it.curP - it.chunkP);
            revert_iterator = iterator(
                chunkMap_.begin() + (it.map_iterator - other.chunkMap_.begin()),
                tmp);
            if (it == other.begin() && other.begin() != other.end()) {
                beginIterator = revert_iterator;
            }
        }
        if (tmp == nullptr) {
            endIterator = beginIterator;
        } else {
            endIterator = revert_iterator + 1;
        }

    } catch (...) {
        for (iterator it = beginIterator; it != revert_iterator; ++it) {
            destroy(it.curP);
        }
        for (iterator it = beginIterator;
             revert_iterator.chunkP - it.chunkP >= 0; ++it) {
            dealloc(*(it.map_iterator));
            it.chunkP = nullptr;
            it.curP = nullptr;
        }
        chunkMap_.clear();
        throw;
    }
}

template <typename T>
Deque<T>::Deque(const int& size)
    : sz_(size),
      chunkMapSize_(std::max(
          3, 2 + size / chunkSize_ + static_cast<int>(size % chunkSize_))) {
    chunkMap_.resize(chunkMapSize_, nullptr);
    int revert_index = -1;  // last chunk with allocated memory
    try {
        for (int i = 1; i < chunkMapSize_ - 1; ++i) {
            alloc(chunkMap_[i]);
            revert_index = i;
        }
        beginIterator.curP = chunkMap_[1];
        beginIterator.chunkP = chunkMap_[1];
        beginIterator.map_iterator = chunkMap_.begin() + 1;
        endIterator = beginIterator + size;
    } catch (...) {
        for (int i = 1; i < revert_index; ++i) {
            dealloc(chunkMap_[i]);
        }
        chunkMap_.clear();
        throw;
    }

    int revert_index_map = -1;
    int revert_index_bucket = -1;
    try {
        for (int i = 1; i < chunkMapSize_ - 1; ++i) {
            for (int j = 0; j < chunkSize_; ++j) {
                revert_index_bucket = j;
                if (j == 0) {
                    revert_index_map = i;
                }
                new (chunkMap_[i] + j) T();
            }
        }
    } catch (...) {
        for (int i = 1; i <= revert_index_map; ++i) {
            int index =
                (i == revert_index_map ? revert_index_bucket : chunkSize_);
            for (int j = 0; j < index; ++j) {
                destroy(chunkMap_[i] + j);
            }
        }
        for (int i = 1; i <= revert_index; ++i) {
            dealloc(chunkMap_[i]);
        }
        chunkMap_.clear();
        throw;
    }
}

template <typename T>
Deque<T>::Deque(const int& size, const T& value)
    : sz_(size),
      chunkMapSize_(std::max(
          3, 2 + size / chunkSize_ +
                 static_cast<int>(static_cast<bool>(size % chunkSize_)))) {
    chunkMap_.resize(chunkMapSize_, nullptr);
    int revert_index = -1;  // last chunk with allocated memory
    try {
        for (int i = 1; i < chunkMapSize_ - 1; ++i) {
            alloc(chunkMap_[i]);
            revert_index = i;
        }
        beginIterator.curP = chunkMap_[1];
        beginIterator.chunkP = chunkMap_[1];
        beginIterator.map_iterator = chunkMap_.begin() + 1;
        endIterator = beginIterator + size;
    } catch (...) {
        for (int i = 1; i < revert_index; ++i) {
            dealloc(chunkMap_[i]);
        }
        chunkMap_.clear();
        throw;
    }

    int revert_index_map = -1;
    int revert_index_bucket = -1;
    try {
        for (int i = 1; i < chunkMapSize_ - 1; ++i) {
            for (int j = 0; j < chunkSize_; ++j) {
                revert_index_bucket = j;
                if (j == 0) {
                    revert_index_map = i;
                }
                construct(chunkMap_[i] + j, value);
            }
        }
    } catch (...) {
        for (int i = 1; i < revert_index_map; ++i) {
            for (int j = 0; j < revert_index_bucket; ++j) {
                destroy(chunkMap_[i] + j);
            }
        }
        for (int i = 1; i < revert_index; ++i) {
            dealloc(chunkMap_[i]);
        }
        chunkMap_.clear();
        throw;
    }
}

template <typename T>
Deque<T>::~Deque() {
    for (iterator it = begin(); it != end(); ++it) {
        destroy(it.curP);
    }
    for (size_t i = 0; i < chunkMap_.size(); ++i) {
        if (chunkMap_[i] != nullptr) {
            dealloc(chunkMap_[i]);
        }
    }
    chunkMap_.clear();
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& other) {
    Deque<T> tmp = other;
    std::swap(sz_, tmp.sz_);
    std::swap(beginIterator, tmp.beginIterator);
    std::swap(endIterator, tmp.endIterator);
    std::swap(chunkMap_, tmp.chunkMap_);
    std::swap(chunkMapSize_, tmp.chunkMapSize_);
    return *this;
}

template <typename T>
size_t Deque<T>::size() const {
    return sz_;
}

template <typename T>
T& Deque<T>::operator[](size_t pos) {
    return *(beginIterator + pos).curP;
}

template <typename T>
const T& Deque<T>::operator[](size_t pos) const {
    return *(beginIterator + pos);
}

template <typename T>
T& Deque<T>::at(size_t pos) {
    if (pos >= static_cast<size_t>(endIterator - beginIterator)) {
        throw std::out_of_range("error");
    }
    return *(beginIterator + pos);
}

template <typename T>
const T& Deque<T>::at(size_t pos) const {
    if (pos >= endIterator - beginIterator) {
        throw std::out_of_range("error");
    }
    return *(beginIterator + pos);
}

template <typename T>
void Deque<T>::insert(const iterator& iter, const T& val) {
    ++sz_;
    T* revert_curP = endIterator.curP;
    T* revert_chunkP = endIterator.chunkP;
    if (iter == endIterator) {
        if (endIterator.chunkP != nullptr) {
            try {
                construct(endIterator.curP, val);
            } catch (...) {
                --sz_;
                throw;
            }
            ++endIterator;
            return;
        }
        try {
            alloc(*(endIterator.map_iterator));
        } catch (...) {
            --sz_;
            throw;
        }
        endIterator.curP = endIterator.chunkP = *(endIterator.map_iterator);
        try {
            construct(endIterator.chunkP, val);
        } catch (...) {
            --sz_;
            endIterator.curP = revert_curP;
            endIterator.chunkP = revert_chunkP;
            throw;
        }
        ++endIterator;
        if (iter.map_iterator + 1 == chunkMap_.end()) {
            try {
                realloc();
            } catch (...) {
                --sz_;
                --endIterator;
                endIterator.curP = revert_curP;
                endIterator.chunkP = revert_chunkP;
                throw;
            }
        }
        return;
    }
    T buff = *(iter.curP);
    T new_val = val;
    for (iterator it = iter; it != endIterator; ++it) {
        buff = *(it.curP);
        *(it.curP) = new_val;
        new_val = buff;
    }
    bool is_nullptr = false;
    if (endIterator.curP != nullptr) {
        is_nullptr = true;
        *(endIterator.curP) = new_val;
        ++endIterator;
        return;
    }
    unsigned flag = 0;
    try {
        alloc(*(endIterator.map_iterator));
        flag = 1;
        endIterator.curP = endIterator.chunkP = *(endIterator.map_iterator);
        construct(endIterator.curP, new_val);
        flag = 2;
        ++endIterator;
        if (endIterator.map_iterator + 1 == chunkMap_.end()) {
            realloc();
        }
    } catch (...) {
        switch (flag) {
            case 2:
                --endIterator;
                destroy(endIterator.curP);
            case 1:
                endIterator.curP = revert_curP;
                endIterator.chunkP = revert_chunkP;
                dealloc(*(endIterator.map_iterator));
                break;
            default:
                break;
        }
        if (is_nullptr) {
            --endIterator;
            dealloc(endIterator.curP);
        }
        for (iterator it = endIterator - 1; it >= iter; --it) {
            buff = *(endIterator.curP);
            *(endIterator.curP) = new_val;
            new_val = buff;
        }
        throw;
    }
}

template <typename T>
void Deque<T>::push_back(const T& value) {
    ++sz_;
    T* revert_curP = endIterator.curP;
    T* revert_chunkP = endIterator.chunkP;
    if (endIterator.curP != nullptr) {
        try {
            construct(endIterator.curP, value);
        } catch (...) {
            --sz_;
            throw;
        }
        ++endIterator;
        return;
    }
    try {
        alloc(*(endIterator.map_iterator));
    } catch (...) {
        --sz_;
        throw;
    }
    endIterator.curP = endIterator.chunkP = *(endIterator.map_iterator);
    try {
        construct(endIterator.curP, value);
    } catch (...) {
        --sz_;
        endIterator.curP = revert_curP;
        endIterator.chunkP = revert_chunkP;
        throw;
    }
    ++endIterator;
    if (endIterator.map_iterator + 1 == chunkMap_.end()) {
        try {
            realloc();
        } catch (...) {
            --endIterator;
            destroy(endIterator.curP);
            endIterator.curP = revert_curP;
            endIterator.chunkP = revert_chunkP;
            dealloc(*(endIterator.map_iterator));
            --sz_;
            throw;
        }
    }
}

template <typename T>
void Deque<T>::push_front(const T& value) {
    ++sz_;
    if (beginIterator.curP == beginIterator.chunkP) {
        int place_to_construct =
            beginIterator.map_iterator - chunkMap_.begin() - 1;
        bool flag = false;
        try {
            alloc(chunkMap_[place_to_construct]);
            flag = true;
            construct(chunkMap_[place_to_construct] + chunkSize_ - 1, value);
        } catch (...) {
            --sz_;
            if (flag) {
                dealloc(chunkMap_[place_to_construct]);
            }
            throw;
        }
        --beginIterator;
        if (beginIterator.map_iterator == chunkMap_.begin()) {
            try {
                realloc();
            } catch (...) {
                --sz_;
                ++beginIterator;
                throw;
            }
        }
    } else {
        --beginIterator;
        try {
            construct(beginIterator.curP, value);
        } catch (...) {
            --sz_;
            ++beginIterator;
        }
    }
}

template <typename T>
void Deque<T>::erase(const iterator& iter) {
    --sz_;
    if (endIterator - 1 == iter) {
        ++sz_;
        try {
            pop_back();
            return;
        } catch (...) {
            throw;
        }
    }
    for (iterator it = iter + 1; it != endIterator; ++it) {
        *((it - 1).curP) = *(it.curP);
    }
    --endIterator;
    destroy(endIterator.curP);
    if (endIterator.curP == endIterator.chunkP) {
        dealloc(*(endIterator.map_iterator));
        endIterator.chunkP = nullptr;
        endIterator.curP = nullptr;
    }
}

template <typename T>
void Deque<T>::pop_back() {
    --sz_;
    if (endIterator.curP == endIterator.chunkP &&
        endIterator.chunkP != nullptr) {
        dealloc(*(endIterator.map_iterator));
        endIterator.chunkP = nullptr;
        endIterator.curP = nullptr;
    }
    --endIterator;
    destroy(endIterator.curP);
    if (endIterator.curP == endIterator.chunkP) {
        dealloc(*(endIterator.map_iterator));
        endIterator.chunkP = nullptr;
        endIterator.curP = nullptr;
    }
}

template <typename T>
void Deque<T>::pop_front() {
    --sz_;
    destroy(beginIterator.curP);
    ++beginIterator;
    if (beginIterator.curP == beginIterator.chunkP) {
        dealloc(*(beginIterator.map_iterator - 1));
    }
}

template <typename T>
void Deque<T>::alloc(T*& p) {
    p = reinterpret_cast<pointer>(new char[chunkSize_ * sizeof(T)]);
}

template <typename T>
void Deque<T>::dealloc(T*& p) {
    delete[] reinterpret_cast<char*>(p);
    p = nullptr;
}

template <typename T>
void Deque<T>::realloc() {
    Deque<T> tmp = *this;
    try {
        int new_size = chunkMapSize_ * 2;
        std::vector<T*> new_map(new_size, nullptr);
        int begin_index = beginIterator.map_iterator - chunkMap_.begin();
        int end_index = endIterator.map_iterator - chunkMap_.begin();
        for (int i = begin_index; i <= end_index; ++i) {
            new_map[i - begin_index + new_size / 4] = chunkMap_[i];
        }
        chunkMap_ = new_map;
        beginIterator.map_iterator = chunkMap_.begin() + new_size / 4;
        endIterator.map_iterator =
            chunkMap_.begin() + end_index - begin_index + new_size / 4;
        chunkMapSize_ = new_size;
    } catch (...) {
        *this = tmp;
        throw;
    }
}

template <typename T>
void Deque<T>::construct(pointer p, const T& value) {
    new (p) T(value);
}

template <typename T>
void Deque<T>::destroy(pointer p) {
    p->~T();
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const {
    return const_iterator(beginIterator);
}
template <typename T>
typename Deque<T>::iterator Deque<T>::begin() {
    return beginIterator;
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rbegin() const {
    return const_reverse_iterator(endIterator);
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() {
    return reverse_iterator(endIterator);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
    return const_iterator(beginIterator);
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const {
    return const_reverse_iterator(endIterator);
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() {
    return endIterator;
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::end() const {
    return const_iterator(endIterator);
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rend() const {
    return const_reverse_iterator(beginIterator);
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() {
    return reverse_iterator(beginIterator);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
    return const_iterator(endIterator);
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const {
    return const_reverse_iterator(beginIterator);
}
