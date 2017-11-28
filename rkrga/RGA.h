//
// Created by root on 17-11-2.
//

#ifndef MPP_LINUX_C_RGA_H
#define MPP_LINUX_C_RGA_H

#define RK_RGA_DEV	"/dev/v4l/by-path/platform-ff680000.rga-video-index0"

extern "C" {
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
};

#define RGA_ALIGN(x, a)   (((x)+(a)-1)&~((a)-1))

class RGA {
public:
    RGA();
    ~RGA();
    int init(int srcW, int srcH, int dstW, int dstH);
    int swscale(int srcFrmFd, int dstFrmFd);
private:
    int chk_dev_cap();
    int set_src_fmt();
    int set_dst_fmt();
    int open_dev_strm();
    int dst_buf_map();
    int src_buf_map();
    int set_img_rotation(unsigned degree);

    int mFd;
    int mSrcW;
    int mSrcH;
    int mDstW;
    int mDstH;
    int mSrcLen;
};


#endif //MPP_LINUX_C_RGA_H
