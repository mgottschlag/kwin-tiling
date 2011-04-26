set ( text_paint_LIB ${KDE4_PLASMA_LIBS} )

MACRO(KEYBOARD_DAEMON_UNIT_TESTS _testname)
               kde4_add_unit_test(${_testname}_test NOGUI ${_testname}_test.cpp ../${_testname}.cpp)
               target_link_libraries(${_testname}_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB})
ENDMACRO(KEYBOARD_DAEMON_UNIT_TESTS)

MACRO(KEYBOARD_DAEMON_UNIT_TESTS2 _testname _src1 _src2 _src3)
               kde4_add_unit_test(${_testname}_test NOGUI ${_testname}_test.cpp ../${_testname}.cpp ../${_src1} ../${_src2} ../${_src3})
               target_link_libraries(${_testname}_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB})
ENDMACRO(KEYBOARD_DAEMON_UNIT_TESTS2)

KEYBOARD_DAEMON_UNIT_TESTS(xkb_rules)
KEYBOARD_DAEMON_UNIT_TESTS(iso_codes)

kde4_add_unit_test(flags_test NOGUI flags_test.cpp ../flags.cpp ../x11_helper.cpp ../keyboard_config.cpp ../xkb_rules.cpp)
target_link_libraries(flags_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB} ${X11_LIBRARIES} ${text_paint_LIB})

kde4_add_unit_test(layout_memory_persister_test NOGUI layout_memory_persister_test.cpp ../layout_memory_persister.cpp ../layout_memory.cpp ../x11_helper.cpp ../keyboard_config.cpp ../xkb_helper.cpp)
target_link_libraries(layout_memory_persister_test ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_Xkbfile_LIB} ${X11_LIBRARIES} ${text_paint_LIB})
