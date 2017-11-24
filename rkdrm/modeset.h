//
// Created by root on 17-11-7.
//

#ifndef MPP_LINUX_C_MODESET_H
#define MPP_LINUX_C_MODESET_H

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm/drm_fourcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bo.h"
#include "dev.h"

struct sp_dev;
struct sp_crtc;

int initialize_screens(struct sp_dev *dev);

struct sp_plane* get_sp_plane(struct sp_dev *dev, struct sp_crtc *crtc);
void put_sp_plane(struct sp_plane *plane);

int set_sp_plane(struct sp_dev *dev, struct sp_plane *plane,
                 struct sp_crtc *crtc, int x, int y);

#ifdef USE_ATOMIC_API
int set_sp_plane_pset(struct sp_dev *dev, struct sp_plane *plane,
		      drmModePropertySetPtr pset, struct sp_crtc *crtc, int x, int y);
#endif

#endif //MPP_LINUX_C_MODESET_H
