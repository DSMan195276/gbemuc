
#include "common.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <emmintrin.h>
#include <protura/fb.h>
#include <protura/event/keyboard.h>
#include <protura/event/protocol.h>

#include "gb.h"
#include "protura_driver.h"

struct gb_display_protura {
    struct gb_gpu_display gb_disp;

    struct fb_map framebuffer;
    struct fb_dimensions dim;

    int offset_x;
    int offset_y;
    int scale;

    int fps_count;
    uint64_t last_tick_us;
    uint64_t fps_start_us;

    int event_fd;
    int fd0_fd;

    int cur_palette;
};

struct gb_protura_driver {
    struct gb_display_protura disp;
};

struct gb_dmg_theme gb_palettes[] = {
    GB_DEFINE_THEME(0xFFFFF77B, 0xFFB5AE4A, 0xFF6B6931, 0xFF212010,
                    0xFFFFF77B, 0xFFB5AE4A, 0xFF6B6931, 0xFF212010,
                    0xFFFFF77B, 0xFFB5AE4A, 0xFF6B6931, 0xFF212010),
    GB_DEFINE_THEME(0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000,
                    0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000,
                    0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000),
    GB_DEFINE_THEME(0xFF9BBC0F, 0xFF8BAC0F, 0xFF305230, 0xFF0F380F,
                    0xFF9BBC0F, 0xFF8BAC0F, 0xFF305230, 0xFF0F380F,
                    0xFF9BBC0F, 0xFF8BAC0F, 0xFF305230, 0xFF0F380F),
};

#define GB_PALETTES (sizeof(gb_palettes)/sizeof(*gb_palettes))


/* GB runs at 59.7 FPS. This number is in microseconds to give us a bit better
 * precision for our sleep */
static const unsigned int gb_us_per_frame = 16750;

static uint64_t timeval_to_us(const struct timeval *timeval)
{
    return (uint64_t)timeval->tv_sec * 1000000 + timeval->tv_usec;
}

static void gb_protura_display(struct gb_gpu_display *disp, union gb_gpu_color_u *buff)
{
    struct gb_display_protura *protura = container_of(disp, struct gb_display_protura, gb_disp);
    uint32_t *fb = protura->framebuffer.framebuffer;

    int x, y, x1, y1;
    int disp_width = protura->dim.width;
    int y_offset = disp_width * protura->offset_y;

    /* A pointer to the starting byte of the current Y we're at.
     *
     * We introduce the protura->offset_x here just as an optimization, since
     * every line starts at that point anyway. */
    uint32_t *fb_y_start = fb + disp_width * protura->offset_y + protura->offset_x;
    union gb_gpu_color_u *buf_offset = buff;

    /* NOTE: We need to write in a linear fashion to the framebuffer memory to
     * achieve the required speed to run at 60fps. This works along with the
     * write-combine caching mode to ensure we only read/write each cache line
     * once, rather than once per write.
     *
     * Since we write 4 bytes at a time, and a cache line is probably 32 bytes,
     * that a potential 8x speed up for linear writes vs. non-linear writes. */
    for (y = 0; y < GB_SCREEN_HEIGHT; y++, buf_offset += GB_SCREEN_WIDTH) {
        for (y1 = 0; y1 < protura->scale; y1++, fb_y_start += disp_width) {

            uint32_t *fb_x_start = fb_y_start;
            union gb_gpu_color_u *cur_line = buf_offset;

            for (x = 0; x < GB_SCREEN_WIDTH; x++, cur_line++) {
                uint32_t color = cur_line->i_color;

                for (x1 = 0; x1 < protura->scale; x1++, fb_x_start++)
                    *fb_x_start = color;
            }
        }
    }

    if (!protura->last_tick_us) {
        struct timeval timeval;
        gettimeofday(&timeval, NULL);

        protura->last_tick_us = timeval_to_us(&timeval);
    } else {
        struct timeval timeval;
        gettimeofday(&timeval, NULL);

        uint64_t cur_tick_us = timeval_to_us(&timeval);
        uint64_t elapsed = cur_tick_us - protura->last_tick_us;

        if (elapsed < gb_us_per_frame) {
            /* useconds_t is signed, eww. But we shouldn't ever get negative
             * since elapsed < gb_us_per_frame */
            useconds_t diff = gb_us_per_frame - elapsed;

            usleep(diff);

            gettimeofday(&timeval, NULL);
            cur_tick_us = timeval_to_us(&timeval);
        }

        protura->last_tick_us = timeval_to_us(&timeval);

        protura->fps_count++;

        /* Print FPS aproximately every second */
        if ((cur_tick_us - protura->fps_start_us) >= 1000000) {
            uint64_t ticks_us = (cur_tick_us - protura->fps_start_us);

            /* Ticks per frame */
            ticks_us = ticks_us / protura->fps_count;

            uint64_t frames_per_tick_ms = 1000000000ULL / ticks_us;

            printf("Ticks per frame: %lld, frames_per_tick_ms: %lld, FPS: %d.%03d\n", ticks_us, frames_per_tick_ms, (int)(frames_per_tick_ms / 1000), (int)(frames_per_tick_ms % 1000));

            protura->fps_count = 0;
            protura->fps_start_us = cur_tick_us;
        }
    }
}

