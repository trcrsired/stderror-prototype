// constexpr fails{E} with try(expr) auto-propagation.
constexpr int f(int x) fails{int} {
  if (x == 0) return failure(42);
  return 2 * x;
}

constexpr int g(int x) fails{int} {
  return try(f(x));   // auto-propagate f's failure
}

constexpr int use_catch(int x) {
  auto r = catch fails(g(x));
  if (r.failed) return r.error;
  return r.value;
}

static_assert(use_catch(3) == 6, "success");
static_assert(use_catch(0) == 42, "failure");

int main(){ return 0; }
