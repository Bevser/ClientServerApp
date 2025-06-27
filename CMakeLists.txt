cmake_minimum_required(VERSION 3.19)
project(ClientServerApp LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

include(GNUInstallDirs)

add_subdirectory(ClientApp)
add_subdirectory(ServerApp)
