install(FILES package_ruboid.rb DESTINATION ${DATA_INSTALL_DIR}/plasma_scriptengine_ruby)
install(FILES applet.rb DESTINATION ${DATA_INSTALL_DIR}/plasma_scriptengine_ruby)
install(FILES data_engine.rb DESTINATION ${DATA_INSTALL_DIR}/plasma_scriptengine_ruby)

install(FILES plasma-scriptengine-ruby-package.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-ruby-applet.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-ruby-dataengine.desktop DESTINATION ${SERVICES_INSTALL_DIR})

