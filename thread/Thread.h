//
// Created by root on 17-11-1.
//

#ifndef MPP_LINUX_C_THREAD_H
#define MPP_LINUX_C_THREAD_H

extern "C" {
#include <stdio.h>
#include <pthread.h>
};

typedef void *(*func)(void *);

class Thread {
public:
    Thread(func fun, void *args, void **ret);
    ~Thread();
    int init();
    void run();
private:
    pthread_t mHandle;
    void *mArgs;
    void **mRet;
    void *(*mFun)(void *args);
};


#endif //MPP_LINUX_C_THREAD_H
