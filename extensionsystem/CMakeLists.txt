cmake_minimum_required(VERSION 3.20)

project(ExtensionSystem)

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME}
    pluginspecification.h
    pluginspecification.cpp
    iplugin.h
    iplugin.cpp
    extensionsystemglobal.h
    extensionsystemtr.h
    pluginmanager.h
    pluginmanager.cpp
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        EXTENSIONSYSTEM_LIBRARY
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Utils
)
