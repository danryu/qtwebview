# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QLinuxWebViewPlugin Plugin:
#####################################################################

# Find WebKitGtk
find_package(PkgConfig REQUIRED)
pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)

qt_internal_add_plugin(QLinuxWebViewPlugin
    OUTPUT_NAME qtwebview_linux
    PLUGIN_TYPE webview
    SOURCES
        qlinuxwebview.cpp qlinuxwebview_p.h
        qlinuxwebviewplugin.cpp
    INCLUDE_DIRECTORIES
        ${WEBKIT2GTK_INCLUDE_DIRS}
    LIBRARIES
        ${WEBKIT2GTK_LIBRARIES}
        Qt::Core
        Qt::Gui
        Qt::WebViewPrivate
    COMPILE_OPTIONS
        ${WEBKIT2GTK_CFLAGS_OTHER}
)

# Set library directories for WebKitGtk
if(WEBKIT2GTK_LIBRARY_DIRS)
    target_link_directories(QLinuxWebViewPlugin PRIVATE ${WEBKIT2GTK_LIBRARY_DIRS})
endif()