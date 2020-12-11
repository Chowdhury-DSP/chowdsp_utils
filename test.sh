cd build
cmake --build . --parallel
ctest -j2 -C Debug -V
