cmake_minimum_required(VERSION 3.28)
project(Rougetsu)

set(CMAKE_CXX_STANDARD 20)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#include(raylib)
include(glfw)
include(glad)
include(glm)

glad_add_library(glad_gl_core_mx_33 REPRODUCIBLE MX API gl:core=3.3)

add_executable(Rougetsu 
        main.cpp
        common/packfile.cpp
        
        # Graphics
        graph3d/gra3dSGD.cpp
        graph3d/sgd_types.h
        graph3d/g3dDebug.cpp
        graph3d/gra3d.cpp
        graph3d/g3dCore.cpp
        graph3d/gra3dDma.cpp
        graph3d/gra3dShadow.cpp
        graph3d/gra3dSGDData.cpp
        graph3d/g3dUtil.h
        utils.cpp
        utils.h
        sce/libvu0.cpp
        sce/libvu0.h
        render/renderer.cpp
        render/renderer.h
        render/Vu1Mem.cpp
        render/Vu1Mem.h
        graph3d/g3d_types.h
)

target_link_libraries(Rougetsu glad_gl_core_mx_33 glfw glm::glm)
