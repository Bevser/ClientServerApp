cmake_minimum_required(VERSION 3.16)

project(ClientApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

add_executable(ClientApp
  main.cpp
  clientprotocol.h
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
