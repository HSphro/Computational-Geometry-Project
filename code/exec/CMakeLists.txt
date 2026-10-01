
# ADD HERE THE NAME OF THE FILE (without the .c, 1 per line)
# This will create an ./exec/name executable for each "name" in the list
set(EXEC_LIST
    example_viewer
    ashape_adaptation_view
)

foreach(EXEC ${EXEC_LIST})
    add_executable(${EXEC})
    target_sources(${EXEC} PRIVATE "${EXEC}.c")
    target_link_libraries(${EXEC} PUBLIC geonum)
endforeach()
