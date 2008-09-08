
if (X11_Xft_FOUND)

  # disable policykit usage for now...
  if (KFI_USE_POLICY_KIT)
    include(FindPkgConfig)
    pkg_check_modules(POLKIT polkit>=0.7)

    if (POLKIT_FOUND)
        pkg_check_modules(DBUS dbus-1>=1.1.20)
    endif (POLKIT_FOUND)

    if (DBUS_FOUND)
        pkg_check_modules(POLKITDBUS polkit-dbus>=0.7)
    endif (DBUS_FOUND)

    if (POLKITDBUS_FOUND)
        set(KFI_POLKIT_INCLUDES ${POLKIT_INCLUDE_DIRS} ${POLKITDBUS_INCLUDE_DIRS} ${DBUS_INCLUDE_DIRS} )
        set(KFI_POLKIT_LIBS -lQtDBus ${POLKIT_LDFLAGS} ${POLKITDBUS_LDFLAGS} ${DBUS_LDFLAGS} )
        set(kfi_polkit_auth_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/PolicyKitAuthenticator.cpp)
        set(USE_POLICYKIT 1)
    else (POLKITDBUS_FOUND)
        set(USE_POLICYKIT 0)
    endif (POLKITDBUS_FOUND)
  else (KFI_USE_POLICY_KIT)
    set(USE_POLICYKIT 0)
  endif (KFI_USE_POLICY_KIT)

  configure_file(config-fontinst.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-fontinst.h)

  include_directories(
      ${FREETYPE_INCLUDE_DIR}
      ${FONTCONFIG_INCLUDE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/lib)

  set(libkfontinstview_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/FontPreview.cpp 
                            ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/PreviewSelectAction.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/CharTip.cpp )
  set(libkfontinstjobrunner_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/JobRunner.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/ActionDialog.cpp
                                 ${kfi_polkit_auth_SRCS})

  add_subdirectory( lib ) 
  add_subdirectory( kcmfontinst ) 
  add_subdirectory( strigi-analyzer )
  add_subdirectory( apps ) 
  add_subdirectory( kio ) 
  add_subdirectory( thumbnail )
  add_subdirectory( viewpart )

  kde4_install_icons( ${ICON_INSTALL_DIR} )

endif (X11_Xft_FOUND)

macro_log_feature(X11_Xft_FOUND "libxft" "X FreeType interface library" "http://www.x.org" FALSE "" "Font installer and font previews")
