#include "decision-graph/util/Str.hpp"
#include <cstring>
#include <cstdlib>

// ----------------------------------------------------------------------------
char* StrDup(const char* s)
{
    const int len = strlen(s);
    char* dup = (char*)malloc(len + 1);
    strcpy(dup, s);
    return dup;
}

// ----------------------------------------------------------------------------
void StrFree(char* s)
{
    free(s);
}
