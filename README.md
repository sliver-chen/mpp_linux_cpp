# MPP_LINUX_C++ - an amazing project
The given demo presents, how to use mpp on linux
if you want to use mpp on android,kindly use MediaCodec directly(hardware codec has been embedded in MediaCodec).    

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
firstly please modify CMakeLists.txt to specified c and c++ compiler.  
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

