MAINTAINERCLEANFILES = Makefile.in

filesdir = $(datadir)/libphone-ui-shr/
files_DATA = widgets.edj

EXTRA_DIST = \
	widgets.edc \
	contacts_slide_call.png \
	contacts_slide_message.png \
	contacts_slide_open.png

clean-local:
	rm -f *.edj

widgets.edj: widgets.edc
	@edje_cc@ widgets.edc widgets.edj

