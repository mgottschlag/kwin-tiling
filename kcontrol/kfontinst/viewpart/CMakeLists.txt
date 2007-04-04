include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/
     )

set(kfontviewpart_PART_SRCS ${libkfontinstview_SRCS} FontViewPart.cpp FontViewPartFactory.cpp )

kde4_automoc(${kfontviewpart_PART_SRCS})
kde4_add_plugin(kfontviewpart WITH_PREFIX ${kfontviewpart_PART_SRCS})

target_link_libraries(kfontviewpart  ${KDE4_KPARTS_LIBS} kfontinstui kfontinst )

install(TARGETS kfontviewpart  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kfontviewpart.rc DESTINATION  ${DATA_INSTALL_DIR}/kfontinst )
install( FILES kfontviewpart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

