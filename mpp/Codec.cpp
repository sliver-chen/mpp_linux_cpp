//
// Created by root on 17-11-1.
//

#include "Codec.h"

#define cxx_log(fmt,...)\
            do {\
                printf(fmt,##__VA_ARGS__);\
            } while(0)

using namespace std;

Codec::Codec() : mFps(0),
                 mEos(0),
                 mID(0),
                 mDisPlay(0),
                 mFrmCnt(0),
                 mFrmRate(0.0),
                 mFin(NULL),
                 mFout(NULL),
                 mPktBuf(NULL),
                 mCtx(NULL),
                 mApi(NULL),
                 mPkt(NULL),
                 mFrmGrp(NULL)
{

}

Codec::~Codec() {
    if (mPkt) {
        mpp_packet_deinit(&mPkt);
        mPkt = NULL;
    }

    if (mCtx) {
        mpp_destroy(mCtx);
        mCtx = NULL;
    }

    if (mPktBuf) {
        delete(mPktBuf);
        mPktBuf = NULL;
    }

    if (mBuffer) {
        mpp_buffer_put(mBuffer);
        mBuffer = NULL;
    }

    if (mFrmGrp) {
        mpp_buffer_group_put(mFrmGrp);
        mFrmGrp = NULL;
    }

    if (mFin) {
        fclose(mFin);
        mFin = NULL;
    }

    if (mFout) {
        fclose(mFout);
        mFout = NULL;
    }

    if (mRGA) {
        delete(mRGA);
        mRGA = NULL;
    }
}

int Codec::init(const char *file_input,
                const char *file_output, MppCodingType type,
                int src_w, int src_h, int display) {
    int ret = 0;
    int x, y, i;
    RK_U32 need_split = 1;
    MpiCmd mpi_cmd = MPP_CMD_BASE;
    MppParam param = NULL;

    mDisPlay = display;
    srcW = src_w;
    srcH = src_h;
    dstW = src_w;
    dstH = src_h;

    mFin = fopen(file_input, "rb");
    if (!mFin) {
        cxx_log("failed to open input file %s.\n", file_input);
        return -1;
    }

    mFout = fopen(file_output, "wb+");
    if (!mFout) {
        cxx_log("failed to open output file %s.\n", file_output);
        return -2;
    }

    mPktBuf = new char[PKT_SIZE];
    if (!mPktBuf) {
        cxx_log("failed to malloc mPktBuf.\n");
        return -3;
    }

    ret = mpp_packet_init(&mPkt, mPktBuf, PKT_SIZE);
    if (ret) {
        cxx_log("failed to exec mpp_packet_init ret %d", ret);
        return -4;
    }

    ret = mpp_create(&mCtx, &mApi);
    if (ret != MPP_OK) {
        cxx_log("failed to exec mpp_create ret %d", ret);
        return -5;
    }

    mpi_cmd = MPP_DEC_SET_PARSER_SPLIT_MODE;
    param = &need_split;
    ret = mApi->control(mCtx, mpi_cmd, param);
    if (ret != MPP_OK) {
        cxx_log("failed to control MPP_DEC_SET_PARSER_SPLIT_MODE.\n");
        return  -6;
    }

    ret = mpp_init(mCtx, MPP_CTX_DEC, type);
    if (ret != MPP_OK) {
        cxx_log("failed to exec mpp_init.\n");
        return -7;
    }

    mRGA = new RGA();
    ret = mRGA->init(srcW, srcH, srcW, srcH);
    if (ret < 0) {
        cxx_log("failed to exec mRGA->init %d.\n", ret);
        return -8;
    }

    mDev = create_sp_dev();
    if (!mDev) {
        cxx_log("failed to exec create_sp_dev.\n");
        return -10;
    }

    ret = initialize_screens(mDev);
    if (ret != 0) {
        cxx_log("failed to exec initialize_screens.\n");
        return -11;
    }

    mPlanes = (sp_plane **)calloc(mDev->num_planes, sizeof(*mPlanes));
    if (!mPlanes) {
        cxx_log("failed to calloc mPlanes.\n");
        return -12;
    }

    mCrtc = &mDev->crtcs[0];
    for (i = 0; i < mCrtc->num_planes; i++) {
        mPlanes[i] = get_sp_plane(mDev, mCrtc);
        if (is_supported_format(mPlanes[i], DRM_FORMAT_NV12))
            mTestPlane = mPlanes[i];
    }

    if (!mTestPlane) {
        cxx_log("failed to get mTestPlane.\n");
        return -13;
    }

    return 0;
}

static RK_S64 get_time() {
    struct timeval tv_date;
    gettimeofday(&tv_date, NULL);
    return (RK_S64)tv_date.tv_sec * 1000000 + (RK_S64)tv_date.tv_usec;
}

int Codec::dump_mpp_frame_to_file(MppFrame frame, FILE *fp)
{
    RK_U32 width = 0;
    RK_U32 height = 0;
    RK_U32 h_stride = 0;
    RK_U32 v_stride = 0;
    MppFrameFormat fmt = MPP_FMT_YUV420SP;
    MppBuffer buffer = NULL;
    RK_U8 *base = NULL;

    if (!fp || !frame)
        return -1;

    width = mpp_frame_get_width(frame);
    height = mpp_frame_get_height(frame);
    h_stride = mpp_frame_get_hor_stride(frame);
    v_stride = mpp_frame_get_ver_stride(frame);
    fmt = mpp_frame_get_fmt(frame);
    buffer = mpp_frame_get_buffer(frame);

    if(!buffer)
        return -2;

    base = (RK_U8 *)mpp_buffer_get_ptr(buffer);
    {
        RK_U32 i;
        RK_U8 *base_y = base;
        RK_U8 *base_u = base + h_stride * v_stride;
        RK_U8 *base_v = base_u + h_stride * v_stride / 4;

        for (i = 0; i < height; i++, base_y += h_stride)
            fwrite(base_y, 1, width, fp);
        for (i = 0; i < height / 2; i++, base_u += h_stride / 2)
            fwrite(base_u, 1, width / 2, fp);
        for (i = 0; i < height / 2; i++, base_v += h_stride / 2)
            fwrite(base_v, 1, width / 2, fp);
    }

    return 0;
}

int Codec::drm_show_frmae(MppFrame frame) {
    sp_bo *bo;
    uint32_t handles[4], pitches[4], offsets[4];
    int width, height;
    int frm_size, ret, fd, err;

    err = mpp_frame_get_errinfo(frame) |
          mpp_frame_get_discard(frame);
    if (err) {
        cxx_log("get err info %d discard %d,go back.\n",
                mpp_frame_get_errinfo(frame),
                mpp_frame_get_discard(frame));
        return -1;
    }


    width = mpp_frame_get_width(frame);
    height = mpp_frame_get_height(frame);
    width = CODEC_ALIGN(width, 16);
    height = CODEC_ALIGN(height, 16);
    frm_size = width * height * 3 / 2;
    fd = mpp_buffer_get_fd(mpp_frame_get_buffer(frame));

    bo = (struct sp_bo *)calloc(1, sizeof(struct sp_bo));
    if (!bo) {
        cxx_log("failed to calloc bo.\n");
        return -2;
    }

    drmPrimeFDToHandle(mDev->fd, fd, &bo->handle);
    bo->dev = mDev;
    bo->width = width;
    bo->height = height;
    bo->depth = 16;
    bo->bpp = 32;
    bo->format = DRM_FORMAT_NV12;
    bo->flags = 0;

    handles[0] = bo->handle;
    pitches[0] = width;
    offsets[0] = 0;
    handles[1] = bo->handle;
    pitches[1] = width;
    offsets[1] = width * height;
    ret = drmModeAddFB2(mDev->fd, bo->width, bo->height,
                        bo->format, handles, pitches, offsets,
                        &bo->fb_id, bo->flags);
    if (ret != 0) {
        cxx_log("failed to exec drmModeAddFb2.\n");
        return -3;
    }

    ret = drmModeSetPlane(mDev->fd, mTestPlane->plane->plane_id,
                          mCrtc->crtc->crtc_id, bo->fb_id, 0, 0, 0,
                          mCrtc->crtc->mode.hdisplay,
                          mCrtc->crtc->mode.vdisplay,
                          0, 0, bo->width << 16, bo->height << 16);
    if (ret) {
        cxx_log("failed to exec drmModeSetPlane.\n");
        return -3;
    }

    if (mTestPlane->bo) {
        if (mTestPlane->bo->fb_id) {
            ret = drmModeRmFB(mDev->fd, mTestPlane->bo->fb_id);
            if (ret)
                cxx_log("failed to exec drmModeRmFB.\n");
        }
        if (mTestPlane->bo->handle) {
            struct drm_gem_close req = {
                    .handle = mTestPlane->bo->handle,
            };

            drmIoctl(bo->dev->fd, DRM_IOCTL_GEM_CLOSE, &req);
        }
        free(mTestPlane->bo);
    }
    mTestPlane->bo = bo;

    return 0;
}

int Codec::decode_one_pkt(char *buf, int size, MppFrame *srcFrm, MppFrame *dstFrm) {
    int ret = 0;
    RK_U32 pkt_done = 0;

    mpp_packet_write(mPkt, 0, buf, size);
    mpp_packet_set_pos(mPkt, buf);
    mpp_packet_set_length(mPkt, size);

    if (mEos)
        mpp_packet_set_eos(mPkt);

    do {
        RK_S32 times = 5;

        if (!pkt_done) {
            ret = mApi->decode_put_packet(mCtx, mPkt);
            if (ret == MPP_OK)
                pkt_done = 1;
        }

        do {
            RK_S32 get_frm = 0;
            RK_U32 frm_eos = 0;

            try_again:
                ret = mApi->decode_get_frame(mCtx, srcFrm);
                if (ret == MPP_ERR_TIMEOUT) {
                    if (times > 0) {
                        times--;
                        usleep(2000);
                        goto try_again;
                    }
                    cxx_log("decode_get_frame failed too much time.\n");
                }

                if (ret != MPP_OK) {
                    cxx_log("decode_get_frame failed ret %d.\n", ret);
                    break;
                }

                if (*srcFrm) {
                    if (mpp_frame_get_info_change(*srcFrm)) {
                        RK_U32 width = mpp_frame_get_width(*srcFrm);
                        RK_U32 height = mpp_frame_get_height(*srcFrm);
                        RK_U32 hor_stride = mpp_frame_get_hor_stride(*srcFrm);
                        RK_U32 ver_stride = mpp_frame_get_ver_stride(*srcFrm);

                        cxx_log("decode_get_frame get info changed found.\n");
                        cxx_log("decoder require buffer w:h [%d:%d] stride [%d:%d]",
                                width, height, hor_stride, ver_stride);

                        ret = mpp_buffer_group_get_internal(&mFrmGrp, MPP_BUFFER_TYPE_DRM);
                        if (ret) {
                            cxx_log("get mpp buffer group failed ret %d.\n", ret);
                            break;
                        }

                        mApi->control(mCtx, MPP_DEC_SET_EXT_BUF_GROUP, mFrmGrp);
                        mApi->control(mCtx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
                    } else {
                        cxx_log("decode_get_frame ID:%d get srcFrm %d.\n", mID, mFrmCnt++);
                        if (mDisPlay) {
                            ret = mRGA->swscale(mpp_buffer_get_fd(mpp_frame_get_buffer(*srcFrm)),
                                                mpp_buffer_get_fd(mpp_frame_get_buffer(*dstFrm)));
                            if (ret < 0) {
                                cxx_log("failed to exec mRGA->swscale ret:%d.\n", ret);
                            } else {
                                drm_show_frmae(*dstFrm);
                                if (mFout) {
                                    /*
                                     * note that write file will leads to IO block
                                     * so if you want to test frame rate,don't wirte
                                     * it.
                                     */
                                    //dump_mpp_frame_to_file(*srcFrm, mFout);
                                }
                            }
                        }
                    }
                    frm_eos = mpp_frame_get_eos(*srcFrm);
                    mpp_frame_deinit(srcFrm);
                    *srcFrm = NULL;
                    get_frm = 1;
                }

                if (mEos && pkt_done && !frm_eos) {
                    usleep(10000);
                    continue;
                }

                if (frm_eos) {
                    cxx_log("found last frame.\n");
                    break;
                }

                if (get_frm)
                    continue;
                break;
        } while (1);

        if (pkt_done)
            break;
        /*
         * why sleep here?
         * mpp has a internal input pakcet list,if it is full, wait here 3ms to
         * wait internal list isn't full.
         */
        usleep(3000);
    } while (1);

    return 0;
}

int Codec::decode() {
    MppFrame srcFrm;
    MppFrame dstFrm;
    double timeDiff;
    int ret = 0;
    int hor_stride = CODEC_ALIGN(dstW, 16);
    int ver_srride = CODEC_ALIGN(dstH, 16);

    ret = mpp_frame_init(&dstFrm);
    if (ret) {
        cxx_log("failed to exec mpp_frame_init %d.\n", ret);
        return -1;
    }

    mpp_buffer_get(mFrmGrp, &mBuffer, hor_stride * ver_srride * 3 / 2);
    mpp_frame_set_width(dstFrm, dstW);
    mpp_frame_set_height(dstFrm, dstH);
    mpp_frame_set_hor_stride(dstFrm, hor_stride);
    mpp_frame_set_ver_stride(dstFrm, ver_srride);
    mpp_frame_set_fmt(dstFrm, MPP_FMT_YUV420P);
    mpp_frame_set_buffer(dstFrm, mBuffer);

    mTimeS = get_time();
    while (!mEos) {
        size_t read_size = fread(mPktBuf, 1, PKT_SIZE, mFin);
        if (read_size != PKT_SIZE || feof(mFin)) {
            cxx_log("found last packet.\n");
            mEos = 1;
        }

        mpp_frame_set_eos(dstFrm, mEos);

        ret = decode_one_pkt(mPktBuf, read_size, &srcFrm, &dstFrm);
        if (ret < 0) {
            cxx_log("failed to exec decode.\n");
            return -2;
        }
    }
    mpp_frame_deinit(&dstFrm);

    mTimeE = get_time();
    timeDiff = double(mTimeE - mTimeS)/1000;
    mFrmRate = (mFrmCnt * 1000) / timeDiff;
    cxx_log("decode frames %d using %.2fms frm rate:%.2f.\n", mFrmCnt,
            timeDiff, mFrmRate);

    return 0;
}

int Codec::deinit() {
    int ret = 0;

    ret = mApi->reset(mCtx);
    if (ret != MPP_OK) {
        cxx_log("failed to exec mApi->reset.\n");
        return -1;
    }
}
