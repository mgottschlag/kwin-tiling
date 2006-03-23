kde4_header()

add_subdirectory( find ) 
add_subdirectory( kdeprint ) 
add_subdirectory( prefmenu ) 
add_subdirectory( recentdocs ) 
add_subdirectory( konsole ) 
add_subdirectory( konq-profiles ) 
add_subdirectory( remote ) 
add_subdirectory( system ) 

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = find kdeprint prefmenu recentdocs konsole konq-profiles remote system
#
