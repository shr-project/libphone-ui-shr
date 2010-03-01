#ifndef _MESSAGE_NEW_VIEW_H
#define _MESSAGE_NEW_VIEW_H

#include <glib.h>
#include <Evas.h>

struct MessageNewViewData {
	struct View view;
	int mode;
	char *content;
	Evas_Object *pager, *left_button, *right_button;
	Evas_Object *box_content, *layout_content;
	Evas_Object *box_recipients, *layout_recipients;
	Evas_Object *entry, *hv, *sc;
	Evas_Object *list_contacts;
	Evas_Object *list_recipients;
	Evas_Object *information;

	GPtrArray *recipients;

	struct ContactListViewData *cdata;

	int messages_sent;
};

struct MessageNewViewData *message_new_view_init(GHashTable *options);
void message_new_view_deinit(struct MessageNewViewData *view);
void message_new_view_show(struct MessageNewViewData *view);

#endif