static void gb_protura_get_keystate(struct gb_emu *emu, struct gb_keypad *keys)
{
    struct gb_display_protura *protura = container_of(emu->gpu.display, struct gb_display_protura, gb_disp);
    struct kern_event event_buffer[20];
    ssize_t ret;

    while (1) {
        ret = read(protura->event_fd, event_buffer, sizeof(event_buffer));
        if (ret == -1)
            break;

        int event_count = ret / sizeof(struct kern_event);
        int i;

        for (i = 0; i < event_count; i++) {
            if (event_buffer[i].type != KERN_EVENT_KEYBOARD)
                continue;

            switch (event_buffer[i].code) {
            case KS_Z:
                keys->key_a = event_buffer[i].value == KERN_EVENT_KEY_RELEASE;
                break;

            case KS_X:
                keys->key_b = event_buffer[i].value == KERN_EVENT_KEY_RELEASE;
                break;

            case KS_UP:
                keys->key_up = event_buffer[i].value == KERN_EVENT_KEY_RELEASE;
                break;

            case KS_DOWN:
                keys->key_down = event_buffer[i].value == KERN_EVENT_KEY_RELEASE;
                break;

            case KS_LEFT:
                keys->key_left = event_buffer[i].value == KERN_EVENT_KEY_RELEASE;
                break;

            case KS_RIGHT:
                keys->key_right = event_buffer[i].value == KERN_EVENT_KEY_RELEASE;
                break;

            case KS_ENTER:
                keys->key_start = event_buffer[i].value == KERN_EVENT_KEY_RELEASE;
                break;

            case KS_RSHIFT:
                keys->key_select = event_buffer[i].value == KERN_EVENT_KEY_RELEASE;
                break;

            case KS_Q:
                emu->stop_emu = 1;
                emu->reason = GB_EMU_STOP;
                break;

            case KS_P:
                if (event_buffer[i].value == KERN_EVENT_KEY_RELEASE) {
                    protura->cur_palette = (protura->cur_palette + 1) % GB_PALETTES;
                    emu->gpu.display->dmg_theme = gb_palettes[protura->cur_palette];
                }
                break;
            }
        }
    }
}

struct gb_gpu_display *gb_protura_driver_get_gb_gpu_display(struct gb_protura_driver *driver)
{
    return &driver->disp.gb_disp;
}

struct gb_protura_driver *gb_protura_driver_new(void)
{
    struct gb_protura_driver *driver = malloc(sizeof(*driver));
    memset(driver, 0, sizeof(*driver));

    driver->disp.gb_disp.disp_buf = gb_protura_display;
    driver->disp.gb_disp.get_keystate = gb_protura_get_keystate;

    /* This keeps keyboard presses from being sent to the active TTY while the
     * emulator is running */
    int err = ioctl(STDOUT_FILENO, TIOSETKBD, TTY_KEYBOARD_STATE_OFF);
    if (err) {
        printf("ioctl TIOSETKBD: %d\n", errno);
        return NULL;
    }

    driver->disp.fd0_fd = open("/dev/fb0", O_RDWR);
    if (driver->disp.fd0_fd == -1) {
        printf("Unable to open /dev/fb0: %s\n", strerror(errno));
        return NULL;
    }

    err = ioctl(driver->disp.fd0_fd, FB_IO_GET_DIMENSION, &driver->disp.dim);
    if (err) {
        printf("ioctl dimension: %d\n", errno);
        return NULL;
    }

    int scale_y = driver->disp.dim.height / GB_SCREEN_HEIGHT;
    int scale_x = driver->disp.dim.width / GB_SCREEN_WIDTH;

    driver->disp.scale = (scale_x < scale_y)? scale_x: scale_y;

    int total_height = GB_SCREEN_HEIGHT * driver->disp.scale;
    int total_width = GB_SCREEN_WIDTH * driver->disp.scale;

    driver->disp.offset_y = (driver->disp.dim.height - total_height) / 2;
    driver->disp.offset_x = (driver->disp.dim.width - total_width) / 2;

    /* This mapps the framebuffer memory into our memory space
     * (We don't yet have a proper userspace mmap) */
    err = ioctl(driver->disp.fd0_fd, FB_IO_MAP_FRAMEBUFFER, &driver->disp.framebuffer);
    if (err) {
        printf("ioctl framebuffer: %d\n", errno);
        return NULL;
    }

    /* This feeds us keyboard events according to the kernel event API */
    driver->disp.event_fd = open("/dev/keyboard", O_RDONLY | O_NONBLOCK);
    if (driver->disp.event_fd == -1) {
        printf("Unable to open /dev/keyboard: %s\n", strerror(errno));
        return NULL;
    }

    /* This both clears the screen and stops the kernel from writing to the
     * framebuffer to display the console */
    err = ioctl(driver->disp.fd0_fd, FB_IO_BLANK_SCREEN, 1);
    if (err) {
        printf("ioctl framebuffer: Unable to blank screen: %d\n", errno);
        return NULL;
    }

    return driver;
}

void gb_protura_driver_destroy(struct gb_protura_driver *driver)
{
    /* This turns the keyboard and display of the console back on. */
    int err = ioctl(STDOUT_FILENO, TIOSETKBD, TTY_KEYBOARD_STATE_ON);
    if (err)
        printf("ioctl tty: Unable to reenable keyboar!!! Err: %d\n", errno);

    err = ioctl(driver->disp.fd0_fd, FB_IO_BLANK_SCREEN, 0);
    if (err)
        printf("ioctl framebuffer: Unable to unblank screen!!!! Err: %d\n", errno);

    close(driver->disp.fd0_fd);
    close(driver->disp.event_fd);
    free(driver);
}
