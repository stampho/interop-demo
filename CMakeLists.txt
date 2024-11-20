cmake_minimum_required(VERSION 3.16)

project(interop-demo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appinterop-demo
    main.cpp
    textureitem.h textureitem.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appinterop-demo PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appinterop-demo
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appinterop-demo PRIVATE
    Qt6::GuiPrivate
    Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appinterop-demo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
