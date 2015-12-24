#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#define DIV_ROUND_UP(a,b) ((a/b) + ((a-b) > 0) ? 1 : 0)

int str_take_till(char *str, char *dst, char till, int limit);
int str_before_last(char *str, char *dst, char symbol);
int str_after_last(char *str, char *dst, char symbol);

#endif // UTILS_H_INCLUDED
