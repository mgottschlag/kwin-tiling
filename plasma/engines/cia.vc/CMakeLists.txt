find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/workspace/lib/)

set(ciavc_engine_SRCS
    ciavc.cpp
)

kde4_automoc(${ciavc_engine_SRCS})
kde4_add_plugin(plasma_engine_ciavc ${ciavc_engine_SRCS})
target_link_libraries(plasma_engine_ciavc ${KDE4_KDECORE_LIBS} ${KDE4_SYNDICATION_LIBS} plasma)

install(TARGETS plasma_engine_ciavc DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-ciavc.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

