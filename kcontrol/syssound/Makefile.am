
# just set the variable
#APPSDIR = $(kde_appsdir)/Settings/Sound
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

SUBDIRS = sounds

bin_PROGRAMS = 	kcmsyssound

kcmsyssound_SOURCES	= main.cpp syssound.cpp

kcmsyssound_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kcmsyssound_LDADD   =   -lmediatool $(LIB_KDEUI) $(LIBCOMPAT)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = syssound.h

# just to make sure, automake makes them 
METASOURCES =	syssound.moc

KDE_ICON = kcmsyssound
EXTRA_DIST = CHANGES

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmsyssound_SOURCES) && mv messages.po ../../po/kcmsyssound.pot

