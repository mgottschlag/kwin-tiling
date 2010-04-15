
########### next target ###############

macro_optional_find_package(Qalculate)
macro_log_feature(QALCULATE_FOUND "Qalculate!" "Qalculate Library" "http://qalculate.sourceforge.net/" FALSE "" "Needed to enable advanced features of the calculator runner")
if ( QALCULATE_FOUND )
    add_definitions(-DENABLE_QALCULATE)

    set(qalculate_engine_SRCS
    qalculate_engine.cpp
    )
 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS} -Wno-deprecated")
endif( QALCULATE_FOUND )

set(krunner_calculatorrunner_SRCS
    calculatorrunner.cpp
)

if ( QALCULATE_FOUND )
    kde4_add_plugin(krunner_calculatorrunner ${qalculate_engine_SRCS} ${krunner_calculatorrunner_SRCS})
    target_link_libraries(krunner_calculatorrunner ${QALCULATE_LIBRARIES} ${CLN_LIBRARIES} ${KDE4_SOLID_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} )
else ( QALCULATE_FOUND )
    kde4_add_plugin(krunner_calculatorrunner ${krunner_calculatorrunner_SRCS})
    target_link_libraries(krunner_calculatorrunner ${KDE4_KDEUI_LIBS} ${QT_QTSCRIPT_LIBRARY} ${KDE4_PLASMA_LIBS})	
endif ( QALCULATE_FOUND )

install(TARGETS krunner_calculatorrunner DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install(FILES plasma-runner-calculator.desktop DESTINATION ${SERVICES_INSTALL_DIR})
