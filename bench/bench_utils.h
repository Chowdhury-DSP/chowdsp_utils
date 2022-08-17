#pragma once

#include <random>
#include <vector>

namespace bench_utils
{
template <typename T, typename allocator = std::allocator<T>>
inline auto makeRandomVector (int num)
{
    std::random_device rnd_device;
    std::mt19937 mersenne_engine { rnd_device() }; // Generates random integers
    std::normal_distribution<float> dist { (T) 0, (T) 1 };

    std::vector<T, allocator> vec ((size_t) num);
    std::generate (vec.begin(), vec.end(), [&dist, &mersenne_engine]()
                   { return dist (mersenne_engine); });

    return std::move (vec);
}
} // namespace bench_utils
