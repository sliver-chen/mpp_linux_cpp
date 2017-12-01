# MPP_LINUX_C++
a demo shows that how to use mpp on linux
if you want to using mpp on android,you can refer that  
https://github.com/sliver-chen/mpp_android_demo

project architecture

├── build   --build directory  
├── CMakeLists.txt      --cmake script  
├── main.cpp            --main program  
├── mpp                 --mpp abstract interface  
├── README.md           --doc  
├── res                 --res directory  
├── rkdrm               --drm interface(abount display)  
├── rkrga               --rga interface(about format and resolution conversion)  
└── thread              --thread abstract interface(use posix)  

## make & test
first please modify CMakeLists.txt to specified c and c++ compiler.  
just do that  
    set(CMAKE_C_COMPILER "enter your toolchain gcc path)  
    set(CMAKE_CXX_coMPILER "enter your toolchain g++ path")  

cmake version >= 2.8 is required  
root:cd build  
root:make  
root:./mpp_linux_demo  

## how you will see
on your device screen,you will see that local avc file
is displayed.  

