set(EXE_NAME duidemo)


if (MSVC AND USE_STATIC_CRT)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif()


if (NOT BUILD_SHARED_LIBS)
	# Use ppxnet static library
	add_definitions(-DPPXBASE_STATIC -DUILIB_STATIC)
endif()

file(GLOB ROOT_SOURCE_FILES 			*.cpp)
list(FILTER ROOT_SOURCE_FILES EXCLUDE REGEX  ".*StdAfx.cpp$")

file(GLOB ROOT_HEADER_FILES 			*.h)
list(FILTER ROOT_HEADER_FILES EXCLUDE REGEX  ".*StdAfx.h$")

source_group("Header Files" FILES 
	${ROOT_HEADER_FILES} StdAfx.h)

source_group("Source Files" FILES 
	${ROOT_SOURCE_FILES} StdAfx.cpp)

add_executable(
	${EXE_NAME}
	WIN32
	${ROOT_HEADER_FILES} StdAfx.h
	${ROOT_SOURCE_FILES} StdAfx.cpp
	duidemo.rc
)

target_include_directories(${EXE_NAME} PRIVATE "../src")


if (MSVC)
    set_source_files_properties(StdAfx.cpp
        PROPERTIES
        COMPILE_FLAGS "/YcStdAfx.h"
        )
    foreach( src_file ${ROOT_SOURCE_FILES})
        set_source_files_properties(
            ${src_file}
            PROPERTIES
            COMPILE_FLAGS "/YuStdAfx.h"
            )
    endforeach()
endif (MSVC)
	


if(BUILD_SHARED_LIBS)
	target_link_libraries(${EXE_NAME} 
		$<TARGET_LINKER_FILE:duilib2>)
	# ppxbase
	find_package(ppxbase REQUIRED)
	target_link_libraries(${EXE_NAME} ppxbase)
else()
	target_link_libraries(${EXE_NAME} 
		$<TARGET_LINKER_FILE:duilib2-static>)
	
	# ppxbase
	find_package(ppxbase REQUIRED)
	target_link_libraries(${EXE_NAME} ppxbase-static)
endif()

