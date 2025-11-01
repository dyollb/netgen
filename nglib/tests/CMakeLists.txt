add_executable(ng_stl test_ng_stl.cpp)
target_link_libraries(ng_stl nglib)

# Apply sanitizers using the cross-platform macro
add_sanitizers_to_target(ng_stl)
