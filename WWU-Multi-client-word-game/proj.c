
#include <stdio.h>
#include <string.h>

#include "proj.h"

int read_stdin(char *buf, int buf_len, int *more) {
    // TODO: Copy your implementation from project 1.
    int count = 0;
    fgets(buf, buf_len, stdin);
    for (;count < buf_len && buf[count]!='\0' && buf[count]!='\n';count++);
    if (count == buf_len-1)
        *more = 1;
    else
        *more = 0;
    return count;
}
