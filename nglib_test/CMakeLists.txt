include_directories(${CMAKE_SOURCE_DIR}/nglib)

add_executable(ng_refine ng_refine.cpp)
target_link_libraries(ng_refine nglib)

add_executable(ng_stl ng_stl.cpp)
target_link_libraries(ng_stl nglib)
