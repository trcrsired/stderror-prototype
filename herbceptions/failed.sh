$HOME/toolchains_build/llvm_herbceptions/build/bin/clang++ -S test.cc  -fherbceptions -std=c++26 -O3
$HOME/toolchains_build/llvm_herbceptions/build/bin/clang++ -S test_func.cc  -fherbceptions -std=c++26 -O3
$HOME/toolchains_build/llvm_herbceptions/build/bin/clang++ -S test_pass.cc  -fherbceptions -std=c++26 -O3
$HOME/toolchains_build/llvm_herbceptions/build/bin/clang++ -S test_rethrow.cc  -fherbceptions -std=c++26 -O3

## should fail
$HOME/toolchains_build/llvm_herbceptions/build/bin/clang++ -S test_rethrow_failed.cc  -fherbceptions -std=c++26 -O3



