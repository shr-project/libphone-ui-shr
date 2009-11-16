MAINTAINERCLEANFILES = Makefile.in

filesdir = $(datadir)/libphone-ui-shr/
files_DATA = default.edj edit-undo.png contact.png number-icon.png call-start.png call-stop.png

EXTRA_DIST = default.edc

clean-local:
	rm -f *.edj

default.edj: default.edc
	@edje_cc@ default.edc

