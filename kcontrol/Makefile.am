
if include_kcontrol_access
#ACCESS_SUBDIR=access
endif

if include_kcontrol_kfontinst
FONTINST_SUBDIR=kfontinst
endif

if include_kcontrol_smartcard
KSMCARD_SUBDIR=smartcard
endif

if include_kcontrol_randr
RANDR_SUBDIR=randr
endif

# readd kicker and taskbar later
# 'colors' and 'fonts' should come after 'krdb'
#
# FIXME : readd dnssd
#
# keys 
SUBDIRS = bell background samba krdb input kdm\
	locale launch style kded arts kcontrol knotify\
	clock pics $(ACCESS_SUBDIR) colors energy fonts \
	icons screensaver crypto privacy smserver \
	$(KSMCARD_SUBDIR) $(FONTINST_SUBDIR) $(RANDR_SUBDIR) \
	componentchooser xinerama kthememanager
