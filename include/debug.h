#ifndef INCLUDE_DEBUG_H
#define INCLUDE_DEBUG_H

void debug_init(void);
void debug_close(void);
void debug_printf(const char *file, const char *func, const char *line, const char *str, ...);

#define DEBUG_FILE "/tmp/gbemuc_debug.txt"

#if CONFIG_DEBUG

extern int global_debug_flag;

#define DEBUG_INIT() debug_init()
#define DEBUG_CLOSE() debug_close()
#define DEBUG_PRINTF(...) \
    do { \
        if (global_debug_flag) \
            debug_printf(__FILE__, __func__, Q(__LINE__), __VA_ARGS__); \
    } while (0)

#define DEBUG_ON() do { global_debug_flag = 1; } while (0)
#define DEBUG_OFF() do { global_debug_flag = 0; } while (0)

#else

#define DEBUG_INIT() do { ; } while (0)
#define DEBUG_CLOSE() do { ; } while (0)
#define DEBUG_PRINTF(...) do { ; } while (0)

#define DEBUG_ON() do { ; } while (0)
#define DEBUG_OFF() do { ; } while (0)

#endif

#endif
