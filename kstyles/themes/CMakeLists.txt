
set( themes_RC
    qtcde.themerc
    qtcleanlooks.themerc
    qtmotif.themerc
    qtplastique.themerc
    qtwindows.themerc
)

if (Q_WS_X11)
  set( themes_RC ${themes_RC}
      qtgtk.themerc
  )
endif (Q_WS_X11)

if (WIN32)
  set( themes_RC ${themes_RC}
      qtwindowsvista.themerc
      qtwindowsxp.themerc
  )
endif (WIN32)

if (APPLE)
  set( themes_RC ${themes_RC}
      qtmacintosh.themerc
  )
endif (APPLE)


########### install files ###############

install( FILES ${themes_RC}  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes )




