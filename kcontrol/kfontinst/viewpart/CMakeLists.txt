include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_BINARY_DIR}
     )

set(kfontviewpart_PART_SRCS ${libkfontinstview_SRCS} ${libkfontinstdbusiface_SRCS} FontViewPart.cpp )

kde4_add_plugin(kfontviewpart ${kfontviewpart_PART_SRCS})

target_link_libraries(kfontviewpart  ${KDE4_KPARTS_LIBS} ${FONTCONFIG_LIBRARIES} kfontinstui kfontinst )

install(TARGETS kfontviewpart  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kfontviewpart.rc DESTINATION  ${DATA_INSTALL_DIR}/kfontinst )
install( FILES kfontviewpart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

