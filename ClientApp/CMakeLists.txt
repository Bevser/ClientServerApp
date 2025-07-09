cmake_minimum_required(VERSION 3.16)
project(ClientApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup(REQUIRES 6.5)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")

qt_add_executable(ClientApp
    main.cpp
    clientprotocol.h
    ${app_icon_resource_windows}
)

target_link_libraries(ClientApp PRIVATE
    Qt6::Core
    Qt6::Network
)

target_sources(ClientApp
  PRIVATE
    clientlogic.h clientlogic.cpp
    ../common/protocol.h
    ../common/iclient.h
    ../common/tcpclient.h
    ../common/tcpclient.cpp
)

include(GNUInstallDirs)
install(TARGETS ClientApp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
