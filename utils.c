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
