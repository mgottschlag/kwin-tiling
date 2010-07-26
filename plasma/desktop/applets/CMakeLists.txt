add_subdirectory(kickoff)
add_subdirectory(trash)
add_subdirectory(tasks)
add_subdirectory(windowlist)

if(NOT WIN32)
   add_subdirectory(pager)
endif(NOT WIN32)
