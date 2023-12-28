# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(pdfviewer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/pdfwidgets/pdfviewer")

# set(PODOFO_BUILD_LIB_ONLY TRUE CACHE BOOL "" FORCE)
# set(PODOFO_BUILD_STATIC TRUE CACHE BOOL "" FORCE)
# add_subdirectory(podofo)

# include_directories(C:/Qt/Examples/Qt-6.6.1/pdfwidgets/pdfviewer/podofo_lib/src)
# link_directories(C:/Qt/Examples/Qt-6.6.1/pdfwidgets/pdfviewer/podofo_lib/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/podofo_lib/src)
include_directories(
        C:/Qt_Plugins/podofo-0.10.3/podofo-0.10.3/src
        C:/Qt_Plugins/podofo-0.10.3/build-win-Release/src/podofo
)

# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/podofo_lib/lib)

find_library(PODOFO_LIB NAMES podofo PATHS C:/Qt_Plugins/podofo-0.10.3/build-win-Release/target/Debug NO_DEFAULT_PATH)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PdfWidgets)
find_package(Qt6 REQUIRED COMPONENTS AxContainer AxServer)

qt_add_executable(pdfviewerwidgets
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    searchresultdelegate.cpp searchresultdelegate.h
    zoomselector.cpp zoomselector.h
    resources.qrc
)

set_target_properties(pdfviewerwidgets PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(pdfviewerwidgets PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::PdfWidgets
    Qt6::AxContainer
    Qt6::AxServer
    ${PODOFO_LIB}
)

target_link_libraries(pdfviewerwidgets PRIVATE
   # podofo_private
   # podofo
   ${PODOFO_LIB}
)

# Resources:
set(resources_resource_files
    "images/zoom-fit-width.svgz"
    "images/zoom-in.svgz"
    "images/go-previous-view-page.svgz"
    "images/zoom-original.svgz"
    "images/go-previous-view.svgz"
    "images/go-next-view-page.svgz"
    "images/zoom-fit-best.svgz"
    "images/zoom-out.svgz"
    "images/zoom-previous.svgz"
    "images/document-open.svgz"
    "images/go-next-view.svgz"
    "images/go-down-search.svgz"
    "images/go-up-search.svgz"
)

qt_add_resources(pdfviewerwidgets "resources"
    PREFIX
        "/icons"
    FILES
        ${resources_resource_files}
)

install(TARGETS pdfviewerwidgets
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
