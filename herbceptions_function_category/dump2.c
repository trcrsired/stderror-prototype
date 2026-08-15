typedef enum { A } e1;
int f(int x) fails{int} { if (x) return failure(42); return 2*x; }
int main() {
  auto t = catch fails(f(0));
  return t.value;
}
