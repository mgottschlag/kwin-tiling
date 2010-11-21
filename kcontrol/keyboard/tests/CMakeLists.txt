MACRO(KEYBOARD_DAEMON_UNIT_TESTS _testname)
               kde4_add_unit_test(${_testname}_test NOGUI ${_testname}_test.cpp ../${_testname}.cpp)
               target_link_libraries(${_testname}_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB})
ENDMACRO(KEYBOARD_DAEMON_UNIT_TESTS)

KEYBOARD_DAEMON_UNIT_TESTS(xkb_rules)
KEYBOARD_DAEMON_UNIT_TESTS(iso_codes)
