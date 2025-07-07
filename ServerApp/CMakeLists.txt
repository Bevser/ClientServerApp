cmake_minimum_required(VERSION 3.16)

project(ServerApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

set(CMAKE_AUTORCC ON)

qt_add_executable(Server
    main.cpp
    tcpserver.cpp
    tcpserver.h
    serverviewmodel.cpp
    serverviewmodel.h
)

qt_add_qml_module(Server
    URI ServerApp
    VERSION 1.0
    QML_FILES
        Main.qml
        QML_FILES UniversalTable.qml
        SOURCES tcpclient.h tcpclient.cpp
        SOURCES iclient.h
        SOURCES
        SOURCES iserver.h
        SOURCES protocol.h
        SOURCES dataprocessing.h dataprocessing.cpp
        SOURCES serverfactory.h
        SOURCES sharedkeys.h
        SOURCES appenums.h
        QML_FILES ConfigurationDialog.qml
        SOURCES serverworker.h serverworker.cpp
        SOURCES tablemodel.h tablemodel.cpp
        QML_FILES ServerManagementDialog.qml
        SOURCES serverlistmodel.h serverlistmodel.cpp
        RESOURCES resource.qrc
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(Server PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Server
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(Server PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS Server
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
