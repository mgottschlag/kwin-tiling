



########### next target ###############

set(kickermain_LIB_SRCS 
   abstractpanelmanager.cpp 
   appletinfo.cpp 
   utils.cpp 
   kickertip.cpp 
   menuinfo.cpp 
   panelbutton.cpp 
   panner.cpp 
   kshadowsettings.cpp 
   kshadowengine.cpp 
   kpanelapplet.cpp 
   kpanelextension.cpp 
   kpanelmenu.cpp 
   simplebutton.cpp 
   extensioninfo.cpp 
   paneldrag.cpp )

kde4_automoc(${kickermain_LIB_SRCS})

kde4_add_kcfg_files(kickermain_LIB_SRCS kickerSettings.kcfgc extensionSettings.kcfgc )

kde4_add_library(kickermain SHARED ${kickermain_LIB_SRCS})

target_link_libraries(kickermain  ${KDE4_KIO_LIBS} ${QT_QT3SUPPORT_LIBRARY} kworkspace)

set_target_properties(kickermain PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kickermain  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES kickerSettings.kcfg extensionSettings.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES kpanelapplet.h kpanelextension.h kpanelmenu.h kickertip.h utils.h paneldrag.h DESTINATION ${INCLUDE_INSTALL_DIR}/plasma )


