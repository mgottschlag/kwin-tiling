kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdm/sessions FILES kde.desktop gnome.desktop      9wm.desktop      aewm++.desktop      aewm.desktop      afterstep.desktop      amaterus.desktop      amiwm.desktop      asclassic.desktop      blackbox.desktop      cde.desktop      ctwm.desktop      cwwm.desktop      enlightenment.desktop      evilwm.desktop      fluxbox.desktop      flwm.desktop      fvwm.desktop      fvwm95.desktop      golem.desktop      icewm.desktop      ion.desktop      larswm.desktop      lwm.desktop      matchbox.desktop      metacity.desktop      mwm.desktop      olvwm.desktop      olwm.desktop      openbox.desktop      oroborus.desktop      phluid.desktop      pwm.desktop      qvwm.desktop      ratpoison.desktop      sapphire.desktop      sawfish.desktop      twm.desktop      ude.desktop      vtwm.desktop      w9wm.desktop      waimea.desktop      wm2.desktop      wmaker.desktop      xfce.desktop      xfce4.desktop )

kde4_footer()



#original Makefile.am contents follow:

#sessionsdir = $(kde_datadir)/kdm/sessions
#sessions_DATA = \
#    kde.desktop gnome.desktop \
#    9wm.desktop \
#    aewm++.desktop \
#    aewm.desktop \
#    afterstep.desktop \
#    amaterus.desktop \
#    amiwm.desktop \
#    asclassic.desktop \
#    blackbox.desktop \
#    cde.desktop \
#    ctwm.desktop \
#    cwwm.desktop \
#    enlightenment.desktop \
#    evilwm.desktop \
#    fluxbox.desktop \
#    flwm.desktop \
#    fvwm.desktop \
#    fvwm95.desktop \
#    golem.desktop \
#    icewm.desktop \
#    ion.desktop \
#    larswm.desktop \
#    lwm.desktop \
#    matchbox.desktop \
#    metacity.desktop \
#    mwm.desktop \
#    olvwm.desktop \
#    olwm.desktop \
#    openbox.desktop \
#    oroborus.desktop \
#    phluid.desktop \
#    pwm.desktop \
#    qvwm.desktop \
#    ratpoison.desktop \
#    sapphire.desktop \
#    sawfish.desktop \
#    twm.desktop \
#    ude.desktop \
#    vtwm.desktop \
#    w9wm.desktop \
#    waimea.desktop \
#    wm2.desktop \
#    wmaker.desktop \
#    xfce.desktop \
#    xfce4.desktop
#
#EXTRA_DIST = $(sessions_DATA)
