


########### install files ###############

install( FILES kde-main.directory  kde-development.directory  kde-editors.directory  kde-edutainment.directory  kde-games-arcade.directory  kde-games-board.directory  kde-games-card.directory  kde-games-kidsgames.directory  kde-games-roguelikes.directory  kde-games-strategy.directory  kde-games.directory  kde-graphics.directory  kde-internet-terminal.directory  kde-internet.directory  kde-more.directory  kde-multimedia.directory  kde-office.directory  kde-science.directory  kde-settingsmenu.directory  kde-system-terminal.directory  kde-system.directory  kde-toys.directory  kde-unknown.directory  kde-utilities-xutils.directory  kde-utilities.directory  kde-utilities-accessibility.directory  kde-utilities-desktop.directory  kde-utilities-peripherals.directory  kde-utilities-file.directory  kde-utilities-pim.directory  kde-development-translation.directory  kde-development-webdevelopment.directory  DESTINATION  ${XDG_DIRECTORY_DIR} )




#original Makefile.am contents follow:

#xdg_directory_DATA = \
#kde-main.directory \
#kde-development.directory \
#kde-editors.directory \
#kde-edutainment.directory \
#kde-games-arcade.directory \
#kde-games-board.directory \
#kde-games-card.directory \
#kde-games-kidsgames.directory \
#kde-games-roguelikes.directory \
#kde-games-strategy.directory \
#kde-games.directory \
#kde-graphics.directory \
#kde-internet-terminal.directory \
#kde-internet.directory \
#kde-more.directory \
#kde-multimedia.directory \
#kde-office.directory \
#kde-science.directory \
#kde-settingsmenu.directory \
#kde-system-terminal.directory \
#kde-system.directory \
#kde-toys.directory \
#kde-unknown.directory \
#kde-utilities-xutils.directory \
#kde-utilities.directory \
#kde-utilities-accessibility.directory \
#kde-utilities-desktop.directory \
#kde-utilities-peripherals.directory \
#kde-utilities-file.directory \
#kde-utilities-pim.directory \
#kde-development-translation.directory \
#kde-development-webdevelopment.directory
#
#install-data-local:
#	for file in `cat $(srcdir)/old_desktop_files` ; do \
#	    if test -f $(DESTDIR)$(xdg_appsdir)/$${file} ; then \
#	        rm $(DESTDIR)$(xdg_appsdir)/$${file} ; \
#		echo Deleting $(DESTDIR)$(xdg_appsdir)/$${file} ; \
#	    fi; \
#	done;
#	
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/.hidden
#	$(INSTALL_DATA) $(srcdir)/hidden.directory $(DESTDIR)$(kde_appsdir)/.hidden/.directory
#
#uninstall-local:
#	-rm -f $(DESTDIR)$(kde_appsdir)/.hidden/.directory
