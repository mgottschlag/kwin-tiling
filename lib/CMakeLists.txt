
project(kworkspacelib)



########### next target ###############

set(kworkspace_LIB_SRCS kworkspace.cpp )

kde4_automoc(${kworkspace_LIB_SRCS})

kde4_add_library(kworkspace SHARED ${kworkspace_LIB_SRCS})

target_link_libraries(kworkspace  ${KDE4_KIO_LIBS} ${X11_LIBRARIES} )

set_target_properties(kworkspace PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kworkspace  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES kworkspace.h DESTINATION ${INCLUDE_INSTALL_DIR}/libworkspace )
