//
// Created by root on 17-11-2.
//

#include <linux/videodev2.h>
#include "RGA.h"

#define rga_log(fmt,...)\
            do {\
                printf(fmt,##__VA_ARGS__);\
            } while(0)

RGA::RGA() : mFd(0),
             mDstH(0),
             mDstW(0),
             mSrcW(0),
             mSrcH(0),
             mSrcLen(0)
{

}

RGA::~RGA() {

}

int RGA::chk_dev_cap() {
    struct v4l2_capability cap = {0};
    int ret = 0;

    ret = ioctl(mFd, VIDIOC_QUERYCAP, &cap);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_QUERYCAP.\n");
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_M2M)) {
        rga_log("dev don't support VIDEO_M2M.\n");
        return -2;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        rga_log("dev don't support V4L2_CAP_STREAMING.\n");
        return -3;
    }

    return 0;
}

int RGA::set_src_fmt() {
    struct v4l2_format fmt;
    int ret = 0;

    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width  = RGA_ALIGN(mSrcW, 16);
    fmt.fmt.pix.height = RGA_ALIGN(mSrcH, 16);
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    ret = ioctl(mFd, VIDIOC_S_FMT, &fmt);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIO_S_FMT.\n");
        return -1;
    }

    return 0;
}

int RGA::set_dst_fmt() {
    struct v4l2_format fmt;
    int ret = 0;

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width  = RGA_ALIGN(mDstW, 16);
    fmt.fmt.pix.height = RGA_ALIGN(mDstH, 16);
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    ret = ioctl(mFd, VIDIOC_S_FMT, &fmt);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_S_FMT.\n");
        return -1;
    }

    return 0;
}

int RGA::open_dev_strm() {
    enum v4l2_buf_type type;
    int ret = 0;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(mFd, VIDIOC_STREAMON, &type);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_STREAMON for %d %s.\n",
                errno, strerror(errno));
        return -1;
    }

    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = ioctl(mFd, VIDIOC_STREAMON, &type);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_STREAMON for %d %s.\n",
                errno, strerror(errno));
        return -2;
    }

    return 0;
}

int RGA::src_buf_map() {
    struct v4l2_buffer buf;
    struct v4l2_requestbuffers reqbuf;
    int nb_src_bufs;
    int ret = 0;
    int i = 0;

    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.count = 5;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    reqbuf.memory = V4L2_MEMORY_DMABUF;

    ret = ioctl(mFd, VIDIOC_REQBUFS, &reqbuf);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_REQBUFS for %d %s.\n", errno, strerror(errno));
        return -1;
    }

    nb_src_bufs = reqbuf.count;
    rga_log("get dma buffer nb:%d.\n", nb_src_bufs);
    for (i = 0; i < nb_src_bufs; i++) {
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf.memory = V4L2_MEMORY_DMABUF;
        buf.index = i;

        ret = ioctl(mFd, VIDIOC_QUERYBUF, &buf);
        if (ret != 0) {
            rga_log("failed to ioctl VIDIOC_QUERYBUF.\n");
            return -2;
        }

    }
    mSrcLen = buf.length;

    return 0;
}

int RGA::dst_buf_map() {
    struct v4l2_buffer buf;
    struct v4l2_requestbuffers reqbuf;
    int ret = 0;
    int nb_dst_bufs;
    int i = 0;

    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.count = 5;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_DMABUF;

    ret = ioctl(mFd, VIDIOC_REQBUFS, &reqbuf);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_REQBUFS for %d %s.\n", errno, strerror(errno));
        return -1;
    }

    nb_dst_bufs = reqbuf.count;
    rga_log("get dma buffer nb:%d.\n", nb_dst_bufs);
    for (i = 0; i < nb_dst_bufs; i++) {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_DMABUF;
        buf.index = i;

        ret = ioctl(mFd, VIDIOC_QUERYBUF, &buf);
        if (ret != 0) {
            rga_log("failed to ioctl VIDIOC_QUERYBUF.\n");
            return -2;
        }

    }

    return 0;
}

int RGA::set_img_rotation(unsigned degree) {
    struct v4l2_control ctrl = {0};
    int ret = 0;

    ctrl.id = V4L2_CID_ROTATE;
    ctrl.value = degree;
    ret = ioctl(mFd, VIDIOC_S_CTRL, &ctrl);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_S_CTRL.\n");
        return -1;
    }
    return 0;
}

int RGA::init(int srcW, int srcH, int dstW, int dstH) {
    int ret = 0;

    mSrcW = srcW;
    mSrcH = srcH;
    mDstW = dstW;
    mDstH = dstH;

    mFd = open(RK_RGA_DEV, O_RDWR);
    if (mFd < 0) {
        rga_log("failed to open rga dev %s.\n", RK_RGA_DEV);
        return -1;
    }

    ret = chk_dev_cap();
    if (ret < 0) {
        rga_log("failed to exec chk_dev_cap %d.\n", ret);
        return -2;
    }

    ret = set_src_fmt();
    if (ret < 0) {
        rga_log("failed to exec set_src_fmt %d.\n", ret);
        return -3;
    }

    ret = set_dst_fmt();
    if (ret < 0) {
        rga_log("failed to exec set_dst_fmt %d.\n", ret);
        return -4;
    }

    ret = set_img_rotation(90);
    if (ret < 0) {
        rga_log("failed to exec set_img_rotation %d.\n", ret);
        return -5;
    }

    ret = src_buf_map();
    if (ret < 0) {
        rga_log("failed to exec src_buf_map %d.\n", ret);
        return -6;
    }

    ret = dst_buf_map();
    if (ret < 0) {
        rga_log("failed to exec dst_buf_map %d.\n", ret);
        return -7;
    }

    ret = open_dev_strm();
    if (ret < 0) {
        rga_log("failed to exec open_dev_strm %d.\n", ret);
        return -8;
    }

    return 0;
}

int RGA::swscale(int srcFrmFd, int dstFrmFd) {
    struct v4l2_buffer buf = {0};
    int ret = 0;

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory = V4L2_MEMORY_DMABUF;
    buf.bytesused = mSrcLen;
    buf.index = 0;
    buf.m.fd = srcFrmFd;
    ret = ioctl(mFd, VIDIOC_QBUF, &buf);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_QBUF for %d %s.\n",
                errno, strerror(errno));
        return -1;
    }

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_DMABUF;
    buf.index = 0;
    buf.m.fd = dstFrmFd;
    ret = ioctl(mFd, VIDIOC_QBUF, &buf);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_QBUF for %d %s.\n",
                errno, strerror(errno));
        return -2;
    }

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory = V4L2_MEMORY_DMABUF;
    ret = ioctl(mFd, VIDIOC_DQBUF, &buf);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_DQBUF for %d %s.\n",
                errno, strerror(errno));
        return -3;
    }

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_DMABUF;
    ret = ioctl(mFd, VIDIOC_DQBUF, &buf);
    if (ret != 0) {
        rga_log("failed to ioctl VIDIOC_DQBUF for %d %s.\n",
                errno, strerror(errno));
        return -4;
    }

    return 0;
}