#include"error.h"
// fails takes a type in braces (fails{E}), not parentheses (fails(E)).
void g() fails(::std::win32_errc);
