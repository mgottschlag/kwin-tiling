# - Try to find the LDAP client libraries
# Once done this will define
#
#  LDAP_FOUND - system has libldap
#  LDAP_INCLUDE_DIR - the ldap include directory
#  LDAP_LIBRARIES - libldap library
#  LBER_LIBRARIES - liblber library

FIND_PATH(LDAP_INCLUDE_DIR ldap.h
   /usr/include
   /usr/local/include
)

FIND_LIBRARY(LDAP_LIBRARIES NAMES ldap
   PATHS
   /usr/lib
   /usr/local/lib
)

FIND_LIBRARY(LBER_LIBRARIES NAMES lber
   PATHS
   /usr/lib
   /usr/local/lib
)


if(LDAP_INCLUDE_DIR AND LDAP_LIBRARIES)
   set(LDAP_FOUND TRUE)
endif(LDAP_INCLUDE_DIR AND LDAP_LIBRARIES)


if(LDAP_FOUND)
   if(NOT LDAP_FIND_QUIETLY)
      message(STATUS "Found ldap: ${LDAP_LIBRARIES} ${LBER_LIBRARIES}")
   endif(NOT LDAP_FIND_QUIETLY)
endif(LDAP_FOUND)

MARK_AS_ADVANCED(LDAP_INCLUDE_DIR LDAP_LIBRARIES LBER_LIBRARIES)
