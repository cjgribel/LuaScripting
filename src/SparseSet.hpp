#ifndef SparseSet_hpp
#define SparseSet_hpp

#include <cassert>
#include <array>

template<class T, int N>
class SparseSet
{
    using DenseArray = std::array<T, N>;
    using SparseArray = std::array<int, N>;

    DenseArray dense;
    SparseArray sparse;
    int dense_count = 0;

public:
    SparseSet()
    {
        sparse.fill(-1);
    }

    // Setters and getters for inspection
    
    void set_dense_array(const DenseArray& array) { dense = array; }
    const DenseArray& get_dense_array() const { return dense; }
    
    void set_sparse_array(const SparseArray& array) { sparse = array; }
    const SparseArray& get_sparse_array() const { return sparse; }
    
    void set_dense_count(int count) { dense_count = count; }

    /// @brief Check if an index exists in the set
    /// @param index 
    /// @return 
    bool contains(int index) const
    {
        assert(index >= 0 && index < N && "Index out of bounds");
        return sparse[index] != -1;
    }

    /// @brief Add an index to the set if it does not already exist
    /// @param index 
    void add(int index)
    {
        assert(dense_count < N && "GridSize limit reached");
        assert(index >= 0 && index < N && "Index out of bounds");
        if (contains(index)) return;

        sparse[index] = dense_count;
        dense[dense_count] = index;
        dense_count++;
    }

    /// @brief Remove an index from the set if it exists
    /// @param index 
    void remove(int index)
    {
        assert(index >= 0 && index < N && "Index out of bounds");
        if (!contains(index)) return;

        int index_to_remove = sparse[index];
        int last_index = dense_count - 1;

        if (index_to_remove != last_index)
        {
            // Back-swap
            dense[index_to_remove] = dense[last_index];
            sparse[dense[last_index]] = index_to_remove;
        }

        // Invalidate the removed element
        sparse[index] = -1;
        dense_count--;
    }

    /// @brief Get index value from the dense map at an index
    /// @param index Dense map index
    /// @return index value
    T get_dense(int index) const
    {
        assert(index >= 0 && index < dense_count && "Index out of bounds");
        return dense[index];
    }

    /// @brief Get number of elements in the dense map
    /// @return Number of added indices 
    int get_dense_count() const
    {
        return dense_count;
    }
};

#endif // SparseSet_hpp
