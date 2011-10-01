project(datetimegroup)
 
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
set(datetimegroup_SRCS datetimegroup.cpp dtgtzwidget.cpp)
kde4_add_ui_files(datetimegroup_SRCS dtgConfig.ui timezonesConfig.ui) 
kde4_add_plugin(plasma_applet_datetimegroup ${datetimegroup_SRCS})
target_link_libraries(plasma_applet_datetimegroup 
			${KDE4_PLASMA_LIBS} 
			plasmaclock
			${KDE4_KIO_LIBS}
			${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_datetimegroup
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-datetimegroup.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(dateTimeGroupEngine)
add_subdirectory(icons)