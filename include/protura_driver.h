#ifndef INCLUDE_PROTURA_DRIVER_H
#define INCLUDE_PROTURA_DRIVER_H

#include "gb/sound.h"
#include "gb/gpu.h"

struct gb_protura_driver;

struct gb_protura_driver *gb_protura_driver_new(void);
void gb_protura_driver_destroy(struct gb_protura_driver *);

struct gb_gpu_display *gb_protura_driver_get_gb_gpu_display(struct gb_protura_driver *);

#endif
