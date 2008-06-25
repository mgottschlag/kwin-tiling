set(cursors_folders
    Oxygen_Black
    Oxygen_Black_Big
    Oxygen_Blue
    Oxygen_Blue_Big
    Oxygen_White
    Oxygen_White_Big
    Oxygen_Yellow
    Oxygen_Yellow_Big
    Oxygen_Zion
    Oxygen_Zion_Big
   )

foreach(theme ${cursors_folders})
    install(DIRECTORY ${theme} DESTINATION ${ICON_INSTALL_DIR} PATTERN .svn EXCLUDE)
endforeach(theme)


