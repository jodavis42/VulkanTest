set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})
set(ExternalDir ${CurrentDirectory})

add_subdirectory(RapidJson)
add_subdirectory(stb)
add_subdirectory(tinyobjloader)
add_subdirectory(glfw-3.3.bin.WIN64)
add_subdirectory(VulkanSDK)
add_subdirectory(SPIRV-Cross)
