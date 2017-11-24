//
// Created by root on 17-11-1.
//

#include "Thread.h"

#define thread_dbg(fmt,...)\
            do {\
                printf(fmt,##__VA_ARGS__);\
            } while(0)

Thread::Thread(func func, void *args, void **ret) {
    mFun = func;
    mArgs = args;
    mRet = ret;
}

Thread::~Thread() {

}

int Thread::init() {
    int ret = 0;

    ret = pthread_create(&mHandle, NULL, mFun, mArgs);
    if (ret < 0) {
        thread_dbg("failed to create thread %d.\n", mHandle);
        return -1;
    }
}

void Thread::run() {
    pthread_join(mHandle, mRet);
    //pthread_detach(mHandle);
}