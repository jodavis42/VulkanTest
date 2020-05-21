set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(Vulkan "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(Vulkan
    PRIVATE
    ${manualAdditions}
)

target_include_directories(Vulkan
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
    $ENV{VULKAN_SDK}/Include
)

ADD_MSVC_PRECOMPILED_HEADER(Vulkan "Precompiled.hpp" "Precompiled.cpp")
Set_Common_TargetCompileOptions(Vulkan)

target_link_libraries(Vulkan
                      PUBLIC
                      GLFW
                      VulkanSDK
                      Common
                      ZilchShaders
                      Graphics
)

set_target_properties(Vulkan PROPERTIES FOLDER "Libraries")
