##-*-cmake-*-###########################################################################################################
# Copyright 2016 - 2022 Inesonic, LLC
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

cmake_minimum_required(VERSION 3.16.3)
project(test LANGUAGES CXX)

enable_testing()

find_package(Qt5 COMPONENTS Core)
find_package(Qt5 COMPONENTS Network)
find_package(Qt5 COMPONENTS Test)

SET(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(test
               test_inewh.cpp
               application_wrapper.cpp
               test_web_hook.cpp
)
add_test(${PROJECT_NAME} ${PROJECT_NAME})

add_dependencies(${PROJECT_NAME} inewh)

target_include_directories(${PROJECT_NAME} PUBLIC "../inewh/include")
include_directories("../inewh/include")

find_path(INECRYPTO_INCLUDE
          REQUIRED
          NAMES crypto_hmac.h crypto_helpers.h
          PATHS /usr/include/ /usr/local/include/ /opt/include/
)

include_directories(${INECRYPTO_INCLUDE})

target_link_libraries(${PROJECT_NAME} inewh)
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Test)

find_library(INECRYPTO_LIB
             REQUIRED
             NAMES inecrypto
             PATHS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64 /opt/lib ${INECRYPTO_LIBDIR}
)

target_link_libraries(${PROJECT_NAME} ${INECRYPTO_LIB})
