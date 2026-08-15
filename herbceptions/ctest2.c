constexpr int add(int a, int b) { return a + b; }
static_assert(add(2, 3) == 5, "constexpr C function");
int main(){return 0;}
