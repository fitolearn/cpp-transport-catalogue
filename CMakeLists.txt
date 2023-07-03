cmake_minimum_required(VERSION 3.23)
project(cpp_transport_catalogue)

set(CMAKE_CXX_STANDARD 17)

include_directories(transport-catalogue)

add_executable(cpp_transport_catalogue
        transport-catalogue/geo.h
        transport-catalogue/main.cpp
        transport-catalogue/transport_catalogue.cpp
        transport-catalogue/transport_catalogue.h
        transport-catalogue/geo.cpp
        transport-catalogue/domain.cpp
        transport-catalogue/domain.h
        transport-catalogue/json.cpp
        transport-catalogue/json.h
        transport-catalogue/json_reader.cpp
        transport-catalogue/json_reader.h
        transport-catalogue/map_renderer.cpp
        transport-catalogue/map_renderer.h
        transport-catalogue/request_handler.cpp
        transport-catalogue/request_handler.h
        transport-catalogue/svg.cpp
        transport-catalogue/svg.h
        transport-catalogue/json_builder.cpp
        transport-catalogue/json_builder.h transport-catalogue/graph.h transport-catalogue/ranges.h transport-catalogue/router.h transport-catalogue/transport_router.cpp transport-catalogue/transport_router.h)
