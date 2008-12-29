# cmake build file for QuikGrid
# (c) by Stefan Roettger

PROJECT(QuikGrid)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF (UNIX)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O")
ENDIF (UNIX)

SET(QUIKGRID_HDRS
  contour.h
  gridxtyp.h
  scatdata.h
  surfgrid.h
  xpand.h
  )

SET(QUIKGRID_SRCS
  contour.cpp
  gridxtyp.cpp
  scatdata.cpp
  surfgrid.cpp
  xpand.cpp
  )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(quikgrid ${QUIKGRID_SRCS})

INSTALL(
   TARGETS quikgrid
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
   )

INSTALL(
   FILES ${QUIKGRID_HDRS}
   DESTINATION include
   )
