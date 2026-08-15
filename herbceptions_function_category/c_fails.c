// C-style fails{E} — a C feature. Everything must be explicit (no invisible
// propagation): calling a fails{E} function requires try() or catch fails().
//
// `catch fails(expr)` yields the N2289 aggregate
//   struct { union { T value; E error; }; bool failed; }
// so `r.failed` is the discriminant, `r.value` the success value (when
// `!r.failed`) and `r.error` the failure value.

int some_c_function(int x) fails{int} {
  if (x == 0) return failure(42);
  return x * 2;
}

int main() {
  // Explicit handling: catch fails(expr) returns the N2289 aggregate.
  auto r = catch fails(some_c_function(0));
  if (r.failed) {
    if (r.error != 42) return 2;   // failure value is 42
  } else {
    return 1;                       // must have failed
  }

  auto ok = catch fails(some_c_function(3));
  if (ok.failed) return 3;          // must have succeeded
  if (ok.value != 6) return 4;      // success value is 6
  return 0;
}
