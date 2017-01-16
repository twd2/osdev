#include "iso9660.h"

#include <stdlib/memory.h>

directory_record_t *next_record(directory_record_t *ptr)
{
    return ((uint8_t *)ptr) + ptr->length;
}

directory_record_t *find_record(const char *pattern, directory_record_t *begin, directory_record_t *end)
{
    for (directory_record_t *iter = begin; iter->length && iter < end; iter = next_record(iter))
    {
        if (ident_cmp(pattern, iter->ident_length, &iter->ident))
        {
            return iter;
        }
    }
    return NULL;
}

bool ident_cmp(const char *pattern, uint32_t ident_length, const char *ident)
{
    uint32_t length = strlen(pattern);
    if (ident_length < length)
    {
        return false;
    }
    if (memcmp(pattern, ident, length))
    {
        return false;
    }
    // FILENAME.EXT;version
    if (ident_length > length && ident[length] != ';' && ident[length] != 0)
    {
        return false;
    }
    return true;
}

void to_ident(char *pattern)
{
    while (*pattern)
    {
        if (*pattern >= 'a' && *pattern <= 'z')
        {
            *pattern = *pattern - 'a' + 'A';
        }
        ++pattern;
    }
}
