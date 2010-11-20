MACRO(KEYBOARD_DAEMON_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp ../xkb_rules.cpp)
               target_link_libraries(${_testname} ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB})
       ENDFOREACH(_testname)
ENDMACRO(KEYBOARD_DAEMON_UNIT_TESTS)

KEYBOARD_DAEMON_UNIT_TESTS(
    xkb_rules_test
)
