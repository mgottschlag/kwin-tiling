FIND_PACKAGE (Qt4)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (. ${QT_INCLUDES})

SET (kephalxml_SRCS xmltype.cpp configurations_xml.cpp xmlnodehandler.cpp)
QT4_WRAP_CPP (kephalxml_SRCS xmltype.h configurations_xml.h xmlnodehandler.h)
ADD_LIBRARY (kephalxml ${kephalxml_SRCS})

