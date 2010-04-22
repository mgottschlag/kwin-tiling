add_subdirectory(kickoff)
add_subdirectory(trash)
add_subdirectory(tasks)

if(NOT WIN32)
   add_subdirectory(pager)
   add_subdirectory(windowlist)
endif(NOT WIN32)
