set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(SimpleMath "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(SimpleMath
    PRIVATE
    ${manualAdditions}
)

target_include_directories(SimpleMath
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
    $ENV{VULKAN_SDK}/Include
)

ADD_MSVC_PRECOMPILED_HEADER(SimpleMath "Precompiled.hpp" "Precompiled.cpp")
Set_Common_TargetCompileOptions(SimpleMath)

target_link_libraries(SimpleMath
                      PUBLIC
)

set_target_properties(SimpleMath PROPERTIES FOLDER "Libraries")
