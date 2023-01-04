# types_list

[![Tests (Ubuntu)](https://github.com/jatinchowdhury18/types_list/workflows/ubuntu/badge.svg)](https://github.com/jatinchowdhury18/types_list/actions/workflows/ubuntu.yml)
[![Tests (Mac)](https://github.com/jatinchowdhury18/types_list/workflows/macos/badge.svg)](https://github.com/jatinchowdhury18/types_list/actions/workflows/macos.yml)
[![Tests (Windows)](https://github.com/jatinchowdhury18/types_list/workflows/windows/badge.svg)](https://github.com/jatinchowdhury18/types_list/actions/workflows/windows.yml)
[![License](https://img.shields.io/github/license/Neargye/nameof.svg)](LICENSE)
[![Compiler explorer](https://img.shields.io/badge/compiler_explorer-online-blue.svg)](https://godbolt.org/z/dsz9GzzP6)

A types_list template object for C++17 and later.

## Features

* C++17
* Header-only
* Dependency-free
* Construction of compile-time types lists
* Inter-operate between types lists and `std::tuple`
* Iterate over types in a types list at run-time or compile-time

## [Examples](example/example.cpp)

- Making a types list:
  ```cpp
  // Making a types list directly
  using List1 = types_list::TypesList<Type1, Type2, std::string, int>;

  // Making a types list by insertion
  using List2 = List1::InsertFront<Type3>::InsertBack<double>;

  // Making a types list by concatenation
  using List3 = List1::Concatenate<List2>;
  ```

- Inter-operating with `std::tuple`
  ```cpp
  using Tuple1 = std::tuple<int, float, double>;

  // Making a types list from a std::tuple type
  using TupleAsList = types_list::TupleList<Tuple1>;

  // Making a std::tuple type from a types list
  using Tuple2 = TupleAsList::Types;

  // Check that everything worked...
  static_assert (std::is_same_v<Tuple1, Tuple2>);
  ```

- Iterating over a types list
  ```cpp
  // Print the epsilon value for different floating point types:
  using List = types_list::TypesList<float, double, long double>;
  types_list::forEach<List>([] (auto typeIndex) {
      using TypeAtIndex = List::AtIndex<typeIndex>;
      std::cout << std::numeric_limits<TypeAtIndex>::epsilon() << std::endl;
  });
  ```

## Integration

You should add required file [types_list.hpp](include/types_list/types_list.hpp).

You can also use something like [CPM](https://github.com/TheLartians/CPM) which is based on CMake's `Fetch_Content` module.

```cmake
CPMAddPackage(
    NAME types_list
    GITHUB_REPOSITORY jatinchowdhury18/types_list
    GIT_TAG x.y.z # Where `x.y.z` is the release version you want to use.
)
```

## Compiler compatibility

* Clang/LLVM >= 6
* MSVC++ >= 14.11 / Visual Studio >= 2017
* Xcode >= 10
* GCC >= 7

## License

`types_list` is licensed under the MIT license. Enjoy!
