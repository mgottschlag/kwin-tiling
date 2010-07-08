set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


########### solidnettest ###############

#set(solidnettest_SRCS
#   solidnettest.cpp )


#kde4_add_unit_test(solidnettest ${solidnettest_SRCS})
#add_definitions(-DFAKE_NETWORKING_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../backends/fakenet/fakenetworking.xml\\"")

#include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../backends/fakenet )

#target_link_libraries(solidnettest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} solidcontrol solidcontrolifaces solid_fakenet_static)

########### install files ###############
