add_subdirectory(kdm)
add_subdirectory(kephal)
add_subdirectory(plasmaclock)
add_subdirectory(solid)

if(NOT WIN32)
 add_subdirectory(taskmanager)
 add_subdirectory(ksysguard)
 add_subdirectory(kworkspace)
endif(NOT WIN32)

if(Nepomuk_FOUND)
  add_subdirectory(nepomukquery)
  add_subdirectory(nepomukqueryclient)
endif(Nepomuk_FOUND)

