
if (X11_XFT_FOUND)

  add_definitions(${FREETYPE_INCLUDE_DIR})

  set(libkfontinstprint_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/KfiPrint.cpp )

  include_directories(${FONTCONFIG_INCLUDE_DIR})

  add_subdirectory( lib ) 
  add_subdirectory( kcmfontinst ) 
  add_subdirectory( kfile-plugin ) 
  add_subdirectory( kfontinst ) 
  add_subdirectory( kio ) 
  add_subdirectory(thumbnail)
  add_subdirectory(viewpart)

else (X11_XFT_FOUND)

  message(STATUS Missing Xft - disabling font installer and font previews)

endif (X11_XFT_FOUND)




########### install files ###############

install( FILES installfont.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqueror/servicemenus )
