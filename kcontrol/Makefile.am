# $Id$

if include_kcontrol_access
ACCESS_SUBDIR=access
endif

if include_kcontrol_kfontinst
FONTINST_SUBDIR=kfontinst
endif

if include_kcontrol_smartcard
KSMCARD_SUBDIR=smartcard
endif

if include_kcontrol_usbview
USBVIEW_SUBDIR=usbview
endif

if include_kcontrol_view1394
VIEW1394_SUBDIR=view1394
endif

# 'colors' and 'fonts' should come after 'krdb'
SUBDIRS = bell background filetypes samba krdb input info ioslaveinfo kdm\
	kicker locale launch keys kio konq thememgr style kded \
	konqhtml email ebrowsing arts kcontrol passwords knotify\
	clock pics $(ACCESS_SUBDIR) colors energy fonts css \
	icons screensaver taskbar crypto privacy smserver \
	konsole spellchecking fileshare $(USBVIEW_SUBDIR) \
	$(KSMCARD_SUBDIR) nics $(FONTINST_SUBDIR) $(RANDR_SUBDIR) \
	componentchooser performance xinerama $(VIEW1394_SUBDIR)
