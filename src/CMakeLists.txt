if(BUILD_SHARED_LIBS)
	set(LIB_NAME duilib2)
else()
	set(LIB_NAME duilib2-static)
endif()


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


file(GLOB ROOT_SOURCE_FILES 			./UIlib.cpp)
file(GLOB CONTROL_SOURCE_FILES 		./Control/*.cpp)
file(GLOB CORE_SOURCE_FILES 		    ./Core/*.cpp)
file(GLOB LAYOUT_SOURCE_FILES 		./Layout/*.cpp)
file(GLOB RENDER_SOURCE_FILES 		./Render/*.cpp)
file(GLOB UTILS_SOURCE_FILES 		./Utils/*.cpp)
file(GLOB INTERNAL_SOURCE_FILES 		./Internal/*.cpp)

file(GLOB ROOT_HEADER_FILES 			./UIlib.h ./UIlibExport.h)
file(GLOB CONTROL_HEADER_FILES 		./Control/*.h ./Control/*.hpp)
file(GLOB CORE_HEADER_FILES 		    ./Core/*.h ./Core/*.hpp)
file(GLOB LAYOUT_HEADER_FILES 		./Layout/*.h ./Layout/*.hpp)
file(GLOB RENDER_HEADER_FILES 		./Render/*.h ./Render/*.hpp)
file(GLOB UTILS_HEADER_FILES 		./Utils/*.h ./Utils/*.hpp)
file(GLOB INTERNAL_HEADER_FILES 		./Internal/*.h ./Internal/*.hpp)

source_group("Header Files" FILES 
	${ROOT_HEADER_FILES} ./StdAfx.h)
source_group("Header Files\\Control" FILES 
	${CONTROL_HEADER_FILES})
source_group("Header Files\\Core" FILES 
	${CORE_HEADER_FILES})
source_group("Header Files\\Layout" FILES 
	${LAYOUT_HEADER_FILES})
source_group("Header Files\\Render" FILES 
	${RENDER_HEADER_FILES})
source_group("Header Files\\Utils" FILES 
	${UTILS_HEADER_FILES})
source_group("Header Files\\Internal" FILES 
	${INTERNAL_HEADER_FILES})

source_group("Source Files" FILES 
	${ROOT_SOURCE_FILES} ./StdAfx.cpp)
source_group("Source Files\\Control" FILES 
	${CONTROL_SOURCE_FILES})
source_group("Source Files\\Core" FILES 
	${CORE_SOURCE_FILES})
source_group("Source Files\\Layout" FILES 
	${LAYOUT_SOURCE_FILES})
source_group("Source Files\\Render" FILES 
	${RENDER_SOURCE_FILES})
source_group("Source Files\\Utils" FILES 
	${UTILS_SOURCE_FILES})
source_group("Source Files\\Internal" FILES 
	${INTERNAL_SOURCE_FILES})



if (MSVC)
    # set_source_files_properties(StdAfx.cpp
        # PROPERTIES
        # COMPILE_FLAGS "/YcStdAfx.h"
        # )
    # foreach( src_file ${ROOT_SOURCE_FILES}
		# ${CONTROL_SOURCE_FILES}
		# ${CORE_SOURCE_FILES}
		# ${LAYOUT_SOURCE_FILES}
		# ${RENDER_SOURCE_FILES}
		# ${UTILS_SOURCE_FILES}
		# ${INTERNAL_SOURCE_FILES} )
        # set_source_files_properties(
            # ${src_file}
            # PROPERTIES
            # COMPILE_FLAGS "/YuStdAfx.h"
            # )
    # endforeach()
	
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /source-charset:.936")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:.936")
endif (MSVC)


if(BUILD_SHARED_LIBS)
	add_definitions(-DUILIB_EXPORTS)
	set(UILIB_STATIC NO)
	
	add_library(
		${LIB_NAME}
		SHARED
		${ROOT_HEADER_FILES} ./StdAfx.h
		${CONTROL_HEADER_FILES}
		${CORE_HEADER_FILES}
		${LAYOUT_HEADER_FILES}
		${RENDER_HEADER_FILES}
		${UTILS_HEADER_FILES}
		${INTERNAL_HEADER_FILES}
		${ROOT_SOURCE_FILES} ./StdAfx.cpp
		${CONTROL_SOURCE_FILES}
		${CORE_SOURCE_FILES}
		${LAYOUT_SOURCE_FILES}
		${RENDER_SOURCE_FILES}
		${UTILS_SOURCE_FILES}
		${INTERNAL_SOURCE_FILES}
	)
	
	# set output name prefix/suffix
	set_target_properties(${LIB_NAME} PROPERTIES 
		OUTPUT_NAME ${LIB_NAME}
		DEBUG_OUTPUT_NAME ${LIB_NAME}${DEBUG_LIBNAME_SUFFIX})
else()
	add_definitions(-DUILIB_STATIC)
	set(UILIB_STATIC YES)
	
	add_library(
		${LIB_NAME}
		STATIC
		${ROOT_HEADER_FILES} ./StdAfx.h
		${CONTROL_HEADER_FILES}
		${CORE_HEADER_FILES}
		${LAYOUT_HEADER_FILES}
		${RENDER_HEADER_FILES}
		${UTILS_HEADER_FILES}
		${INTERNAL_HEADER_FILES}
		${ROOT_SOURCE_FILES} ./StdAfx.cpp
		${CONTROL_SOURCE_FILES}
		${CORE_SOURCE_FILES}
		${LAYOUT_SOURCE_FILES}
		${RENDER_SOURCE_FILES}
		${UTILS_SOURCE_FILES}
		${INTERNAL_SOURCE_FILES}
	)
	
	# set output name prefix/suffix
    set_target_properties(${LIB_NAME} PROPERTIES 
		OUTPUT_NAME ${LIB_NAME}
		DEBUG_OUTPUT_NAME ${LIB_NAME}${DEBUG_LIBNAME_SUFFIX})
		
	set_target_properties(${LIB_NAME} PROPERTIES 
		INTERFACE_COMPILE_DEFINITIONS UILIB_STATIC)
endif()


target_include_directories(${LIB_NAME} PRIVATE ".")



# PPXBASE
find_package(ppxbase REQUIRED)
if(BUILD_SHARED_LIBS)
	target_link_libraries(${LIB_NAME} PUBLIC 
		ppxbase)
else()
	target_link_libraries(${LIB_NAME} PUBLIC 
		ppxbase-static)
endif()



# Set output directory
set_target_properties(${LIB_NAME}
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

target_include_directories(${LIB_NAME} INTERFACE
	$<INSTALL_INTERFACE:include>)

install(TARGETS ${LIB_NAME}
	EXPORT ${LIB_NAME}-target
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)


install(FILES ${ROOT_HEADER_FILES} 			DESTINATION include/)
install(FILES ${CONTROL_HEADER_FILES} 		DESTINATION include/Control)
install(FILES ${CORE_HEADER_FILES} 		    DESTINATION include/Core)
install(FILES ${LAYOUT_HEADER_FILES} 		DESTINATION include/Layout)
install(FILES ${RENDER_HEADER_FILES} 		DESTINATION include/Render)
install(FILES ${UTILS_HEADER_FILES} 		DESTINATION include/Utils)


install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/duilib2-config.cmake 
	DESTINATION ${DUILIB_INSTALL_CMAKE_DIR})

export(TARGETS ${LIB_NAME}
       APPEND FILE ${PROJECT_BINARY_DIR}/${LIB_NAME}-target.cmake
)

install(EXPORT ${LIB_NAME}-target
        FILE ${LIB_NAME}-target.cmake
        DESTINATION ${DUILIB_INSTALL_CMAKE_DIR}
)