cmake_minimum_required(VERSION 3.7.1)

project(dbus-asio VERSION 1.0.0)

set(CMAKE_CXX_VISIBILITY_PRESET protected)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold")

set(HEADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_asio.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_auth.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_connection.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_error.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_introspectable.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_log.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_matchrule.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_message.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_messageprotocol.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_messageostream.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_messageistream.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_names.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_octetbuffer.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_platform.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_transport.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_any.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_array.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_boolean.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_byte.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_dictentry.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_double.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_int16.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_int32.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_int64.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_objectpath.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_signature.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_string.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_struct.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_uint16.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_uint32.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_uint64.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_unixfd.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_type_variant.h
	${CMAKE_CURRENT_LIST_DIR}/src/dbus_utils.h
)

add_library(dbus-asio SHARED
	src/dbus_auth.cpp
	src/dbus_introspectable.cpp
	src/dbus_log.cpp
	src/dbus_matchrule.cpp
	src/dbus_message.cpp
	src/dbus_messageistream.cpp
	src/dbus_messageprotocol.cpp
	src/dbus_names.cpp
	src/dbus_octetbuffer.cpp
	src/dbus_platform.cpp
	src/dbus_transport.cpp
	src/dbus_type.cpp
	src/dbus_type_any.cpp
	src/dbus_type_array.cpp
	src/dbus_type_boolean.cpp
	src/dbus_type_byte.cpp
	src/dbus_type_dictentry.cpp
	src/dbus_type_double.cpp
	src/dbus_type_int16.cpp
	src/dbus_type_int32.cpp
	src/dbus_type_int64.cpp
	src/dbus_type_objectpath.cpp
	src/dbus_type_signature.cpp
	src/dbus_type_string.cpp
	src/dbus_type_struct.cpp
	src/dbus_type_uint16.cpp
	src/dbus_type_uint32.cpp
	src/dbus_type_uint64.cpp
	src/dbus_type_unixfd.cpp
	src/dbus_type_variant.cpp
	src/dbus_utils.cpp
)

# ASIO
option(DBUS_ASIO_STANDALONE "Use ASIO standalone library (non-boost)" OFF)
if(DBUS_ASIO_STANDALONE)
    add_compile_definitions(ASIO_STANDALONE)
    set(DBUS_ASIO_INCLUDE "asio.hpp")
    set(DBUS_ASIO_NAMESPACE "::asio")
    set(DBUS_ERROR_CODE_NAMESPACE "::std")
    set(DBUS_ERROR_CODE_ERRC "::std::errc")
    set(DBUS_MAKE_ERROR_CODE "::std::make_error_code")
else()
    set(DBUS_ASIO_INCLUDE "boost/asio.hpp")
    set(DBUS_ASIO_NAMESPACE "::boost::asio")
    set(DBUS_ERROR_CODE_NAMESPACE "::boost::system")
    set(DBUS_ERROR_CODE_ERRC "::boost::system::errc::errc_t")
    set(DBUS_MAKE_ERROR_CODE "::boost::asio::error::make_error_code")

    find_package (Threads REQUIRED)
    target_link_libraries(dbus-asio Threads::Threads)
endif()
set(DBUS_ASIO_H ${CMAKE_CURRENT_SOURCE_DIR}/src/dbus_asio.h)
configure_file(${DBUS_ASIO_H}.in ${DBUS_ASIO_H})
unset(DBUS_ASIO_H)

set_target_properties(dbus-asio PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER "${HEADER_FILES}")

set(PLATFORM_LIBS "-ldbus-asio")

configure_file(dbus-asio.pc.in dbus-asio.pc @ONLY)

target_include_directories(dbus-asio PUBLIC src)

include(CTest)

option(ENABLE_EXAMPLES "Build example projects" OFF)
if (ENABLE_EXAMPLES)
	add_subdirectory(example)
endif(ENABLE_EXAMPLES)
add_subdirectory(tests)

install(TARGETS dbus-asio
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION include/libdbus-asio)

install(FILES ${CMAKE_BINARY_DIR}/dbus-asio.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(FILES dbus-asio-config.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dbus-asio/)

include(GNUInstallDirs)
