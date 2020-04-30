set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(TinyObjLoader INTERFACE)

target_include_directories(TinyObjLoader 
    INTERFACE 
    ${CurrentDirectory}
    ${ExternalDir}
)
