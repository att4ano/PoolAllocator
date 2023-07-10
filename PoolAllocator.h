#include <iostream>
#include <initializer_list>

template<typename T>
class PoolAllocator {
public:
    using size_type = size_t;
    using value_type = T;
    using pointer = T*;
    using pair = std::pair<size_t, size_t>;
private:

    struct Pool {
        size_type number_chunk_ = 0;
        size_type chunk_size_ = 0;
        size_type remain_ = 0;
        uint8_t* pool = nullptr;
        bool* pool_mask = nullptr;

        Pool() = default;

        Pool(size_type chunk_size, size_type number_chunk) :
                number_chunk_(number_chunk),
                chunk_size_(chunk_size),
                remain_(number_chunk * chunk_size) {
            pool = static_cast<uint8_t*>(malloc(remain_));
            pool_mask = static_cast<bool*>(malloc(number_chunk));
            for (size_type i = 0; i < number_chunk; ++i) {
                pool_mask[i] = false;
            }
        }

        ~Pool() {
            free(pool_mask);
            free(pool);
        }
    };

    size_type number_pools = 0;

    size_type FindBest(size_type n) {
        size_type min_index = number_pools;
        for (size_type i = 0; i < number_pools; ++i) {
            if (min_index != number_pools) {
                if (pools[min_index]->remain_ > pools[i]->remain_) {
                    min_index = i;
                }
            } else if (pools[i]->remain_ >= n * sizeof(T)) {
                min_index = i;
            }
        }
        if (min_index == number_pools) {
            throw std::bad_alloc();
        } else {
            return min_index;
        }
    }

    size_type FindPool(pointer ptr) noexcept {
        for (size_type i = 0; i < number_pools; ++i) {
            if ((reinterpret_cast<uint8_t*>(ptr) - (pools[i]->pool)) >= 0 &&
                ((reinterpret_cast<uint8_t*>(ptr) - (pools[i]->pool))) <=
                pools[i]->number_chunk_ * pools[i]->chunk_size_) {
                return i;
            }
        }
        return number_pools;
    }

    Pool** pools;

public:

    PoolAllocator(const std::initializer_list<pair>& pairs) {
        pools = reinterpret_cast<Pool**>(malloc(pairs.size() * sizeof (Pool**)));
        size_type index = 0;
        number_pools = pairs.size();
        for (auto& pair: pairs) {
            size_type size_of_chunks = pair.first;
            size_type number_of_elements = pair.second;
            pools[index] = new Pool(size_of_chunks, number_of_elements);
            ++index;
        }
    }

    ~PoolAllocator() {
        std::destroy_n(pools, number_pools);
    }

    template<class U>
    friend class PoolAllocator;

    template<typename U>
    explicit PoolAllocator(const PoolAllocator<U>& other) : pools(reinterpret_cast<Pool*>(other.pools)),
                                                            number_pools(other.number_pools) {}

    pointer allocate(size_type n) {
        size_type best_pool = FindBest(n);
        size_type counter = 0;
        size_type chunks_amount = ((n * sizeof(T) / pools[best_pool]->chunk_size_) +
                                   ((((n * sizeof(T)) % pools[best_pool]->chunk_size_ != 0)) * 1));
        for (size_type i = 0; i < pools[best_pool]->number_chunk_; ++i) {
            if (!pools[best_pool]->pool_mask[i]) {
                ++counter;
                if (counter == chunks_amount) {
                    for (size_type j = i - counter + 1; j <= i; ++j) {
                        pools[best_pool]->pool_mask[j] = true;
                    }
                    pools[best_pool]->remain_ -= chunks_amount * pools[best_pool]->chunk_size_;
                    return reinterpret_cast<pointer>(pools[best_pool]->pool +
                                                     (i - counter + 1) * pools[best_pool]->chunk_size_);
                }
            } else {
                counter = 0;
            }
        }
        throw std::bad_alloc();
    }

    void deallocate(pointer ptr, size_type k) {
        size_type pool_index = FindPool(ptr);
        if (pool_index == number_pools) {
            throw std::logic_error("Pointer is not in our memory");
        } else {
            size_type chunks_amount = ((k * sizeof(value_type) / pools[pool_index]->chunk_size_) +
                                       ((((k * sizeof(value_type)) % pools[pool_index]->chunk_size_ != 0)) * 1));
            size_type position = (ptr - reinterpret_cast<pointer>(pools[pool_index]->pool));
            for (size_type i = 0; i < chunks_amount; ++i) {
                pools[pool_index]->pool_mask[position + i] = false;
            }
            pools[pool_index]->remain_ += pools[pool_index]->chunk_size_ * chunks_amount;
        }
    }

};

