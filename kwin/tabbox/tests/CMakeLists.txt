########################################################
# Test TabBox::ClientModel
########################################################
set( testTabBoxClientModel_SRCS
     ../clientmodel.cpp
     ../desktopmodel.cpp
     ../tabboxconfig.cpp
     ../tabboxhandler.cpp
     test_tabbox_clientmodel.cpp
     mock_declarative.cpp
     mock_tabboxhandler.cpp
     mock_tabboxclient.cpp
)

kde4_add_unit_test( testTabBoxClientModel TESTNAME testTabBoxClientModel ${testTabBoxClientModel_SRCS} )

target_link_libraries( testTabBoxClientModel ${KDE4_KDEUI_LIBS} ${QT_QTDECLARATIVE_LIBRARY} ${X11_LIBRARIES} ${QT_QTTEST_LIBRARY} )
