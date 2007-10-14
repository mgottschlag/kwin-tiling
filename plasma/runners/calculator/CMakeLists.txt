
########### next target ###############

set(krunner_calculatorrunner_SRCS
    calculatorrunner.cpp
)

kde4_add_plugin(krunner_calculatorrunner ${krunner_calculatorrunner_SRCS})
target_link_libraries(krunner_calculatorrunner ${KDE4_KDECORE_LIBS} ${QT_QTSCRIPT_LIBRARY} plasma)

install(TARGETS krunner_calculatorrunner DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES plasma-runner-calculator.desktop DESTINATION ${SERVICES_INSTALL_DIR})

