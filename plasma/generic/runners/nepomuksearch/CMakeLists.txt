project(nepomuksearchrunner)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_INCLUDES}
)

SET(nepomuksearchrunner_SRCS
	nepomuksearchrunner.cpp
    queryclientwrapper.cpp
)

kde4_add_plugin(krunner_nepomuksearchrunner ${nepomuksearchrunner_SRCS})

target_link_libraries(krunner_nepomuksearchrunner
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${NEPOMUK_LIBRARIES}
  ${NEPOMUK_QUERY_LIBRARIES}
  ${QT_QTGUI_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_PLASMA_LIBS}
)

install(
  TARGETS krunner_nepomuksearchrunner
  DESTINATION ${PLUGIN_INSTALL_DIR}
  )

install(
  FILES plasma-runner-nepomuksearch.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
)
