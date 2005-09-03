
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

# readd kicker and taskbar later
# 'colors' and 'fonts' should come after 'krdb'
#
# FIXME : readd dnssd
# 
SUBDIRS = bell background filetypes samba krdb input info ioslaveinfo kdm\
	locale launch keys kio style kded \
	arts kcontrol knotify\
	clock pics $(ACCESS_SUBDIR) colors energy fonts \
	icons screensaver crypto privacy smserver \
	 $(USBVIEW_SUBDIR) \
	$(KSMCARD_SUBDIR) nics $(FONTINST_SUBDIR) $(RANDR_SUBDIR) \
	componentchooser xinerama $(VIEW1394_SUBDIR) display kthememanager \
        $(JOYSTICK_SUBDIR)
