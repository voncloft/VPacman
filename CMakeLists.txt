cmake_minimum_required(VERSION 3.21)
project(VPacman VERSION 4.3.4 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Install prefix" FORCE)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
include(GNUInstallDirs)
find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

add_executable(VPacman
    untitled1/main.cpp
    untitled1/mainwindow.cpp
    untitled1/mainwindow.h
    untitled1/mainwindow.ui
    resources.qrc
)
target_link_libraries(VPacman PRIVATE Qt6::Widgets)
target_compile_definitions(VPacman PRIVATE VPACMAN_VERSION="${PROJECT_VERSION}")
install(TARGETS VPacman RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
configure_file(vpacman.desktop.in vpacman.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vpacman.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES assets/vpacman.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/vpacman)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(vpacman_test tests/vpacman_test.cpp untitled1/mainwindow.cpp untitled1/mainwindow.h untitled1/mainwindow.ui resources.qrc)
    target_include_directories(vpacman_test PRIVATE untitled1)
    target_link_libraries(vpacman_test PRIVATE Qt6::Widgets Qt6::Test)
    target_compile_definitions(vpacman_test PRIVATE VPACMAN_VERSION="${PROJECT_VERSION}")
    add_test(NAME vpacman COMMAND vpacman_test)
    set_tests_properties(vpacman PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
