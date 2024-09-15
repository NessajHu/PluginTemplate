cmake_minimum_required(VERSION 3.20)

project(Utils)

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME}
    utilsglobal.h
    hostinfo.h
    hostinfo.cpp
    stringutils.h
    stringutils.cpp
    algorithm.h
    settings.h
    settings.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        UTILS_LIBRARY
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
)
