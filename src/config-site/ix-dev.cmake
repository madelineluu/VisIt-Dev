#/home/users/hank/third_party/cmake/3.24.3/linux-x86_64_gcc-11.4/bin/cmake
##
## ./build_visit3_4_0 generated host.cmake
## created: Wed May  6 09:42:16 PM PDT 2026
## system: Linux ix-dev 5.15.0-173-generic #183-Ubuntu SMP Fri Mar 6 13:29:34 UTC 2026 x86_64 x86_64 x86_64 GNU/Linux
## by: hank

##
## Setup VISITHOME & VISITARCH variables.
##
SET(VISITHOME /home/users/hank/third_party)
SET(VISITARCH linux-x86_64_gcc-11.4)

##SET(CMAKE_EXE_LINKER_FLAGS
##"-L/home/users/hank/third_party/osmesa/17.3.9/linux-x86_64_gcc-11.4/lib \
##-L/home/users/hank/third_party/llvm/6.0.1/linux-x86_64_gcc-11.4/lib \
##-Wl,-rpath,/home/users/hank/third_party/osmesa/17.3.9/linux-x86_64_gcc-11.4/lib \
##-Wl,-rpath,/home/users/hank/third_party/llvm/6.0.1/linux-x86_64_gcc-11.4/lib"
##CACHE STRING "" FORCE)


## Compiler flags.
##
VISIT_OPTION_DEFAULT(VISIT_C_COMPILER gcc TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_CXX_COMPILER g++ TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_FORTRAN_COMPILER no TYPE FILEPATH)
VISIT_OPTION_DEFAULT(VISIT_C_FLAGS " -m64 -fPIC -fvisibility=hidden" TYPE STRING)
VISIT_OPTION_DEFAULT(VISIT_CXX_FLAGS " -m64 -fPIC -fvisibility=hidden" TYPE STRING)

##
## VisIt Thread Option
##
VISIT_OPTION_DEFAULT(VISIT_THREAD OFF TYPE BOOL)

##############################################################
##
## Database reader plugin support libraries
##
## The HDF5 and NetCDF libraries must be first so that
## their libdeps are defined for any plugins that need them.
##
## For libraries with LIBDEP settings, order matters.
## Libraries with LIBDEP settings that depend on other
## Library's LIBDEP settings must come after them.
##############################################################
##

##
## ZLIB
##
SETUP_APP_VERSION(ZLIB 1.2.11)
VISIT_OPTION_DEFAULT(VISIT_ZLIB_DIR ${VISITHOME}/zlib/${ZLIB_VERSION}/${VISITARCH})

##
## Python
##
VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR ${VISITHOME}/python/3.7.7/${VISITARCH})

##
## Qt
##
SETUP_APP_VERSION(QT 5.14.2)
VISIT_OPTION_DEFAULT(VISIT_QT_DIR ${VISITHOME}/qt/${QT_VERSION}/${VISITARCH})

##
## QWT
##
SETUP_APP_VERSION(QWT 6.1.2)
VISIT_OPTION_DEFAULT(VISIT_QWT_DIR ${VISITHOME}/qwt/${QWT_VERSION}/${VISITARCH})

##
## LLVM
##
VISIT_OPTION_DEFAULT(VISIT_LLVM_DIR ${VISITHOME}/llvm/6.0.1/${VISITARCH})

##
## OSMesa
##
VISIT_OPTION_DEFAULT(VISIT_OSMESA_DIR ${VISITHOME}/osmesa/17.3.9/${VISITARCH})

##
## VTK
##
SETUP_APP_VERSION(VTK 8.1.0)
VISIT_OPTION_DEFAULT(VISIT_VTK_DIR ${VISITHOME}/vtk/${VTK_VERSION}/${VISITARCH})
VISIT_OPTION_DEFAULT(VISIT_VTK_INCDEP ZLIB_INCLUDE_DIR)
VISIT_OPTION_DEFAULT(VISIT_VTK_LIBDEP ZLIB_LIBRARY)

set(HAVE_OSMESA true CACHE BOOL "Have OSMesa library" FORCE)
set(VISIT_USE_X OFF CACHE BOOL "Use X11" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS
    "-L/home/users/hank/third_party/osmesa/17.3.9/linux-x86_64_gcc-11.4/lib \
     -L/home/users/hank/third_party/llvm/6.0.1/linux-x86_64_gcc-11.4/lib \
     -L/home/users/hank/third_party/vtk/8.1.0/linux-x86_64_gcc-11.4/lib \
     -Wl,-rpath,/home/users/hank/third_party/osmesa/17.3.9/linux-x86_64_gcc-11.4/lib \
     -Wl,-rpath,/home/users/hank/third_party/llvm/6.0.1/linux-x86_64_gcc-11.4/lib \
     -Wl,-rpath,/home/users/hank/third_party/vtk/8.1.0/linux-x86_64_gcc-11.4/lib"
    CACHE STRING "" FORCE)
