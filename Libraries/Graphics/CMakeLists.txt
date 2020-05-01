set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(Graphics "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(Graphics
    PRIVATE
    ${manualAdditions}
)

target_include_directories(Graphics
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
)

ADD_MSVC_PRECOMPILED_HEADER(Graphics "Precompiled.hpp" "Precompiled.cpp")
Set_Common_TargetCompileOptions(Graphics)

target_link_libraries(Graphics
                      PUBLIC
                      Utilities
                      Math
                      Vulkan
                      StbImage
                      TinyObjLoader
                      GLFW
                      VulkanSDK
                      SPIRV-Cross
)

set_target_properties(Graphics PROPERTIES FOLDER "Libraries")
