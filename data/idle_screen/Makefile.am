MAINTAINERCLEANFILES = Makefile.in

filesdir = $(datadir)/libphone-ui-shr/
files_DATA = idle_screen.edj

EXTRA_DIST = \
	idle_screen.edc \
	Miso.ttf \
	VeraBd.ttf \
	alarm.png \
	background.png \
	bat_no.png bat_10.png bat_20.png bat_40.png bat_60.png bat_80.png bat_100.png \
	bluetooth.png \
	bottom.png \
	clock.png \
	cpu.png \
	display.png \
	gps.png \
	gsm_no.png gsm_0.png gsm_1.png gsm_2.png gsm_3.png gsm_4.png \
	logo.png \
	missedCalls.png \
	slider.png sliderbackground_left.png sliderbackground_middle.png sliderbackground_right.png \
	unfinishedTasks.png \
	unreadMessages.png \
	wifi.png

clean-local:
	rm -f *.edj

idle_screen.edj: idle_screen.edc
	@edje_cc@ idle_screen.edc idle_screen.edj

