add_subdirectory(bookmarks)
add_subdirectory(calculator)
add_subdirectory(locations)
add_subdirectory(places)
add_subdirectory(services)
add_subdirectory(recentdocuments)
add_subdirectory(shell)
add_subdirectory(webshortcuts)

if(Nepomuk_FOUND)
  add_subdirectory(nepomuksearch)
endif(Nepomuk_FOUND)

if(NOT WIN32)
add_subdirectory(powerdevil)
add_subdirectory(sessions)
add_subdirectory(windows)
endif(NOT WIN32)

