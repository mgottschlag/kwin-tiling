find_library(STRIGIQTDBUSCLIENT_LIBRARY NAMES strigiqtdbusclient
  PATHS
  /usr/lib
  /usr/local/lib
  ${LIB_INSTALL_DIR}
  $ENV{STRIGI_HOME}/lib
)

macro_log_feature(STRIGIQTDBUSCLIENT_LIBRARY "strigi indexing" "Strigi built with indexing" "http://strigi.sourceforge.net/" FALSE "" "For the search functionality of the Run Command window")


if (STRIGIQTDBUSCLIENT_LIBRARY)

  set(krunner_searchrunner_SRCS searchrunner.cpp)
  kde4_add_plugin(krunner_searchrunner ${krunner_searchrunner_SRCS})
  include_directories(${STRIGI_INCLUDE_DIR})
  target_link_libraries(krunner_searchrunner ${KDE4_KDECORE_LIBS} plasma ${STRIGIQTDBUSCLIENT_LIBRARY})

  install(TARGETS krunner_searchrunner DESTINATION ${PLUGIN_INSTALL_DIR} )

  ########### install files ###############

  install(FILES plasma-runner-search.desktop DESTINATION ${SERVICES_INSTALL_DIR})

endif (STRIGIQTDBUSCLIENT_LIBRARY)
