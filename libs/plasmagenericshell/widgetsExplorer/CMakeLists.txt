project(appletBrowser)

cmake_minimum_required(VERSION 2.6)

#find_package(Qt4 REQUIRED)
#include(${QT_USE_FILE})

set(CMAKE_MODULE_PATH ${HOME}/development/KDEBuilder/install/share/apps/cmake/modules ${CMAKE_MODULE_PATH})
#set(CMAKE_MODULE_PATH ${HOME}/development/KDE/install_dir/share/apps/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(QT_INCLUDE_DIR ${HOME}/development/qt/install/include)
#set(QT_BINARY_DIR ${HOME}/development/qt/install/bin)
#set(QT_LIBRARY_DIR ${HOME}/development/qt/install/lib)

find_package(KDE4 REQUIRED)

INCLUDE_DIRECTORIES(${KDE4_INCLUDES} ${HOME}/development/workspace/plasmaOnMyBranch/soc-plasma-appletbrowser/shells/common/newWidgetsExplorer/build)

AUTOMOC4_ADD_EXECUTABLE(appletBrowser
	       "kcategorizeditemsviewmodels.cpp"
	       "plasmaappletitemmodel.cpp"
	       "appleticon.cpp"
	       "appletsfiltering.cpp"
	       "applettooltip.cpp"
	       "appletslist.cpp"
	       "widgetexplorer.cpp"
	       "openwidgetassistant.cpp"
	       "main.cpp"
	       "kcategorizeditemsviewmodels_p.h"
	       "plasmaappletitemmodel_p.h"
	       "appleticon.h"
	       "appletsfiltering.h"
	       "applettooltip.h"
	       "appletslist.h"
	       "widgetexplorer.h"
	       "openwidgetassistant_p.h")

target_link_libraries(appletBrowser ${QT_LIBRARIES} ${KDE4_PLASMA_LIBS})
