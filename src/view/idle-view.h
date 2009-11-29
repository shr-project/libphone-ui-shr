#ifndef _IDLE_VIEW_H
#define _IDLE_VIEW_H
#include <Evas.h>
#include <glib.h>
#include "window.h"
#include <phoneui/phoneui.h>

void *idle_screen_view_show(struct Window *win, GHashTable * options);

void idle_screen_view_hide(struct Window *win);

void idle_screen_view_update(enum PhoneuiIdleScreenRefresh type);
#endif
