if include_kcontrol_access
ACCESS_SUBDIR=access
endif

if include_kcontrol_kfontinst
FONTINST_SUBDIR=kfontinst
endif

if include_kcontrol_smartcard
KSMCARD_SUBDIR=smartcard
endif

# 'colors' and 'fonts' should come after 'krdb'
SUBDIRS = bell background filetypes samba krdb input info ioslaveinfo kdm\
	kicker locale launch keys kio konq thememgr style kded \
	konqhtml email ebrowsing arts kcontrol passwords knotify\
	clock pics $(ACCESS_SUBDIR) colors energy fonts css \
	icons screensaver taskbar crypto midi smserver \
	socks konsole spellchecking fileshare usbview \
	$(KSMCARD_SUBDIR) nics $(FONTINST_SUBDIR) \
	componentchooser

