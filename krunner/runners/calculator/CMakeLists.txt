
########### next target ###############

include_directories(${CMAKE_SOURCE_DIR}/workspace/plasma/lib)

set(krunner_calculatorrunner_SRCS
    calculatorrunner.cpp
)

kde4_automoc(${krunner_calculatorrunner_SRCS})
kde4_add_plugin(krunner_calculatorrunner ${krunner_calculatorrunner_SRCS})
target_link_libraries(krunner_calculatorrunner ${KDE4_KDECORE_LIBS} plasma)

install(TARGETS krunner_calculatorrunner DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES krunner_calculatorrunner.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

