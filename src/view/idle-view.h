#ifndef _IDLE_VIEW_H
#define _IDLE_VIEW_H
#include <Evas.h>
#include <glib.h>
#include "window.h"
#include <phoneui/phoneui.h>

struct IdleScreenViewData {
	struct Window *win;
	Evas_Object *wallpaper;
};

struct IdleScreenViewData *
idle_screen_view_show(struct Window *win, GHashTable *options);

void
idle_screen_view_hide(struct IdleScreenViewData *data);

void
idle_screen_view_update(enum PhoneuiIdleScreenRefresh type, struct Window *win);
#endif
