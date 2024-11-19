# file(GLOB SOURCE *.cpp)
set(SOURCE
main.cpp
domain.cpp
geo.cpp
json_builder.cpp 
json.cpp
json_reader.cpp
map_renderer.cpp
request_handler.cpp
svg.cpp
transport_catalogue.cpp
transport_router.cpp
# input_reader.cpp
# stat_reader.cpp
)
add_executable(main ${SOURCE})
add_library(sut STATIC ${SOURCE})
target_include_directories(sut PUBLIC .)
