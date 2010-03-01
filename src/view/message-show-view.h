#ifndef _MESSAGE_SHOW_VIEW_H
#define _MESSAGE_SHOW_VIEW_H

#include <glib.h>
#include <Elementary.h>
#include "ui-utils.h"

struct MessageShowViewData {
	struct View parent;
	char *path, *number, *name, *photopath;
	GHashTable *properties;
	Evas_Object *content, *photo, *sc_content, *bt1, *bt2, *bt3, *hv, *bx,
		    *hbt1, *hbt2, *hbt3;

	void (*callback) ();
	void *callback_data;
};

int message_show_view_init(char *path, GHashTable *properties);
int message_show_view_is_init(const char *path);
void message_show_view_deinit(struct MessageShowViewData *view);
void message_show_view_show(const char *path);
void message_show_view_hide(const char *path);

#endif