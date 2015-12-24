#include "utils.h"

int str_take_till(char *str, char *dst, char till, int limit){
    int taken = 0;
    while (*str != '\0' && *str != till
           && (limit < 0 || limit > taken)){
        *dst++ = *str++;
        taken++;
    }

    return taken;
}

int str_before_last(char *str, char *dst, char symbol){
    int last = 0, i = 0;
    char *copy = str;
    while (*copy != '\0'){
        if (*copy == symbol){
            last = i;
        }
        i++;
        copy++;
    }

    for (i = 0; i < last; i++){
        *(dst + i) = *(str + i);
    }
    *(dst + last) = '\0';
    return last;
}

int str_after_last(char *str, char *dst, char symbol){
    int last = 0, total = 0, i = 0;
    char *copy = str;
    while (*copy != '\0'){
        if (*copy == symbol){
            last = total;
        }
        copy++;
        total++;
    }

    for (i = last + 1; i < total; i++){
        *(dst + i - last - 1) = *(str + i);
    }

    *(dst + total - last - 1) = '\0';
    return last;
}
