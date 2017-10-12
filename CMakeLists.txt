project(ANPR)
cmake_minimum_required(VERSION 2.8)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

find_package(OpenCV REQUIRED core imgproc video highgui features2d imgcodecs)
find_package(tesseract)
find_package(caffe)

################ Boost #################
set(Boost_USE_STATIC_LIBS        ON)
FIND_PACKAGE(Boost COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR} /usr/local/include /usr/local/cuda-8.0/include/ distribute/include distribute/src)
########################################
link_directories (${CMAKE_SOURCE_DIR}/libs /usr/local/lib /usr/local/cuda-8.0/lib64 ${CMAKE_SOURCE_DIR}/distribute/lib)

#add_definitions(-DCUDA -DCUDNN -DGPU -D_DEBUG)
file(GLOB DETECT_TEXT_SOURCES *.cpp *.c)
file(GLOB DETECT_TEXT_HEADERS *.hpp *.h)

aux_source_directory(. SRC_LIST)

add_executable(ANPR src/charExtraction.cpp src/charExtraction.h src/main.cpp src/segmentation.cpp src/segmentation.h src/TextDetection.cpp src/TextDetection.h src/charRecognition.h src/charRecognition.cpp distribute/src/arapaho.hpp distribute/src/arapaho.cpp)

target_link_libraries(ANPR
    ${Boost}
    darknet-cpp-shared
    ${OpenCV_LIBS}
    lept
    tesseract
    cuda
    glog
    caffe
    boost_system
    hdf5
    hdf5_serial
    hdf5_hl
    curand
    cudart
    cublas
   
)
