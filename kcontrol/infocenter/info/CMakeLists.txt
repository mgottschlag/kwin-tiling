

if(OPENGL_FOUND AND OPENGL_GLU_FOUND)
   ADD_DEFINITIONS(-DINFO_OPENGL_AVAILABLE ) # added manually
   include_directories( ${OPENGL_INCLUDE_DIR} ${OPENGL_GLU_INCLUDE_DIR} )
else(OPENGL_FOUND AND OPENGL_GLU_FOUND)
   MESSAGE(STATUS "OpenGL information module has been disabled.")
endif(OPENGL_FOUND AND OPENGL_GLU_FOUND)
########### next target ###############

set(kcm_info_PART_SRCS main.cpp memory.cpp opengl.cpp )

kde4_automoc(${kcm_info_PART_SRCS})

kde4_add_plugin(kcm_info ${kcm_info_PART_SRCS})


target_link_libraries(kcm_info  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${X11_X11_LIB})
if(OPENGL_FOUND AND OPENGL_GLU_FOUND)
  target_link_libraries(kcm_info ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
endif(OPENGL_FOUND AND OPENGL_GLU_FOUND)

install(TARGETS kcm_info  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES memory.desktop processor.desktop dma.desktop  	interrupts.desktop ioports.desktop opengl.desktop pci.desktop sound.desktop  	devices.desktop scsi.desktop partitions.desktop xserver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

