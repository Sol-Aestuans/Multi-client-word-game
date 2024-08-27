/*
 * Do not modify the following lines. Add your code at the end of this file.
 */
#ifdef DEBUG_FLAG
#define DEBUG_wARG(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define DEBUG_MSG(msg) fprintf(stderr, "%s\n", msg);
#else
#define DEBUG_wARG(fmt, ...)
#define DEBUG_MSG(msg)
#endif

#define PRINT_wARG(fmt, ...) printf(fmt, __VA_ARGS__); fflush(stdout)
#define PRINT_MSG(msg) printf("%s", msg); fflush(stdout)

int read_stdin(char *buf, int buf_len, int *more);

/* ========= DO NOT modify anything above this. Add your code below this line. =============== */
