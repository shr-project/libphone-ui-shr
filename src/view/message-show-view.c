/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *		Martin Jansa <Martin.Jansa@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 */


#include <glib.h>
#include <glib-object.h>
#include <Elementary.h>
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils-calls.h>
#include <phoneui/phoneui-utils-contacts.h>
#include <phoneui/phoneui-utils-messages.h>
#include "common-utils.h"
#include "ui-utils.h"
#include "views.h"
#include "message-show-view.h"
#include "phoneui-shr.h"


static GHashTable *messageviews = NULL;

static void _close_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _answer_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _delete_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _call_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _forward_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _new_contact_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _hover_bt_1(void *_data, Evas_Object * obj, void *event_info);
static void _common_name_callback(GError *error, GHashTable *contact, void *_data);
static void _delete_cb(struct View *view, Evas_Object * win, void *event_info);
static void _destroy_cb(struct View *_view);


int
message_show_view_init(char* path, GHashTable *properties)
{
	struct MessageShowViewData *view;
	Evas_Object *win, *ico, *box, *obj;
	int ret;
	GVariant *tmp;
	const char *direction = NULL;
	Eina_Bool in_msg = EINA_FALSE;

	/* path MUST always be set! It will be freed by
	destroying the messageviews hashtable in here, thus must be a copy */
	if (!path) {
		g_warning("Trying to initialize a message view without path!");
		return 1;
	}

	g_debug("Initializing the contact view for '%s'", path);

	view = malloc(sizeof(struct MessageShowViewData));
	if (!view) {
		g_critical("Failed to allocate message view for '%s'", path);
		if (properties) {
			g_hash_table_unref(properties);
		}
		free(path);
		return 1;
	}

	ret = ui_utils_view_init(VIEW_PTR(*view), ELM_WIN_BASIC, D_("Message"),
				 NULL, NULL, _destroy_cb);
	if (ret) {
		g_critical("Failed to init message view for '%s'", path);
		if (properties) {
			g_hash_table_unref(properties);
		}
		free(view);
		free(path);
		return ret;
	}

	/* cache the views to open only one view per message */
	if (messageviews == NULL) {
		messageviews = g_hash_table_new_full(g_str_hash, g_str_equal,
						     free, NULL);
	}
	g_hash_table_insert(messageviews, path, view);

	view->path = path;
	view->number = NULL;
	view->name = NULL;
	view->photopath = NULL;

	elm_theme_extension_add(NULL, phoneui_theme);

	win = ui_utils_view_window_get(VIEW_PTR(*view));
	ui_utils_view_delete_callback_set(VIEW_PTR(*view), _delete_cb);

	ui_utils_view_layout_set(VIEW_PTR(*view), phoneui_theme,
				 "phoneui/messages/show");

	GList *keys = g_hash_table_get_keys(properties);
	for (; keys; keys = keys->next) {
		tmp = g_hash_table_lookup(properties, keys->data);
		if (tmp) {
			g_debug("--- %s: %s", (char *)keys->data, g_variant_print(tmp, TRUE));
		}
	}
	tmp = g_hash_table_lookup(properties, "Peer");
	if (!tmp) {
		tmp = g_hash_table_lookup(properties, "Sender");
	}
	if (!tmp) {
		tmp = g_hash_table_lookup(properties, "Recipient");
	}
	if (tmp) {
		view->number = g_variant_dup_string(tmp, NULL);
		g_debug("Found number %s - starting lookup", view->number);
		// FIXME: use new @Contacts feature from opimd whenever it is
		//        clear how to do that :P
		phoneui_utils_contact_lookup(view->number,
					     _common_name_callback,
					     common_utils_object_ref(view));
		ui_utils_view_text_set(VIEW_PTR(*view), "text_number",
				       view->number);
	}

	tmp = g_hash_table_lookup(properties, "Timestamp");
	if (tmp) {
		char *date = common_utils_timestamp_to_date(
					(long)g_variant_get_int32(tmp));
		if (date) {
			g_debug("Found date %s", date);
			ui_utils_view_text_set(VIEW_PTR(*view), "text_date", date);
			free(date);
		}
	}

	view->photo = elm_icon_add(win);
	evas_object_size_hint_aspect_set(view->photo,
					 EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_file_set(view->photo, phoneui_theme, "icon/contact");
	ui_utils_view_swallow(VIEW_PTR(*view), "photo", view->photo);
	evas_object_show(view->photo);

	ico = elm_icon_add(win);
	tmp = g_hash_table_lookup(properties, "Direction");
	if (tmp) {
		direction = g_variant_get_string(tmp, NULL);
		if (strcmp(direction, "in") == 0) {
			g_debug("Setting status icon for an incoming message");
			in_msg = EINA_TRUE;
			elm_icon_file_set(ico, phoneui_theme,
					  "icon/phonelog-incoming");
		}
		else {
			g_debug("Setting status icon for a sent message");
			in_msg = EINA_FALSE;
			elm_icon_file_set(ico, phoneui_theme,
					  "icon/phonelog-outgoing");
		}
	}
	ui_utils_view_swallow(VIEW_PTR(*view), "icon_status", ico);
	evas_object_show(ico);

	const char *content = NULL;
	tmp = g_hash_table_lookup(properties, "Content");
	if (tmp) {
		content = elm_entry_utf8_to_markup(g_variant_get_string(tmp, NULL));
	}

	view->sc_content = elm_scroller_add(win);
	elm_scroller_bounce_set(view->sc_content, EINA_FALSE, EINA_FALSE);

	view->content = elm_anchorblock_add(win);
	elm_anchorblock_hover_style_set(view->content, "popout");
	elm_anchorblock_hover_parent_set(view->content, win);
	evas_object_size_hint_weight_set(view->content, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	if (content) {
		elm_object_text_set(view->content, content);
	}
	elm_scroller_content_set(view->sc_content, view->content);
	evas_object_show(view->content);
	ui_utils_view_swallow(VIEW_PTR(*view), "text_content", view->sc_content);
	evas_object_show(view->sc_content);


	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Close"));
	evas_object_smart_callback_add(obj, "clicked",
				       _close_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_close", obj);
	evas_object_show(obj);

	// Options button with hover
	view->hv = elm_hover_add(win);

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Options"));
	evas_object_smart_callback_add(obj, "clicked", _hover_bt_1,
				       view->hv);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_options", obj);
	evas_object_show(obj);

	elm_hover_parent_set(view->hv, win);
	elm_hover_target_set(view->hv, obj);

	box = elm_box_add(win);
	elm_box_horizontal_set(box, 0);
	elm_box_homogeneous_set(box, 1);
	evas_object_show(box);

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Delete"));
	evas_object_size_hint_min_set(obj, 140, 80);
	evas_object_smart_callback_add(obj, "clicked",
				       _delete_clicked, view);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);

	if (in_msg) {
		obj = elm_button_add(win);
		elm_object_text_set(obj, D_("Call"));
		evas_object_size_hint_min_set(obj, 140, 80);
		evas_object_smart_callback_add(obj, "clicked",
						   _call_clicked, view);
		evas_object_show(obj);
		elm_box_pack_end(box, obj);
	}

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Add Contact"));
	evas_object_size_hint_min_set(obj, 140, 80);
	evas_object_smart_callback_add(obj, "clicked",
					   _new_contact_clicked,
					   view);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);
	view->add_contact_bt = obj;

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Forward"));
	evas_object_size_hint_min_set(obj, 140, 80);
	evas_object_smart_callback_add(obj, "clicked", _forward_clicked, view);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);

	elm_hover_content_set(view->hv, "top", box);


	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Answer"));
	evas_object_smart_callback_add(obj, "clicked",
				       _answer_clicked, view);
	ui_utils_view_swallow(VIEW_PTR(*view), "button_answer", obj);
	evas_object_show(obj);

	if (in_msg) {
		g_debug("going to set read status for the message");
		phoneui_utils_message_set_read_status(view->path, 1, NULL, NULL);
		g_debug("done - destroying properties now");
	}

	g_hash_table_destroy(properties);

	g_debug("done");

	return 0;
}

