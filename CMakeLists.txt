cmake_minimum_required(VERSION 3.10)

# set the project name
project(transport-catalogue CXX)

set(CMAKE_CXX_STANDARD 17)
include_directories(transport-catalogue)

set(CMAKE_PREFIX_PATH /Users/fitopisya/Documents/protobuf)
set(Protobuf_PROTOC_EXECUTABLE /usr/bin/protoc)
set(-DBUILD_DEPS=ON)
# Эта команда найдёт собранный нами пакет Protobuf.
# REQUIRED означает, что библиотека обязательна.
# Путь для поиска укажем в параметрах команды cmake.
find_package(Protobuf REQUIRED)
# Помимо Protobuf, понадобится библиотека Threads
find_package(Threads REQUIRED)

# Команда вызова protoc.
# Ей переданы названия переменных, в которые будут сохранены
# списки сгенерированных файлов, а также сам proto-файл.
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto graph.proto transport_router.proto map_renderer.proto svg.proto)

set(HEADER_FILES domain.h geo.h graph.h json_builder.h json_reader.h json.h map_renderer.h ranges.h request_handler.h router.h
        serialization.h svg.h transport_catalogue.h transport_router.h)
set(CXX_FILES main.cpp json_reader.cpp domain.cpp transport_catalogue.cpp map_renderer.cpp request_handler.cpp
        svg.cpp json.cpp geo.cpp json_builder.cpp transport_router.cpp serialization.cpp)

# add the executable
add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${HEADER_FILES} ${CXX_FILES})

# find_package определила переменную Protobuf_INCLUDE_DIRS,
# которую нужно использовать как include-путь.
# Также нужно добавить как include-путь директорию, куда
# protoc положит сгенерированные файлы.
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Также find_package определила Protobuf_LIBRARY.
# Protobuf зависит от библиотеки Threads. Добавим и её при компоновке.
target_link_libraries(transport_catalogue ${Protobuf_LIBRARY} Threads::Threads)
target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
