/*
 * linux mpp c++ performance avaluation.
 * base in mpp & rga & sdl.
 * cfg info writed in params[6].
 */

#include <iostream>
#include "rkrga/RGA.h"
#include "mpp/Codec.h"
#include "thread/Thread.h"

extern "C" {
#include <stdlib.h>
#include <signal.h>
#include "rk_mpi.h"
}

#define dbg(fmt,...)\
            do {\
                printf(fmt,##__VA_ARGS__);\
            } while(0)

using namespace std;

struct param {
    int display;
    int width;
    int height;
    string input;
    string output;
    MppCodingType type;
};

/*
 * specified res file info here
 */
param params[1] = {
    {1, 1920, 1080, "../res/Tennis1080p.h264", "../res/Tennis1080p.yuv", MPP_VIDEO_CodingAVC},
};

void *thread_exec(void *args) {
    int ret = 0;
    param *arg = (param *)args;
    double *retRate = new double;

    Codec *ctx = new Codec();
    ret = ctx->init(arg->input.c_str(), arg->output.c_str(), arg->type,
                    arg->width, arg->height, arg->display);
    if (ret < 0) {
        cout << "failed to init codec" << endl;
        return NULL;
    }

    ret = ctx->decode();
    if (ret <0)
        cout << "codec decode exec failed" << endl;
    else
        cout << "codec decode exec success id:" << endl;

    ctx->deinit();
    delete ctx;

    return NULL;
}

void stop(int signo) {
    cout << "mpp_linux_demo exit";
    _exit(0);
}

int main() {
    int ret = 0;
    signal(SIGINT, stop);

    Thread *tFirst = new Thread(thread_exec, &params[0], NULL);
    tFirst->init();

    tFirst->run();

    delete(tFirst);
    return 0;
}
