MAINTAINERCLEANFILES = Makefile.in

filesdir = $(datadir)/libphone-ui-shr/
files_DATA = default.edj idle_screen.edj edit-undo.png contact.png number-icon.png call-start.png call-stop.png

EXTRA_DIST = default.edc

clean-local:
	rm -f *.edj

default.edj: default.edc call-management.edc keypad.edc phonelog.edc colors.edc dialer.edc messages.edc quick-settings.edc contacts.edc icons.edc notification.edc styles.edc
	@edje_cc@ default.edc

idle_screen.edj: idle_screen/idle_screen.edc
	cd idle_screen; @edje_cc@ idle_screen.edc ../idle_screen.edj

