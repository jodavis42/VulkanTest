set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})
set(LibrariesDir ${CurrentDirectory})

add_subdirectory(Utilities)
add_subdirectory(Engine)
add_subdirectory(Resources)
add_subdirectory(Vulkan)
add_subdirectory(Graphics)
add_subdirectory(Application)
