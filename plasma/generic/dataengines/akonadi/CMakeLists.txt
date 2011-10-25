include_directories(
        ${Boost_INCLUDE_DIR}
        ${KDEPIMLIBS_INCLUDE_DIRS}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(akonadi_engine_srcs
    akonadiengine.cpp
)

kde4_add_plugin(plasma_engine_akonadi ${akonadi_engine_srcs})

target_link_libraries(
    plasma_engine_akonadi
    ${KDE4_KPIMUTILS_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_AKONADI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KMIME_LIBS}
    ${KDE4_AKONADI_KMIME_LIBS}
    ${KDE4_KABC_LIBS}
    ${QT_QTXML_LIBRARY}
    microblog
)

install(TARGETS plasma_engine_akonadi DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-akonadi.desktop DESTINATION ${SERVICES_INSTALL_DIR})

