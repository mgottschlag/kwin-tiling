project(appletBrowser)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
#include(${QT_USE_FILE})

#set(CMAKE_MODULE_PATH ${HOME}/development/KDEBuilder/install/share/apps/cmake/modules ${CMAKE_MODULE_PATH})
#set(CMAKE_MODULE_PATH ${HOME}/development/KDE/install_dir/share/apps/cmake/modules ${CMAKE_MODULE_PATH})
find_package(KDE4 REQUIRED)
INCLUDE_DIRECTORIES(${KDE4_INCLUDES})

AUTOMOC4_ADD_EXECUTABLE(appletBrowser
	       "kcategorizeditemsviewmodels.cpp"
	       "plasmaappletitemmodel.cpp"
	       "customwidgets.cpp"
	       "widgetexplorer.cpp"
	       "openwidgetassistant.cpp"
	       "main.cpp"
	       "kcategorizeditemsviewmodels_p.h"
	       "plasmaappletitemmodel_p.h"
	       "customwidgets.h"
	       "widgetexplorer.h"
	       "openwidgetassistant_p.h")

target_link_libraries(appletBrowser ${QT_LIBRARIES} ${KDE4_PLASMA_LIBS})
