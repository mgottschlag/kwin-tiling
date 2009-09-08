## Private Library

set(polkit_private_LIB_SRCS
    PkKAuthorization.cpp
    PkKAction.cpp
    PkKStrings.cpp
    AuthorizationsFilterModel.cpp
    ExplicitAuthorizationsModel.cpp
    PkKBlockGrantAuth.cpp
    pkitemdelegate.cpp
    PoliciesModel.cpp
    PolicyItem.cpp
)

kde4_add_ui_files(polkit_private_LIB_SRCS
    PkKAuthorization.ui
    PkKAction.ui
    PkKBlockGrantAuth.ui
)

kde4_add_library(polkitkdeprivate SHARED ${polkit_private_LIB_SRCS})

target_link_libraries(polkitkdeprivate
	${KDE4_KDEUI_LIBS}
	${POLKITDBUS_LIBRARIES}
	${POLKITGRANT_LIBRARIES}
	${POLKIT_LIBRARIES}
	${POLKITQT_LIBRARIES}
	${POLKITQT_CORE_LIBRARY}
)

set_target_properties(polkitkdeprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS polkitkdeprivate  ${INSTALL_TARGETS_DEFAULT_ARGS})

## Auth Application

set(polkit_authorization_SRCS
    main.cpp
    PkKAuthorizationDialog.cpp
)

kde4_add_executable(polkit-kde-authorization ${polkit_authorization_SRCS})
target_link_libraries(polkit-kde-authorization polkitkdeprivate ${KDE4_KDEUI_LIBS})

install(TARGETS polkit-kde-authorization ${INSTALL_TARGETS_DEFAULT_ARGS})

# kcm module

set(pkk_authorization_SRCS
    KcmPkKAuthorization.cpp
)

kde4_add_plugin(kcm_pkk_authorization ${pkk_authorization_SRCS})
target_link_libraries(kcm_pkk_authorization polkitkdeprivate ${KDE4_KUTILS_LIBS} )

install( FILES kcm_pkk_authorization.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

install(TARGETS kcm_pkk_authorization DESTINATION ${PLUGIN_INSTALL_DIR} )
