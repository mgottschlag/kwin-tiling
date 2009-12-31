# If libutempter is not available on the system, we build
# kwrited as an executable that is installed with setuid root
# (or setgid utmp) in order to be able to login successfully
# (and thus be useful, as kwrited does not work at all if it
# cannot login to the terminal device that it opens).

if (NOT KDE4_KPTY_BUILT_WITH_UTEMPTER)
    set(BUILD_AS_EXECUTABLE 1)
endif (NOT KDE4_KPTY_BUILT_WITH_UTEMPTER)

configure_file(config-kwrited.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwrited.h)

if (BUILD_AS_EXECUTABLE)
    kde4_add_executable(kwrited kwrited.cpp)
    target_link_libraries(kwrited  ${KDE4_KDEUI_LIBS} ${KDE4_KPTY_LIBS})
    install(TARGETS kwrited  ${INSTALL_TARGETS_DEFAULT_ARGS} )
    install(CODE "
               set(KWRITED_PATH \"\$ENV{DESTDIR}${BIN_INSTALL_DIR}/kwrited\")

               # On Debian we setgid instead of setuid.
               # Feel free to add checks here for other distros as well, if needed.
               if (EXISTS /etc/debian_version)
                   set(SETGID 1)
                   set(SETGID_GROUP utmp)
               endif (EXISTS /etc/debian_version)

               if (SETGID)
                   execute_process(COMMAND sh -c \"chgrp \${SETGID_GROUP} '\${KWRITED_PATH}' && chmod g+s '\${KWRITED_PATH}'\")
               else (SETGID)
                   execute_process(COMMAND sh -c \"chown root '\${KWRITED_PATH}' && chmod u+s '\${KWRITED_PATH}'\")
               endif(SETGID)
           ")

    install(FILES kwrited-autostart.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
else (BUILD_AS_EXECUTABLE)
    kde4_add_plugin(kded_kwrited kwrited.cpp)
    target_link_libraries(kded_kwrited  ${KDE4_KDEUI_LIBS} ${KDE4_KPTY_LIBS})
    install(TARGETS kded_kwrited  DESTINATION ${PLUGIN_INSTALL_DIR} )

    install(FILES kwrited.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
endif (BUILD_AS_EXECUTABLE)

# The .notifyrc file is common
install(FILES kwrited.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kwrited)
