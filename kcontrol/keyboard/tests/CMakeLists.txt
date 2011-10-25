set ( text_paint_LIB ${KDE4_PLASMA_LIBS} )

MACRO(KEYBOARD_DAEMON_UNIT_TESTS _testname)
               kde4_add_unit_test(${_testname}_test NOGUI ${_testname}_test.cpp ../${_testname}.cpp)
               target_link_libraries(${_testname}_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB})
ENDMACRO(KEYBOARD_DAEMON_UNIT_TESTS)

MACRO(KEYBOARD_DAEMON_UNIT_TESTS2 _testname _src1 _src2 _src3)
               kde4_add_unit_test(${_testname}_test NOGUI ${_testname}_test.cpp ../${_testname}.cpp ../${_src1} ../${_src2} ../${_src3})
               target_link_libraries(${_testname}_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB})
ENDMACRO(KEYBOARD_DAEMON_UNIT_TESTS2)

#install( FILES config/base.xml config/base.1.1.xml DESTINATION ${BIN_INSTALL_DIR} )


KEYBOARD_DAEMON_UNIT_TESTS(xkb_rules)
KEYBOARD_DAEMON_UNIT_TESTS(iso_codes)

kde4_add_unit_test(flags_test NOGUI flags_test.cpp ../flags.cpp ../x11_helper.cpp ../keyboard_config.cpp ../xkb_rules.cpp)
target_link_libraries(flags_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB} ${X11_LIBRARIES} ${text_paint_LIB})

if( X11_XTest_FOUND )
   kde4_add_unit_test(keyboard_daemon_test NOGUI keyboard_daemon_test.cpp ../keyboard_daemon.cpp ../layout_memory.cpp ../layout_memory_persister.cpp ../flags.cpp ../x11_helper.cpp ../xkb_helper.cpp ../xinput_helper.cpp ../layout_tray_icon.cpp ../keyboard_config.cpp ../xkb_rules.cpp ../keyboard_hardware.cpp ../bindings.cpp ../layouts_menu.cpp ../numlockx.c)
   target_link_libraries(keyboard_daemon_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB} ${X11_XTest_LIB} ${X11_LIBRARIES} ${text_paint_LIB})
   if(XiGetDevicePresence_FOUND)
      target_link_libraries(keyboard_daemon_test ${X11_Xinput_LIB})
   endif(XiGetDevicePresence_FOUND)
endif( X11_XTest_FOUND )
