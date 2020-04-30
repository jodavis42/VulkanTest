set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(StbImage INTERFACE)

target_include_directories(StbImage 
    INTERFACE 
    ${CurrentDirectory}
    ${ExternalDir}
)
