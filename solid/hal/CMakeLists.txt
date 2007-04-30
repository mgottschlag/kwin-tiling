
########### next target ###############

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../control )

set(solid_hal_power_PART_SRCS 
   halpower.cpp 
   halsuspendjob.cpp )

kde4_automoc(${solid_hal_power_PART_SRCS})

kde4_add_library(solid_hal_power_static STATIC ${solid_hal_power_PART_SRCS})
target_link_libraries(solid_hal_power_static ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrolifaces)

kde4_add_plugin(solid_hal_power module_power.cpp ${solid_hal_power_PART_SRCS})

target_link_libraries(solid_hal_power ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrolifaces)



install(TARGETS solid_hal_power DESTINATION ${PLUGIN_INSTALL_DIR}  )


########### install files ###############

install( FILES solid_hal_power.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )


