project(appletBrowser)

cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
#include(${QT_USE_FILE})

set(CMAKE_MODULE_PATH ${HOME}/Development/KDEBuilder/install/share/apps/cmake/modules ${CMAKE_MODULE_PATH})
find_package(KDE4 REQUIRED)
INCLUDE_DIRECTORIES(${KDE4_INCLUDES})

AUTOMOC4_ADD_EXECUTABLE(appletBrowser
	       "standardcustomwidget.cpp"
	       "customwidgets.cpp"
	       "plasmaappletitemmodel.cpp"
	       "kcategorizeditemsviewdelegate.cpp"
	       "customdragtreeview.cpp"
	       "kcategorizeditemsview.cpp"
	       "kcategorizeditemsviewmodels.cpp"
	       "openwidgetassistant.cpp"
	       "widgetexplorer.cpp"
               "main.cpp"
               "standardcustomwidget.h"
               "customwidgets.h"
	       "plasmaappletitemmodel_p.h"
	       "kcategorizeditemsviewdelegate_p.h"
	       "customdragtreeview_p.h"
	       "ui_kcategorizeditemsviewbase.h"
	       "kcategorizeditemsview_p.h"
	       "kcategorizeditemsviewmodels_p.h"
	       "openwidgetassistant_p.h"
	       "widgetexplorer.h")

target_link_libraries(appletBrowser ${QT_LIBRARIES} ${KDE4_PLASMA_LIBS})
