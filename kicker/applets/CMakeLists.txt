
add_subdirectory( systemtray ) 
add_subdirectory( minipager ) 
MESSAGE(STATUS "taskbar panel applet disable (since taskbar lib is disabled), reactivate it when we use plasma or remove it")
#add_subdirectory( taskbar ) 
add_subdirectory( run ) 
add_subdirectory( launcher ) 
add_subdirectory( naughty ) 
add_subdirectory( lockout ) 
add_subdirectory( menu ) 
add_subdirectory( media ) 
add_subdirectory( trash ) 



########### install files ###############





#original Makefile.am contents follow:

## network
#SUBDIRS = systemtray minipager taskbar run launcher naughty lockout menu media trash
