project(plasma-clock)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/workspace/plasma/lib ${CMAKE_SOURCE_DIR}/workspace/lib)

########### next target ###############
set(clock_SRCS
    clock.cpp)

kde4_automoc(${clock_SRCS})

kde4_add_library(plasma-clock SHARED ${clock_SRCS})

target_link_libraries(plasma-clock plasma ${KDE4_KIO_LIBS} )

set_target_properties(plasma-clock PROPERTIES VERSION 1.0.0 SOVERSION 1 )

## Install stuff ##

install(TARGETS plasma-clock DESTINATION ${LIB_INSTALL_DIR} )
install(FILES plasma-clock-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
