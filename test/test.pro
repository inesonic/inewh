##-*-makefile-*-########################################################################################################
# Copyright 2016 Inesonic, LLC
#
# MIT License:
#   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
#   documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
#   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
#   permit persons to whom the Software is furnished to do so, subject to the following conditions:
#   
#   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
#   Software.
#   
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
#   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
#   OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
#   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
########################################################################################################################

########################################################################################################################
# Basic build characteristics
#

TEMPLATE = app
QT += core testlib gui widgets network
CONFIG += testcase c++14

HEADERS = application_wrapper.h \
          test_web_hook.h \

SOURCES = test_inewh.cpp \
          application_wrapper.cpp \
          test_web_hook.cpp \

########################################################################################################################
# Libraries
#

INEWH_BASE = $${OUT_PWD}/../inewh
INCLUDEPATH += $${PWD}/../inewh/include

INCLUDEPATH += $${INECRYPTO_INCLUDE}
INCLUDEPATH += $${BOOST_INCLUDE}

unix {
    CONFIG(debug, debug|release) {
        LIBS += -L$${INEWH_BASE}/build/debug/ -linewh
        PRE_TARGETDEPS += $${INEWH_BASE}/build/debug/libinewh.a
    } else {
        LIBS += -L$${INEWH_BASE}/build/release/ -linewh
        PRE_TARGETDEPS += $${INEWH_BASE}/build/release/libinewh.a
    }

    LIBS += -L$${INECRYPTO_LIBDIR} -linecrypto
}

win32 {
    CONFIG(debug, debug|release) {
        LIBS += $${INEWH_BASE}/build/Debug/inewh.lib
        PRE_TARGETDEPS += $${INEWH_BASE}/build/Debug/inewh.lib
    } else {
        LIBS += $${INEWH_BASE}/build/Release/inewh.lib
        PRE_TARGETDEPS += $${INEWH_BASE}/build/Release/inewh.lib
    }

    LIBS += $${INECRYPTO_LIBDIR}/inecrypto.lib
}

########################################################################################################################
# Locate build intermediate and output products
#

TARGET = test_inewh

CONFIG(debug, debug|release) {
    unix:DESTDIR = build/debug
    win32:DESTDIR = build/Debug
} else {
    unix:DESTDIR = build/release
    win32:DESTDIR = build/Release
}

OBJECTS_DIR = $${DESTDIR}/objects
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/rcc
UI_DIR = $${DESTDIR}/ui
