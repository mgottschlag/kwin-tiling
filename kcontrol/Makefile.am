
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

if include_kcontrol_randr
RANDR_SUBDIR=randr
endif

if include_kcontrol_joystick
JOYSTICK_SUBDIR=joystick
endif

# 'colors' and 'fonts' should come after 'krdb'
SUBDIRS = bell background dnssd filetypes samba krdb input info ioslaveinfo kdm\
	kicker locale launch keys kio konq style kded \
	konqhtml ebrowsing arts kcontrol knotify\
	clock pics $(ACCESS_SUBDIR) colors energy fonts css \
	icons screensaver taskbar crypto privacy smserver \
	konsole spellchecking $(USBVIEW_SUBDIR) \
	$(KSMCARD_SUBDIR) nics $(FONTINST_SUBDIR) $(RANDR_SUBDIR) \
	componentchooser performance xinerama $(VIEW1394_SUBDIR) display kthememanager \
        $(JOYSTICK_SUBDIR)
