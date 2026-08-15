#include"error.h"
void f() throws;
// noexcept function calling a throws function without handling is an error.
void g() noexcept { f(); }
