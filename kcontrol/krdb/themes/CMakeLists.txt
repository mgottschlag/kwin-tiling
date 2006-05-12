kde4_header()

add_subdirectory( HighContrastDark ) 
add_subdirectory( HighContrastDark-big ) 
add_subdirectory( HighContrastLight ) 
add_subdirectory( HighContrastLight-big ) 
add_subdirectory( YellowOnBlue ) 
add_subdirectory( YellowOnBlue-big ) 
add_subdirectory( KDE_Classic ) 
add_subdirectory( Keramik ) 
add_subdirectory( Plastik ) 
add_subdirectory( Platinum ) 
add_subdirectory( Sunshine ) 
add_subdirectory( Redmond ) 



########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = HighContrastDark  HighContrastDark-big \
#          HighContrastLight HighContrastLight-big \
#          YellowOnBlue      YellowOnBlue-big \
#          KDE_Classic Keramik Plastik Platinum Sunshine Redmond