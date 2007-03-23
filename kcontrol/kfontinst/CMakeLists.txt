
if (X11_XFT_FOUND)

  add_definitions(${FREETYPE_INCLUDE_DIR})

  set(libkfontinstview_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/FontPreview.cpp 
                            ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/PreviewSelectAction.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/CharTip.cpp )
  set(libkfontinstjobrunner_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/JobRunner.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/ActionDialog.cpp )

  include_directories(${FONTCONFIG_INCLUDE_DIR})

  add_subdirectory( lib ) 
  add_subdirectory( kcmfontinst ) 
#  add_subdirectory( kfile-plugin ) 
  add_subdirectory( kfontinst ) 
  add_subdirectory( kio ) 
  add_subdirectory( thumbnail )
  add_subdirectory( viewpart )
  add_subdirectory( mime )
  install( FILES installfont.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqueror/servicemenus )

else (X11_XFT_FOUND)

  message(STATUS Missing Xft - disabling font installer and font previews)

endif (X11_XFT_FOUND)
