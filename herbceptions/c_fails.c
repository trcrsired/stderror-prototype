// C-style fails{E} — a C feature. Everything must be explicit (no invisible
// propagation): calling a fails{E} function requires try() or catch fails().

int some_c_function(int x) fails{int} {
  if (x == 0) return failure(42);
  return x * 2;
}

int main() {
  // Explicit handling: catch fails(expr) returns the built-in either{T, E}.
  auto r = catch fails(some_c_function(0));
  if (r.positive) return 1;      // must have failed
  if (r.right != 42) return 2;   // failure value is 42
  return 0;
}
