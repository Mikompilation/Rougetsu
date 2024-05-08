cmake_minimum_required(VERSION 3.28)
project(Rougetsu)

set(CMAKE_CXX_STANDARD 20)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(raylib)

add_executable(Rougetsu 
        main.cpp
        packfile.cpp
        
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
)

target_link_libraries(Rougetsu raylib)
