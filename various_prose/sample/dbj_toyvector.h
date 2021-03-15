
#ifndef DBJ_SMALL_VECTOR_INC_
#define DBJ_SMALL_VECTOR_INC_
/*
(c) 20201 dbj@dbj.org https://dbj.org/license_dbj

This is a toy for playing with toys

https://godbolt.org/z/n679zh
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>

namespace dbj
{

    template <typename T, size_t N, typename buffer_type = T[N] >
    struct toy_vector final
    {
        static_assert(std::is_trivial_v<T>, "dbj toy_vector value type must be trivial");

        using value_type = T ;

        constexpr inline static auto stack_taken_ = N;
        constexpr inline static auto max_stack_taken_ = 0xFF ;

        static_assert(N < max_stack_taken_, "dbj toy_vector can not take more than 0xFF from the stack");
        static_assert( std::is_nothrow_move_assignable_v<T>  , "dbj toy_vector is movable only it T is movable");
        static_assert( std::is_nothrow_move_constructible_v <T>  , "dbj toy_vector is movable only it T is movable");

        buffer_type stack_buffer_{value_type()};
        size_t size{0};
        size_t capacity{stack_taken_};
        value_type *heap_buffer_{ nullptr };

        const value_type * const data () noexcept 
        {
              if ( size > stack_taken_ )
                  return heap_buffer_ ;
              else 
                  return stack_buffer_ ;   
        }

        // no copy
        toy_vector(const toy_vector &rhs) = delete;
        toy_vector &operator=(const toy_vector &rhs) = delete;

        toy_vector() noexcept : heap_buffer_(stack_buffer_) {}

        ~toy_vector()
        {
            if ( size > stack_taken_ )
            {
                free(heap_buffer_);
                heap_buffer_ = nullptr;
            }
        };

        // how realloc should be done
        static void increase(value_type **data, size_t new_size_ ) noexcept
        {
            *data = (value_type*)realloc(*data, new_size_ * sizeof(value_type));
            if (*data == nullptr) 
            {
                perror("dbj::toy_vector::increase() realloc failed ") ;
                exit(0);
            }
        }

        value_type &push_back(int value) noexcept
        {
            if (size == capacity)
            {
                capacity += capacity ; // double but use + not *
                    if ( size == stack_taken_ ) {
                         // leaving the stack_buffer_
                         heap_buffer_ = (value_type*)calloc( capacity, sizeof(value_type) ) ;
                         memcpy( heap_buffer_, stack_buffer_, stack_taken_ ) ; 
                    } else {
                        increase( & heap_buffer_, capacity ) ;
                    }
            }
            return heap_buffer_[size++] = value;
        }

        // valid index: 0 ... size-1
        value_type &operator[](size_t index) noexcept
        {
            return heap_buffer_[index > (size-1)  ? 0 : index ]; // ridiculous :)
        }

        value_type &at(size_t index) = delete;
    };
} // dbj

#if 0 // testing example
#include <iostream>
#define SX(X_) std::cout << "\n" << __LINE__ << ": " << #X_ << " : " << (X_)

int main (void)
{
    auto print = []( auto & tv ) {
    std::cout << "\n\n size: " << tv.size << ", capacity: " << tv.capacity << " --> {";
    for ( size_t j = 0; j < tv.size; ++j )
    {
        std::cout << " " << tv[j] ;
    }
    std::cout << " }" ;
    };

    auto tv = dbj::toy_vector<char,3>();

    tv.push_back('A') ;
    tv.push_back('B') ;
    tv.push_back('C') ; 

    print(tv);

    tv.push_back('D') ;
    tv.push_back('E') ;
    tv.push_back('F') ; 

    print(tv);

    SX(  tv[99] ) ;

    return 42;
}
#endif // 0

#endif // DBJ_SMALL_VECTOR_INC_
