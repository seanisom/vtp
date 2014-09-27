# Defines
# MINI_FOUND
# MINI_INCLUDE_DIR
# MINI_LIBRARIES

set(MINI_FIND_DEBUG TRUE CACHE BOOL "Also search for the debug version of the MINI library")

if(MINI_FIND_DEBUG)
	if(MINI_INCLUDE_DIR AND MINI_LIBRARY AND MINI_LIBRARY_DEBUG)
		set(MINI_FIND_QUIETLY TRUE)
	endif(MINI_INCLUDE_DIR AND MINI_LIBRARY AND MINI_LIBRARY_DEBUG)
else(MINI_FIND_DEBUG)
	if(MINI_INCLUDE_DIR AND MINI_LIBRARY)
		set(MINI_FIND_QUIETLY TRUE)
	endif(MINI_INCLUDE_DIR AND MINI_LIBRARY)
endif(MINI_FIND_DEBUG)

find_path(MINI_INCLUDE_DIR mini/mini.h PATHS .. ../deps DOC "Directory containing mini/mini.h")

find_library(MINI_LIBRARY NAMES Mini libMini PATHS .. ../mini ../deps/mini DOC "Path to Mini library")
find_library(MINISFX_LIBRARY NAMES MiniSFX libMiniSFX PATHS .. ../mini ../deps/mini DOC "Path to MiniSFX library")
if(MINI_FIND_DEBUG)
	find_library(MINI_LIBRARY_DEBUG NAMES libMinid PATHS .. ../mini ../deps/mini DOC "Path to Mini debug library")
	find_library(MINISFX_LIBRARY_DEBUG NAMES libMiniSFX PATHS .. ../mini ../deps/mini DOC "Path to MiniSFX debug library")
endif(MINI_FIND_DEBUG)

# handle the QUIETLY and REQUIRED arguments and set MINI_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MINI DEFAULT_MSG MINI_LIBRARY MINISFX_LIBRARY MINI_INCLUDE_DIR)

if(MINI_FOUND)
	if(MINI_FIND_DEBUG)
		if(NOT MINI_LIBRARY_DEBUG)
			set(MINI_LIBRARY_DEBUG ${MINI_LIBRARY})
		endif(NOT MINI_LIBRARY_DEBUG)
		if(NOT MINISFX_LIBRARY_DEBUG)
			set(MINISFX_LIBRARY_DEBUG ${MINISFX_LIBRARY})
		endif(NOT MINISFX_LIBRARY_DEBUG)
		set(MINI_LIBRARIES debug ${MINI_LIBRARY_DEBUG} ${MINISFX_LIBRARY_DEBUG} optimized ${MINI_LIBRARY} ${MINISFX_LIBRARY})
	else(MINI_FIND_DEBUG)
		set(MINI_LIBRARIES ${MINI_LIBRARY} ${MINISFX_LIBRARY})
	endif(MINI_FIND_DEBUG)
else(MINI_FOUND)
	SET(MINI_LIBRARIES )
endif(MINI_FOUND)
