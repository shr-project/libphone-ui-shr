#ifndef _MESSAGE_NEW_VIEW_H
#define _MESSAGE_NEW_VIEW_H

#include <glib.h>
#include <Evas.h>

#include "contact-list-common.h"

struct MessageNewViewData {
	struct View view;
	int mode;
	char *content;
	Evas_Object *pager, *left_button, *right_button;
	Evas_Object *layout_content, *layout_recipients;
	Evas_Object *layout_contacts, *layout_number;
	Evas_Object *content_entry, *number_entry, *hv;
	Evas_Object *list_recipients;

	GPtrArray *recipients;
	struct ContactListData contact_list_data;

	int messages_sent;
};

struct MessageNewViewData *message_new_view_init(GHashTable *options);
void message_new_view_deinit(struct MessageNewViewData *view);
void message_new_view_show(struct MessageNewViewData *view);

#endif