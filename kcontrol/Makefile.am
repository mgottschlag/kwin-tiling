
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
SUBDIRS = bell background samba krdb input kdm\
	locale launch dateandtime style kded arts kcontrol knotify\
	pics $(ACCESS_SUBDIR) colors energy fonts \
	icons screensaver crypto smserver keys \
	$(KSMCARD_SUBDIR) $(FONTINST_SUBDIR) $(RANDR_SUBDIR) \
	componentchooser xinerama kthememanager hardware
