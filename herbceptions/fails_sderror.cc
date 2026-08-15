#include"error.h"
// std::error is not allowed as a fails error type.
void g() fails{::std::error};
