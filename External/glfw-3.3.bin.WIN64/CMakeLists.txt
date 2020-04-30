set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(GLFW STATIC IMPORTED GLOBAL)
set_target_properties(GLFW PROPERTIES IMPORTED_LOCATION ${CurrentDirectory}/lib-vc2017/glfw3.lib)
target_include_directories(GLFW INTERFACE ${CurrentDirectory} ${CurrentDirectory}/include)