configure_file (config-kfontinst.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kfontinst.h )

include_directories(
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/ 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/fonts/
	 )


########### next target ###############

if (HAVE_X11_FONTS_FONTENC_H)
   set(kfontinst_bin_SRCS ${libkxftconfig_SRCS}
      CompressedFile.cpp 
      Main.cpp 
      Encodings.cpp 
      FontEngine.cpp 
      Fontmap.cpp 
      GetPid.c 
      XConfig.cpp)

   kde4_automoc(${kfontinst_bin_SRCS})

   kde4_add_executable(kfontinst_bin ${kfontinst_bin_SRCS})
   set_target_properties(kfontinst_bin PROPERTIES OUTPUT_NAME kfontinst)

   target_link_libraries(kfontinst_bin  ${KDE4_KIO_LIBS} kfontinst fontenc )

   install(TARGETS kfontinst_bin DESTINATION ${BIN_INSTALL_DIR})

endif (HAVE_X11_FONTS_FONTENC_H)



########### install files ###############


