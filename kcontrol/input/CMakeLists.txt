
add_subdirectory( pics ) 

macro_bool_to_01(LIBUSB_FOUND HAVE_LIBUSB)
configure_file (config-kcontrol-input.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcontrol-input.h )

include_directories( ${KDE4_INCLUDES} )


if (LIBUSB_FOUND)
  include_directories( ${LIBUSB_INCLUDE_DIR} )
else( LIBUSB_FOUND)
  #use macro_log_feature for it
  MESSAGE(STATUS "Logitech mouse support is disabled. Provide libusb to enable it.")
endif (LIBUSB_FOUND)

if(X11_Xcursor_FOUND)
	set( libnoinst_SRCS xcursor/themepage.cpp xcursor/previewwidget.cpp )
	include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/xcursor/ )
else(X11_Xcursor_FOUND)
	set( libnoinst_SRCS core/themepage.cpp )
	include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/core/ )
endif(X11_Xcursor_FOUND)



########### next target ###############

set(kapplymousetheme_SRCS kapplymousetheme.cpp )

kde4_automoc(${kapplymousetheme_SRCS})

kde4_add_executable(kapplymousetheme ${kapplymousetheme_SRCS})

target_link_libraries(kapplymousetheme  ${KDE4_KDE3SUPPORT_LIBS} ${X11_Xrender_LIB})
if (X11_Xcursor_FOUND)
   target_link_libraries(kapplymousetheme ${X11_Xcursor_LIB})
endif (X11_Xcursor_FOUND)

install(TARGETS kapplymousetheme DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

set(kcm_input_PART_SRCS mouse.cpp main.cpp logitechmouse.cpp ${libnoinst_SRCS})

kde4_automoc(${kcm_input_PART_SRCS})

kde4_add_ui3_files(kcm_input_PART_SRCS kmousedlg.ui logitechmouse_base.ui )

kde4_add_plugin(kcm_input ${kcm_input_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_input )

target_link_libraries(kcm_input ${KDE4_KDE3SUPPORT_LIBS})
if (LIBUSB_FOUND)
   target_link_libraries(kcm_input ${LIBUSB_LIBRARIES})
endif (LIBUSB_FOUND)
if (X11_Xcursor_FOUND)
   target_link_libraries(kcm_input ${X11_Xcursor_LIB})
endif (X11_Xcursor_FOUND)
if (X11_Xrender_FOUND)
   target_link_libraries(kcm_input ${X11_Xrender_LIB})
endif (X11_Xrender_FOUND)

install(TARGETS kcm_input  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES mouse.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES cursor_large_black.pcf.gz cursor_large_white.pcf.gz cursor_small_white.pcf.gz  DESTINATION  ${DATA_INSTALL_DIR}/kcminput )
install( FILES mouse_cursor_theme.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )

