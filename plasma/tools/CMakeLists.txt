macro_optional_find_package(Soprano)
macro_log_feature(Soprano_FOUND "Soprano" "Soprano Libraries" "kdesupport" FALSE "" "Required to build the Plasma Engine Explorer.")

if (Soprano_FOUND)
  add_subdirectory(engineexplorer)
endif (Soprano_FOUND)

add_subdirectory(plasmapkg)
add_subdirectory(plasmoidviewer)
