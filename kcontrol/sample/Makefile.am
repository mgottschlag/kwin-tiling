
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI)

noinst_PROGRAMS = kcmsample

kcmsample_SOURCES = main.cpp

KDE_ICON = sample

messages:
	$(XGETTEXT) $(kcmsample_SOURCES) -o $(podir)/kcmsample.pot

