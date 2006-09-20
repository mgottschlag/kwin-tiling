
project(kpersonalizer)

add_subdirectory( pics ) 



########### next target ###############

set(kpersonalizer_SRCS 
   krefinepage.cpp 
   kstylepage.cpp 
   keyecandypage.cpp 
   kospage.cpp 
   kcountrypage.cpp 
   kpersonalizer.cpp 
   main.cpp 
   kfindlanguage.cpp 
   stylepreview.cpp
   ksysinfo.cpp )

kde4_automoc(${kpersonalizer_SRCS})

kde4_add_ui_files(kpersonalizer_SRCS 
   stylepreview.ui 
)

kde4_add_ui3_files(kpersonalizer_SRCS 
   kcountrypagedlg.ui 
   kospagedlg.ui 
   keyecandypagedlg.ui 
   kstylepagedlg.ui 
   krefinepagedlg.ui )

kde4_add_executable(kpersonalizer ${kpersonalizer_SRCS})

target_link_libraries(kpersonalizer  ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kpersonalizer DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES kpersonalizer.desktop  DESTINATION  ${XDG_APPS_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )


