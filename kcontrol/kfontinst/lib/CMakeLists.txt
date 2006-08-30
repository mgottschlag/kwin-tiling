


########### next target ###############

set(kfontinst_LIB_SRCS Misc.cpp FcEngine.cpp )

kde4_automoc(${kfontinst_LIB_SRCS})

kde4_add_library(kfontinst SHARED ${kfontinst_LIB_SRCS})

target_link_libraries(kfontinst  ${KDE4_KIO_LIBS} ${FREETYPE_LIBRARIES} ${FONTCONFIG_LIBRARIES} ${X11_Xft_LIB})

set_target_properties(kfontinst PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kfontinst  DESTINATION ${LIB_INSTALL_DIR} )
