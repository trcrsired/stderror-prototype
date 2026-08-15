int f(int x) fails{int} {
  if (x == 0) return failure(42);
  return 2 * x;
}
int use_catch(int x) {
  auto r = catch fails(f(x));
  if (r.failed) return r.error;
  return r.value;
}
static_assert(use_catch(3) == 6, "success");
