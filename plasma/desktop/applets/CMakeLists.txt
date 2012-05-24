add_subdirectory(kickoff)
add_subdirectory(showActivityManager)
add_subdirectory(trash)

if(NOT WIN32)
    #task and windowlist depend on libs/taskmanager
   add_subdirectory(tasks)
   add_subdirectory(windowlist)

   add_subdirectory(pager)
endif(NOT WIN32)
