SUBDIRS = samba bell display input kwm info kdm kcontrol kpanel \
	locale syssound keys sample $(ODBC_DIR) kio konq email

messages:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	for subdir in $(SUBDIRS) odbc; do \
	echo "Making messages in $$subdir"; \
	  (cd $$subdir && $(MAKE) -f Makefile.am messages) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"

