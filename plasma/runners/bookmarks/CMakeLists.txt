
########### next target ###############

set(krunner_bookmarksrunner_SRCS
    bookmarksrunner.cpp
)

kde4_add_plugin(krunner_bookmarksrunner ${krunner_bookmarksrunner_SRCS})
target_link_libraries(krunner_bookmarksrunner ${KDE4_KIO_LIBS} ${QT_QTSCRIPT_LIBRARY} plasma)

install(TARGETS krunner_bookmarksrunner DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES plasma-runner-bookmarks.desktop DESTINATION ${SERVICES_INSTALL_DIR})

