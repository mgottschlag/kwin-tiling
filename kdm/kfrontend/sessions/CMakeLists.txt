configure_file(kde-plasma.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kde-plasma.desktop)
configure_file(kde-plasma-safe.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kde-plasma-safe.desktop)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/kde-plasma.desktop
	${CMAKE_CURRENT_BINARY_DIR}/kde-plasma-safe.desktop
	gnome.desktop
	9wm.desktop
	aewm++.desktop
	aewm.desktop
	afterstep.desktop
	amaterus.desktop
	amiwm.desktop
	asclassic.desktop
	blackbox.desktop
	cde.desktop
	ctwm.desktop
	cwwm.desktop
	e16.desktop
        enlightenment.desktop
	evilwm.desktop
	fluxbox.desktop
	flwm.desktop
	fvwm.desktop
	fvwm95.desktop
	golem.desktop
	icewm.desktop
	ion.desktop
	larswm.desktop
	lwm.desktop
	matchbox.desktop
	metacity.desktop
	mwm.desktop
	olvwm.desktop
	olwm.desktop
	openbox.desktop
	oroborus.desktop
	phluid.desktop
	pwm.desktop
	qvwm.desktop
	ratpoison.desktop
	sapphire.desktop
	sawfish.desktop
	twm.desktop
	ude.desktop
	vtwm.desktop
	w9wm.desktop
	waimea.desktop
	wm2.desktop
	wmaker.desktop
	xfce.desktop
	xfce4.desktop

	DESTINATION ${DATA_INSTALL_DIR}/kdm/sessions
)
