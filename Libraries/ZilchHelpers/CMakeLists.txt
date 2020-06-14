set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(ZilchHelpers "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(ZilchHelpers
    PRIVATE
    ${manualAdditions}
)

target_include_directories(ZilchHelpers
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
)

ADD_MSVC_PRECOMPILED_HEADER(ZilchHelpers "Precompiled.hpp" "Precompiled.cpp")
Set_Common_TargetCompileOptions(ZilchHelpers)

target_link_libraries(ZilchHelpers
                      PUBLIC
                      Zilch
)

set_target_properties(ZilchHelpers PROPERTIES FOLDER "Libraries")
