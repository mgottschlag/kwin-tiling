include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/
     )



########### next target ###############


########### next target ###############

set(kfontviewpart_PART_SRCS FontViewPart.cpp FontViewPartFactory.cpp FontPreview.cpp ${libkfontinstprint_SRCS})

kde4_automoc(${kfontviewpart_PART_SRCS})

kde4_add_plugin(kfontviewpart WITH_PREFIX ${kfontviewpart_PART_SRCS})


target_link_libraries(kfontviewpart  ${KDE4_KPARTS_LIBS} ${KDE4_KDEPRINT_LIBS} kfontinst )

install(TARGETS kfontviewpart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kfontview_SRCS FontViewerApp.cpp )

kde4_automoc(${kfontview_SRCS})

kde4_add_executable(kfontview ${kfontview_SRCS})

target_link_libraries(kfontview  ${KDE4_KPARTS_LIBS} )

install(TARGETS kfontview DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES kfontview.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kfontviewpart.rc kfontviewui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kfontview )
install( FILES kfontviewpart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

