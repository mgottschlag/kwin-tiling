set(places_engine_SRCS
    placesengine.cpp
)

kde4_add_plugin(plasma_engine_places ${places_engine_SRCS})
target_link_libraries(plasma_engine_places plasma ${KDE4_KFILE_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS plasma_engine_places DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-places.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

