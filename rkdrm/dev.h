//
// Created by root on 17-11-7.
//

#ifndef MPP_LINUX_C_DEV_H
#define MPP_LINUX_C_DEV_H

#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <drm/drm.h>
#include <drm/drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "bo.h"
#include "modeset.h"

struct sp_bo;
struct sp_dev;

struct sp_plane {
    struct sp_dev *dev;
    drmModePlanePtr plane;
    struct sp_bo *bo;
    int in_use;
    uint32_t format;

    /* Property ID's */
    uint32_t crtc_pid;
    uint32_t fb_pid;
    uint32_t zpos_pid;
    uint32_t crtc_x_pid;
    uint32_t crtc_y_pid;
    uint32_t crtc_w_pid;
    uint32_t crtc_h_pid;
    uint32_t src_x_pid;
    uint32_t src_y_pid;
    uint32_t src_w_pid;
    uint32_t src_h_pid;
};

struct sp_crtc {
    drmModeCrtcPtr crtc;
    int pipe;
    int num_planes;
    struct sp_bo *scanout;
};

struct sp_dev {
    int fd;

    int num_connectors;
    drmModeConnectorPtr *connectors;

    int num_encoders;
    drmModeEncoderPtr *encoders;

    int num_crtcs;
    struct sp_crtc *crtcs;

    int num_planes;
    struct sp_plane *planes;
};

int is_supported_format(struct sp_plane *plane, uint32_t format);
struct sp_dev* create_sp_dev(void);
void destroy_sp_dev(struct sp_dev *dev);

#endif //MPP_LINUX_C_DEV_H
