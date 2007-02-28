# the following two are included directly in krunner itself
#add_subdirectory(app)
#add_subdirectory(shell)

# these are all loadable runners
add_subdirectory(search)
add_subdirectory(calculator)

install( FILES  krunnerrunner.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR}  )
