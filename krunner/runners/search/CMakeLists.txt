
########### next target ###############

include_directories(${CMAKE_SOURCE_DIR}/workspace/plasma/lib)

set(krunner_searchrunner_SRCS
    searchrunner.cpp
)

kde4_automoc(${krunner_searchrunner_SRCS})
kde4_add_plugin(krunner_searchrunner ${krunner_searchrunner_SRCS})
target_link_libraries(krunner_searchrunner ${KDE4_KDECORE_LIBS} plasma)

install(TARGETS krunner_searchrunner DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES krunner_searchrunner.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

