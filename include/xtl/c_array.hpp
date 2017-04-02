/// @file      c_array.hpp
/// @brief     eXtra Template Library (XTL) - C array utilities implementation.
/// @copyright Licensed under the MIT License.
/// @author    Rostislav Ostapenko (rostislav.ostapenko@gmail.com)
/// @date      13-Mar-2011

#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <stdexcept>

namespace xtl
{

    /// @brief Adapter for classic C array implementing standard container interface.
    ///        Can be used with legacy code if for some reasons std::array or std::vector usage
    ///        is not acceptable, e.g. when C array is used as part of well-known interface and
    ///        cannot be easily changed to modern standard container.
    template <typename T, size_t Size> class array_ref
    {
    public:

        /// @brief Standard container typedefs: value, reference, pointer, iterator
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        /// @brief Wrapped array typedefs
        using array_type = T[Size];
        using array_reference = array_type&;
        using const_array_reference = const array_reference;

        // Zero size arrays are illegal in C++, but just for case ;-)
        static_assert(Size > 0, "Zero size array not allowed");

        array_ref(array_reference ref): ref_(ref)
        {
        }

        array_ref(const array_ref& rhs): ref_(rhs.ref_)
        {
        }

        array_ref& operator=(const array_ref& rhs)
        {
            if (&ref_ != &rhs.ref_)
                std::copy(rhs.begin(), rhs.end(), begin());
            return *this;
        }

        template<typename X, size_type OtherSize>
        array_ref& operator=(const array_ref<X, OtherSize>& rhs)
        {
            const size_type size_to_copy = Size < OtherSize ? Size : OtherSize;
            if (begin() != rhs.begin())
                std::copy(rhs.begin(), rhs.begin() + size_to_copy, begin());
            return *this;
        }

        template<typename X, size_type OtherSize>
        array_ref& operator=(const X(&rhs)[OtherSize])
        {
            const size_type size_to_copy = Size < OtherSize ? Size : OtherSize;
            std::copy(rhs, rhs + size_to_copy, begin());
            return *this;
        }

        void swap(array_ref& rhs)
        {
            std::swap_ranges(rhs.begin(), rhs.end(), begin());
        }

        void swap(array_reference rhs)
        {
            std::swap_ranges(rhs, rhs + Size, begin());
        }

        reference operator[](size_type index)
        {
            return ref_[index];
        }

        const_reference operator[](size_type index) const
        {
            return ref_[index];
        }

        reference at(size_type index)
        {
            check_index(index);
            return ref_[index];
        }

        const_reference at(size_type index) const
        {
            check_index(index);
            return ref_[index];
        }

        reference front()
        {
            return ref_[0];
        }

        const_reference front() const
        {
            return ref_[0];
        }

        reference back()
        {
            return ref_[Size - 1];
        }

        const_reference back() const
        {
            return ref_[Size - 1];
        }

        size_type size() const
        {
            return Size; // std::extent<array_type>::value could be used
        }

        bool empty() const
        {
            return size() == 0;
        }

        size_type max_size() const
        {
            return std::numeric_limits<size_type>::max();
        }

        const_iterator cbegin() const
        {
            return ref_;
        }

        const_iterator cend() const
        {
            return ref_ + Size;
        }

        const_iterator begin() const
        {
            return ref_;
        }

        const_iterator end() const
        {
            return ref_ + Size;
        }

        iterator begin()
        {
            return ref_;
        }

        iterator end()
        {
            return ref_ + Size;
        }

        const_reverse_iterator crbegin()
        {
            return reverse_iterator(cend());
        }

        const_reverse_iterator crend()
        {
            return reverse_iterator(cbegin());
        }

        reverse_iterator rbegin()
        {
            return reverse_iterator(end());
        }

        reverse_iterator rend()
        {
            return reverse_iterator(begin());
        }

        bool valid_index(size_type index) const
        {
            return index < size();
        }

        bool find_index(const_reference item, size_type& item_index) const
        {
            const const_iterator found = std::find(cbegin(), cend(), item);
            if (found == cend()) return false;

            item_index = found - cbegin(); // Assuming ptrdiff_t is compatible with size_type
            return true;
        }

    private:

        void check_index(size_t index)
        {
            if (!valid_index(index))
                throw std::out_of_range("xtl::array_ref - array index out of bounds");
        }

        array_reference ref_; ///< Wrapped array reference
    };

    /// @brief     Wraps C array with array_ref deducing template arguments automatically.
    /// @tparam    T    type of array elements
    /// @tparam    Size static size of array
    /// @param[in] ref  reference to source array to be wrapped
    /// @retval    array_ref<T, Size> instance wrapping the source array
    template <typename T, size_t Size>
    array_ref<T, Size> make_array_ref(T (&ref)[Size])
    {
        return array_ref<T, Size>(ref);
    }

    /// @brief     Casts C array from pointer and wraps it with array_ref.
    /// @tparam    Size static size of array. It's responsibility of caller to match it to actual array size.
    /// @tparam    T    type of array elements
    /// @param[in] ref  pointer to first element of array to be wrapped. If pointed memory block actually has smaller size
    ///                 than required to place array of Size then behavior is undefined.
    /// @retval    array_ref<T, Size> instance wrapping the source array
    template <size_t Size, typename T>
    array_ref<T, Size> make_array_ref(T* ptr)
    {
        using arr_ref = array_ref<T, Size>;
        return arr_ref(reinterpret_cast<arr_ref::array_reference>(*ptr));
    }

} // namespace xtl
