# Once done this will define
#
#  KMETADATA_FOUND - system has KMetaData
#  KMETADATA_INCLUDES - the KMetaData include directory
#  KMETADATA_LIBRARIES - Link these to use KMetaData
#

FIND_PATH(KMETADATA_INCLUDES
  NAMES
  kmetadata/kmetadata.h
  PATHS
  /usr/include
  /usr/local/include
  ${KDE4_INCLUDE_DIR}
  ${INCLUDE_INSTALL_DIR}
)

FIND_LIBRARY(KMETADATA_LIBRARIES 
  NAMES
  kmetadata
  PATHS
  /usr/lib
  /usr/local/lib
  ${KDE4_LIB_DIR}
  ${LIB_INSTALL_DIR}
)

if(KMETADATA_INCLUDES AND KMETADATA_LIBRARIES)
  set(KMetaData_FOUND TRUE)
endif(KMETADATA_INCLUDES AND KMETADATA_LIBRARIES)

if(KMetaData_FOUND)
  if(NOT KMetaData_FIND_QUIETLY)
    message(STATUS "Found KMetaData: ${KMETADATA_LIBRARIES}")
  endif(NOT KMetaData_FIND_QUIETLY)
else(KMetaData_FOUND)
  if(KMetaData_FIND_REQUIRED)
    if(NOT KMETADATA_INCLUDES)
      message(FATAL_ERROR "Could not find KMetaData includes.")
    endif(NOT KMETADATA_INCLUDES)
    if(NOT KMETADATA_LIBRARIES)
      message(FATAL_ERROR "Could not find KMetaData library.")
    endif(NOT KMETADATA_LIBRARIES)
  endif(KMetaData_FIND_REQUIRED)
endif(KMetaData_FOUND)
