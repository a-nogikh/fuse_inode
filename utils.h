#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#define DIV_ROUND_UP(a,b) ((a/b) + ((a-b) > 0) ? 1 : 0)

int str_take_till(const char *str, char *dst, char till, int limit);

#endif // UTILS_H_INCLUDED
