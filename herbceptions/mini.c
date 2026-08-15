typedef int myint;
int f(int x) fails{int} { if (x) return failure(42); return x; }
int main() { auto r = catch fails(f(1)); return r.failed ? r.error : r.value; }