int
message_show_view_is_init(const char* path)
{
	struct MessageShowViewData *view;

	if (messageviews == NULL) {
		return 0;
	}
	view = (struct MessageShowViewData *)
			g_hash_table_lookup(messageviews, path);
	if (view && ui_utils_view_is_init(VIEW_PTR(*view))) {
		return 1;
	}
	return 0;
}

void
message_show_view_deinit(struct MessageShowViewData *view)
{
	if (view) {
		g_debug("Deiniting view for message");
		ui_utils_view_deinit(VIEW_PTR(*view));
	}
	else {
		g_warning("Deiniting a message view without view?");
	}
}

void
message_show_view_show(const char* path)
{
	struct MessageShowViewData *view;

	g_debug("looking up message view for '%s'", path);

	if (messageviews == NULL) {
		g_debug("No message views loaded yet");
		return;
	}
	view = (struct MessageShowViewData *)
			g_hash_table_lookup(messageviews, path);
	if (view) {
		ui_utils_view_show(VIEW_PTR(*view));
	}
	else {
		g_warning("Could not find view for message '%s'", path);
	}
	g_debug("message view show done");

}

void
message_show_view_hide(const char* path)
{
	(void) path;
}

/* --- evas callbacks ------------------------------------------------------- */

static void
_close_clicked(void *_data, Evas_Object * obj,
				void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;
	message_show_view_deinit(view);
	free(view);
}

