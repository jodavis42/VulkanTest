set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(Resources "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(Resources
    PRIVATE
    ${manualAdditions}
)

target_include_directories(Resources
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
)

ADD_MSVC_PRECOMPILED_HEADER(Resources "Precompiled.hpp" "Precompiled.cpp")
Set_Common_TargetCompileOptions(Resources)

target_link_libraries(Resources
                      PUBLIC
                      Utilities
                      Foundation
                      Memory
                      Zilch
                      ZilchHelpers
)

set_target_properties(Resources PROPERTIES FOLDER "Libraries")
