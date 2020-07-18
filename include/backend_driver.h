#ifndef INCLUDE_BACKEND_DRIVER_H
#define INCLUDE_BACKEND_DRIVER_H

#if defined(GBEMUC_BACKEND_SDL)
#include "sdl_driver.h"

typedef struct gb_sdl_driver gb_backend_driver;

#define gb_backend_driver_new() gb_sdl_driver_new()
#define gb_backend_driver_destroy(driver) gb_sdl_driver_destroy((driver))

#define gb_backend_get_gpu_display(driver) gb_sdl_driver_get_gb_gpu_display((driver))
#define gb_backend_get_apu_sound(driver)   gb_sdl_driver_get_gb_apu_sound((driver))

#else
#error "No gbemuc backend setting!"
#endif

#endif