static void
_answer_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	(void) obj;
	(void) event_info;
	GHashTable *options;
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_answer_clicked()");

	options = g_hash_table_new_full(g_str_hash, g_str_equal,
					NULL, common_utils_variant_unref);
        if (!view->number) {
		g_warning("Trying to answer a message without number?!");
		return;
	}
	g_hash_table_insert(options, "Phone",
			      g_variant_ref_sink(g_variant_new_string(view->number)));
	if (view->name) {
		g_hash_table_insert(options, "Name",
			    g_variant_ref_sink(g_variant_new_string(view->name)));
	}
	if (view->photopath) {
		g_hash_table_insert(options, "Photo",
			g_variant_ref_sink(g_variant_new_string(view->photopath)));
	}

	phoneui_messages_message_new(options);
	//g_hash_table_destroy(options);
}

static void
_call_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;
	char *number = view->number;
	g_debug("_call_clicked()");

	evas_object_hide(view->hv);
	phoneui_utils_dial(number, NULL, NULL);
}

static void
_forward_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;

	char *content;
	GHashTable *options;
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;

	evas_object_hide(view->hv);

	options = g_hash_table_new_full(g_str_hash, g_str_equal,
					NULL, common_utils_variant_unref);
	content = elm_entry_markup_to_utf8(elm_object_text_get(view->content));
	if (content) {
		g_hash_table_insert(options, "Content",
		                    g_variant_ref_sink(g_variant_new_string(content)));
		free(content);
	}

	phoneui_messages_message_new(options);
}

static void
_delete_result_cb(GError *error, gpointer data)
{
	struct MessageShowViewData *view = (struct MessageShowViewData *)data;
	if (error) {
		ui_utils_dialog(VIEW_PTR(view),
				D_("Deleting the message failed!"), DIALOG_OK,
				NULL, NULL);
		g_warning("Deleting the message failed: %s", error->message);
		return;
	}
	message_show_view_deinit(view);
}

static void
_delete_confirm_cb(int result, void *data)
{
	struct MessageShowViewData *view = (struct MessageShowViewData *)data;
	if (result == DIALOG_YES && view->path) {
		phoneui_utils_message_delete(view->path, _delete_result_cb,
					     view);
	}
}

static void
_delete_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;

	g_debug("_delete_clicked()");

	evas_object_hide(view->hv);
	ui_utils_dialog(VIEW_PTR(*view),
				D_("Really delete this message?"),
				DIALOG_YES|DIALOG_NO,
				_delete_confirm_cb, view);

}

static void
_hover_bt_1(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	Evas_Object *hv = (Evas_Object *) _data;
	evas_object_show(hv);
}



/* callbacks */


static void
_common_name_callback(GError *error, GHashTable *contact, void *_data)
{
	struct MessageShowViewData *view = (struct MessageShowViewData *) _data;
	char *tmp;
	GVariant *gtmp;

	// FIXME: show some nice notification
	if (error || !contact)
		return;

	if (!ui_utils_view_is_init(VIEW_PTR(*view))) {
		return;
	}

	tmp = phoneui_utils_contact_display_name_get(contact);
	if (tmp) {
		ui_utils_view_text_set(VIEW_PTR(*view), "text_number", tmp);
		free(tmp);

		if (view->add_contact_bt) {
			evas_object_del(view->add_contact_bt);
			view->add_contact_bt = NULL;
		}
	}
	gtmp = g_hash_table_lookup(contact, "Photo");
	if (gtmp) {
		elm_icon_file_set(view->photo,
				  g_variant_get_string(gtmp, NULL), NULL);
	}
}


static void
_new_contact_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct MessageShowViewData *view;
	view = (struct MessageShowViewData *)_data;
	evas_object_hide(view->hv);
	GHashTable *options = g_hash_table_new_full(g_str_hash, g_str_equal,
					       NULL, common_utils_variant_unref);
	g_hash_table_insert(options, "Phone",
			g_variant_ref_sink(g_variant_new_string(view->number)));

	phoneui_contacts_contact_new(options);
}



static void
_delete_cb(struct View *view, Evas_Object * win, void *event_info)
{
	(void)win;
	(void)event_info;
	g_debug("_delete_cb");
	message_show_view_deinit((struct MessageShowViewData *)view);
	free(view);
	g_debug("_delete_cb DONE");
}

static void
_destroy_cb(struct View *_view)
{
	struct MessageShowViewData *view = (struct MessageShowViewData *)_view;
	g_debug("_destroy_cb");
	g_hash_table_remove(messageviews, view->path);
	g_debug("_destroy_cb DONE");
}

