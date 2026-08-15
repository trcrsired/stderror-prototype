CXX="$HOME/toolchains_build/llvm_herbceptions/build/bin/clang++"
CC="$HOME/toolchains_build/llvm_herbceptions/build/bin/clang"
FAIL=0

pass() { echo "PASS  $1"; }
fail() { echo "FAIL  $1"; FAIL=1; }

check_ok() {
  if "$CXX" -S "$1" -fherbceptions -std=c++26 -O3 -o /dev/null 2>/tmp/herb_err.txt; then
    pass "$1"
  else
    fail "$1 (should compile)"
    sed 's/^/       /' /tmp/herb_err.txt | head -5
  fi
}

check_fail() {
  if "$CXX" -S "$1" -fherbceptions -std=c++26 -O3 -o /dev/null 2>/tmp/herb_err.txt; then
    fail "$1 (should NOT compile)"
  else
    pass "$1 (rejected as expected)"
  fi
}

check_c_ok() {
  if "$CC" -fsyntax-only "$1" -fherbceptions -std=c2x 2>/tmp/herb_err.txt; then
    pass "$1"
  else
    fail "$1 (should compile as C)"
    sed 's/^/       /' /tmp/herb_err.txt | head -5
  fi
}

# --- C++: should compile ---
check_ok test.cc
check_ok test_func.cc
check_ok test_pass.cc
check_ok test_rethrow.cc
check_ok test_rethrow_condition.cc      # throws(true)
check_ok test_main.cc                   # main: unhandled -> __builtin_trap()
check_ok test_pass_with_fails_eh.cc     # fails{E} interop into throws

# --- C++: should fail ---
check_fail test_rethrow_failed_noexcept.cc  # noexcept calling throws
check_fail fails_paren.cc                   # fails(E) is invalid, use fails{E}
check_fail fails_sderror.cc                 # fails{std::error} not allowed
check_fail noexcept_calls_throws.cc         # noexcept calling throws without handling

# --- C: fails{E} is a C feature ---
check_c_ok c_fails.c

exit $FAIL
