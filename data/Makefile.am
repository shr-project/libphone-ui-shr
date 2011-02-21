SUBDIRS = idle_screen widgets

MAINTAINERCLEANFILES = Makefile.in

filesdir = $(datadir)/libphone-ui-shr/
files_DATA = default.edj \
	call-start.png call-stop.png \
	contact.png \
	edit-undo.png \
	ilist_1.png ilist_2.png ilist_item_shadow.png \
	number-icon.png \
	phone-openmoko-freerunner.png \
	pl-all.png pl-in.png pl-missed.png pl-out.png

EXTRA_DIST = \
	default.edc \
	call-management.edc \
	keypad.edc \
	phonelog.edc \
	colors.edc \
	dialer.edc \
	messages.edc \
	quick-settings.edc \
	contacts.edc \
	icons.edc \
	notification.edc \
	sim-manager.edc \
	styles.edc \
	DejaVuSans-Bold.ttf \
	DejaVuSans.ttf \
	call-start.png call-stop.png \
	contact.png \
	edit-undo.png \
	ilist_1.png ilist_2.png ilist_item_shadow.png \
	number-icon.png \
	phone-openmoko-freerunner.png \
	pl-all.png pl-in.png pl-missed.png pl-out.png

clean-local:
	rm -f *.edj

default.edj: default.edc call-management.edc keypad.edc phonelog.edc colors.edc dialer.edc messages.edc quick-settings.edc contacts.edc icons.edc notification.edc styles.edc
	@edje_cc@ default.edc

