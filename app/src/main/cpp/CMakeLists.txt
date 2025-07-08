# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("grpcjniclient")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
# 设置基础路径
set(GRPC_ROOT ${CMAKE_CURRENT_LIST_DIR}/grpc)
set(GRPC_LIB_DIR ${GRPC_ROOT}/lib)
set(GRPC_INCLUDE_DIR ${GRPC_ROOT}/include)

set(CORE_GRPC_LIBS
        ${GRPC_LIB_DIR}/libgrpc++.so
        ${GRPC_LIB_DIR}/libgrpc.so
        ${GRPC_LIB_DIR}/libgpr.so
        ${GRPC_LIB_DIR}/libprotobuf.so
        ${GRPC_LIB_DIR}/libaddress_sorting.so
        ${GRPC_LIB_DIR}/libre2.so
        ${GRPC_LIB_DIR}/libssl.so
        ${GRPC_LIB_DIR}/libcrypto.so
)

set(UTF8_LIBS
        ${GRPC_LIB_DIR}/libutf8_range_lib.so
        ${GRPC_LIB_DIR}/libutf8_validity.so
)

file(GLOB ABSL_LIBS "${GRPC_LIB_DIR}/libabsl_*.so")

file(GLOB UPB_LIBS "${GRPC_LIB_DIR}/libupb_*.so")

set(ALL_LIBS ${CORE_GRPC_LIBS} ${ABSL_LIBS} ${UPB_LIBS} ${UTF8_LIBS})

add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.

        helloworld.protoc.grpc.pb.cc
        helloworld.protoc.pb.cc
        rfcomm.protoc.pb.cc
        rfcomm.protoc.grpc.pb.cc

        native-lib.cpp)

include_directories(${GRPC_INCLUDE_DIR})

target_link_libraries(${CMAKE_PROJECT_NAME} ${ALL_LIBS})

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library

        android
        log)