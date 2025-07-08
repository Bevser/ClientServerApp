cmake_minimum_required(VERSION 3.16)
project(ServerApp VERSION 0.1 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Network Qml Quick QuickControls2)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup(REQUIRES 6.5)

# C++ файлы
qt_add_executable(Server
    main.cpp

    core/tcpserver.cpp
    core/tcpserver.h
    core/serverworker.cpp
    core/serverworker.h
    core/dataprocessing.cpp
    core/dataprocessing.h
    core/serverfactory.h
    core/iserver.h
    core/sharedkeys.h
    core/appenums.h

    models/serverviewmodel.cpp
    models/serverviewmodel.h
    models/serverlistmodel.cpp
    models/serverlistmodel.h
    models/tablemodel.cpp
    models/tablemodel.h

    qml/resource.qrc

    ../common/tcpclient.h
    ../common/tcpclient.cpp
    ../common/iclient.h
    ../common/protocol.h
)

# QML модуль
qt_add_qml_module(Server
    URI ServerApp
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/UniversalTable.qml
        qml/ConfigurationDialog.qml
        qml/ServerManagementDialog.qml
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
