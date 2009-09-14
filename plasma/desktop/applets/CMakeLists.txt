add_subdirectory(kickoff)
add_subdirectory(trash)


if(NOT WIN32)
   add_subdirectory(pager)
   add_subdirectory(tasks)
endif(NOT WIN32)
