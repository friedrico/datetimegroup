project(plasma-datetimegroup)
 
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
set(datetimegroup_engine_SRCS datetimegrouplib.cpp)
 
kde4_add_plugin(plasma_engine_datetimegroup ${datetimegroup_engine_SRCS})
target_link_libraries(plasma_engine_datetimegroup
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS})
 
install(TARGETS plasma_engine_datetimegroup
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-engine-datetimegroup.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})


