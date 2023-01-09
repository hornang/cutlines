cmake_minimum_required(VERSION 3.11)

project(cutlines LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cutlines
    include/cutlines/cutlines.h
    src/cutlines.cpp
)

include(GenerateExportHeader)
generate_export_header(cutlines)

include(CMakePackageConfigHelpers)

target_include_directories(cutlines PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/CutlinesConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cutlines
)

install(TARGETS cutlines
    EXPORT CutlinesTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/cutlines/cutlines.h
        ${CMAKE_CURRENT_BINARY_DIR}/cutlines_export.h
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}/cutlines
)

install(
    EXPORT CutlinesTargets
    FILE CutlinesTargets.cmake
    NAMESPACE Cutlines::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cutlines
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/CutlinesConfig.cmake"
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/cutlines
)
