add_subdirectory(kickoff)
add_subdirectory(trash)

if(NOT WIN32)
   add_subdirectory(pager)
   add_subdirectory(tasks)
   add_subdirectory(windowlist)
endif(NOT WIN32)
