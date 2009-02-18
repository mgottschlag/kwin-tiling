include_directories(
        ${CMAKE_SOURCE_DIR}/calendar
        ${KDEPIMLIBS_INCLUDE_DIR}
        ${Boost_INCLUDE_DIR}
)

set(calendar_engine_srcs
    calendarengine.cpp
)

kde4_add_plugin(plasma_engine_calendar ${calendar_engine_srcs})

target_link_libraries(
    plasma_engine_calendar
    kholidays
    ${KDE4_KDECORE_LIBS} ${KDE4_AKONADI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KMIME_LIBS}
    ${KDE4_AKONADI_KMIME_LIBS}
)

install(TARGETS plasma_engine_calendar DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-calendar.desktop DESTINATION ${SERVICES_INSTALL_DIR})

