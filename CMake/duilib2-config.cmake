# duilib-config.cmake - package configuration file

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

if(EXISTS ${SELF_DIR}/duilib2-target.cmake)
	include(${SELF_DIR}/duilib2-target.cmake)
endif()

if(EXISTS ${SELF_DIR}/duilib2-static-target.cmake)
	include(${SELF_DIR}/duilib2-static-target.cmake)
endif()
