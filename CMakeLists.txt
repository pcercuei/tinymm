cmake_minimum_required(VERSION 3.0)
project(tinymm LANGUAGES C VERSION 0.1)

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
		"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
endif()

add_library(${PROJECT_NAME} tinymm.c tinymm.h)
set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	PUBLIC_HEADER tinymm.h
	C_STANDARD 90
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)

option(WITH_THREADS "Thread-safe code" ON)
if (WITH_THREADS)
	find_library(PTHREAD_LIBRARIES pthread REQUIRED)
	find_path(PTHREAD_INCLUDE_DIR pthread.h REQUIRED)

	include_directories(${PTHREAD_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${PTHREAD_LIBRARIES})
	target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_THREADS)
endif (WITH_THREADS)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
