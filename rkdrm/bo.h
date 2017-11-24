/*
 * Copyright 2016 Rockchip Electronics S.LSI Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef MPP_LINUX_C_BO_H
#define MPP_LINUX_C_BO_H

#include <drm/drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>

#include "dev.h"

#undef USE_ATOMIC_API
struct sp_dev;

struct sp_bo {
    struct sp_dev *dev;

    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t bpp;
    uint32_t format;
    uint32_t flags;

    uint32_t fb_id;
    uint32_t handle;
    void *map_addr;
    uint32_t pitch;
    uint32_t size;
};

int add_fb_sp_bo(struct sp_bo *bo, uint32_t format);
struct sp_bo* create_sp_bo(struct sp_dev *dev, uint32_t width, uint32_t height,
                           uint32_t depth, uint32_t bpp, uint32_t format, uint32_t flags);

void fill_bo(struct sp_bo *bo, uint8_t a, uint8_t r, uint8_t g, uint8_t b);
void draw_rect(struct sp_bo *bo, uint32_t x, uint32_t y, uint32_t width,
               uint32_t height, uint8_t a, uint8_t r, uint8_t g, uint8_t b);

void free_sp_bo(struct sp_bo *bo);

#endif //MPP_LINUX_C_BO_H
