cmake_minimum_required(VERSION 3.1.3)
project(Errol)
enable_testing()

include(CTest)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY test)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)


if(MINGW)
	add_definitions(-DMINGW_HAS_SECURE_API)
endif()

if(NOT MSVC)
	add_definitions(-Wall)
	set(CMAKE_C_EXTENSIONS OFF)
	set(CMAKE_CXX_EXTENSIONS OFF)
else()
	foreach(flagvar CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
	    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		string(REGEX REPLACE "/MD" "/MT" ${flagvar} "${${flagvar}}")
		string(REGEX REPLACE "/MDd" "/MTd" ${flagvar} "${${flagvar}}")
	endforeach()
endif()

include_directories(lib ${DOUBLE_CONVERSION_INCLUDE_DIR})
link_directories(build ${DOUBLE_CONVERSION_LIBRARY})

file(GLOB errol_srcs lib/*.c)
file(GLOB tests_srcs test/*.c test/*.cpp)

add_library(errol ${errol_srcs})

if(BUILD_TESTING)
	find_package(DoubleConversion REQUIRED)
	find_package(GMP REQUIRED)
	include_directories(${DOUBLE_CONVERSION_INCLUDE_DIRS} ${GMP_INCLUDE_DIR})
	link_directories(${DOUBLE_CONVERSION_LIBRARIES} ${GMP_LIBRARIES})
	add_executable(run ${tests_srcs})
	target_link_libraries(run errol double-conversion gmp)
	add_test(testall tests/run)
endif()
