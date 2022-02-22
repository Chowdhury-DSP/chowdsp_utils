cd build
cmake --build . --parallel
ctest -j4 -C Debug -V
