cmake_minimum_required(VERSION 3.0.0)
project(DeltaProject VERSION 0.1.0)

include(CTest)
enable_testing()

set(ExName delpro)
add_library(${ExName} SHARED delpro.cpp mainwindow.cpp plot.cpp textfield.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0)

target_include_directories(${ExName} PUBLIC ${GTKMM_INCLUDE_DIRS})
target_link_libraries(${ExName} ${GTKMM_LIBRARIES})

pkg_check_modules(PLPLOT plplot)

target_include_directories(${ExName} PUBLIC ${PLPLOT_INCLUDE_DIRS})
target_link_libraries(${ExName} plplotcxx)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

#add_custom_command(TARGET ${ExName} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:${ExName}> ${PROJECT_SOURCE_DIR}/${ExName}
#    COMMENT "Created ${PROJECT_SOURCE_DIR}/${ExName}"
#)
