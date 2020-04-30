set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(RapidJson INTERFACE)

target_include_directories(RapidJson 
    INTERFACE 
    ${CurrentDirectory}
    ${ExternalDir}
)
