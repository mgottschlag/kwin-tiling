project(calendar_engine)

include_directories(
        ${KDEPIMLIBS_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(calendar_engine_srcs
    calendarengine.cpp
)

if(AKONADI_FOUND)
    ADD_DEFINITIONS(-DAKONADI_FOUND)
    set(calendar_engine_srcs ${calendar_engine_srcs}
        eventdatacontainer.cpp
        # taken from kdepim/akonadi/kcal as long as it's not yet exported:
        akonadi/calendar.cpp
        akonadi/calendarmodel.cpp
        akonadi/calfilterproxymodel.cpp
        akonadi/utils.cpp
        akonadi/blockalarmsattribute.cpp
        akonadi/collectionselection.cpp
    )
endif(AKONADI_FOUND)

kde4_add_plugin(plasma_engine_calendar ${calendar_engine_srcs})

target_link_libraries(
    plasma_engine_calendar
    ${KDEPIMLIBS_KHOLIDAYS_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KMIME_LIBS}
    ${KDE4_KCALCORE_LIBS}
    ${KDE4_KCALUTILS_LIBS}
)

if(AKONADI_FOUND)
    target_link_libraries(
        plasma_engine_calendar
        ${KDE4_AKONADI_LIBS}
        ${KDEPIMLIBS_AKONADI_KCAL_LIBS}
    )
endif(AKONADI_FOUND)

install(TARGETS plasma_engine_calendar DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-calendar.desktop DESTINATION ${SERVICES_INSTALL_DIR})

