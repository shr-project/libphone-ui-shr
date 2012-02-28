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
#include <time.h>
#include <Elementary.h>
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils-calls.h>
#include <phoneui/phoneui-utils-contacts.h>
#include <phoneui/phoneui-utils-messages.h>
#include <phoneui/phoneui-info.h>

#include "phoneui-shr.h"
#include "views.h"
#include "common-utils.h"
#include "ui-utils.h"
#include "message-list-view.h"

#define MSG_PER_UPDATE 6
#define MSG_PAGE_SIZE MSG_PER_UPDATE*5

struct MessageListViewData  {
	struct View view;
	char *path;
	unsigned int msg_start;
	unsigned int msg_end;
	Evas_Object *list, *hv, *bx, *call_bt, *answer_bt, *top_pb, *bottom_pb,
	            *forward_bt, *right_bt;
	Elm_Object_Item *latest_it;
	Eina_Bool scroll_lock;
};
static struct MessageListViewData view;
static Elm_Genlist_Item_Class itc;

typedef enum _InsertMode {
	LIST_INSERT_APPEND,
	LIST_INSERT_PREPEND,
	LIST_INSERT_SORTED
} InsertMode;

static void _process_messages(GError *error, GHashTable **messages, int count, gpointer data);
static void _process_message_get(GError* error, GHashTable* message, gpointer data);
static void _process_message(gpointer _message, gpointer _data);
static void _remove_message(const char *path);
static void _add_message(const char *path);
static void _message_changed_cb(void *data, const char *path, enum PhoneuiInfoChangeType type);
static void _hide_cb(struct View *view);
static void _delete_cb(struct View *data, Evas_Object *obj, void *event_info);

static void _new_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _show_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _answer_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _call_clicked(void *_data, Evas_Object *obj, void *event_info);
static void _forward_clicked(void *_data, Evas_Object *obj, void *event_info);
static void _edit_clicked(void *_data, Evas_Object *obj, void *event_info);
static void _delete_clicked(void *_data, Evas_Object * obj, void *event_info);
static void _hover_bt_1(void *_data, Evas_Object * obj, void *event_info);
static Eina_Bool _release_scroll_lock(void *_data);
static void _scroll_bottom(void *_data, Evas_Object * obj, void *event_info);
static void _scroll_top(void *_data, Evas_Object * obj, void *event_info);
static void _selected_changed(void *_data, Evas_Object * obj, void *event_info);
static char *gl_text_get(void *data, Evas_Object * obj, const char *part);
static Evas_Object * gl_content_get(void *data, Evas_Object * obj, const char *part);
static Eina_Bool gl_state_get(void *data, Evas_Object *obj, const char *part);
static void gl_del(void *data, Evas_Object * obj);

int
message_list_view_init()
{
	Evas_Object *win, *box, *obj;
	int ret;

	g_debug("Initing message list view");
	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("Messages"),
				 NULL, _hide_cb, NULL);
	if (ret) {
		g_critical("Failed to init the message list view");
		return ret;
	}

	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);
	ui_utils_view_layout_set(VIEW_PTR(view), phoneui_theme,
				 "phoneui/messages/list");
	elm_theme_extension_add(NULL, phoneui_theme);

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("New"));
	evas_object_smart_callback_add(obj, "clicked", _new_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_new", obj);
	evas_object_show(obj);

	// Options button with hover
	view.hv = elm_hover_add(win);

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Options"));
	evas_object_smart_callback_add(obj, "clicked", _hover_bt_1, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_options", obj);
	evas_object_show(obj);

	elm_hover_parent_set(view.hv, win);
	elm_hover_target_set(view.hv, obj);

	box = elm_box_add(win);
	elm_box_horizontal_set(box, 0);
	elm_box_homogeneous_set(box, 1);
	evas_object_show(box);

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Answer"));
	evas_object_size_hint_min_set(obj, 130, 80);
	evas_object_smart_callback_add(obj, "clicked", _answer_clicked, NULL);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);
	view.answer_bt = obj;

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Call"));
	evas_object_size_hint_min_set(obj, 130, 80);
	evas_object_smart_callback_add(obj, "clicked", _call_clicked, NULL);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);
	view.call_bt = obj;

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Forward"));
	evas_object_size_hint_min_set(obj, 130, 80);
	evas_object_smart_callback_add(obj, "clicked", _forward_clicked, NULL);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);
	view.forward_bt = obj;

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Delete"));
	evas_object_size_hint_min_set(obj, 130, 80);
	evas_object_smart_callback_add(obj, "clicked", _delete_clicked, NULL);
	evas_object_show(obj);
	elm_box_pack_end(box, obj);

	elm_object_part_content_set(view.hv, "top", box);

	obj = elm_button_add(win);
	elm_object_text_set(obj, D_("Show"));
	evas_object_smart_callback_add(obj, "clicked", _show_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_show", obj);
	evas_object_show(obj);
	view.right_bt = obj;

	view.list = elm_genlist_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "list", view.list);
	itc.item_style = "message";
	itc.func.text_get = gl_text_get;
	itc.func.content_get = gl_content_get;
	itc.func.state_get = gl_state_get;
	itc.func.del = gl_del;
	evas_object_show(view.list);

	view.scroll_lock = FALSE;
	evas_object_smart_callback_add(view.list, "scroll,edge,bottom", _scroll_bottom, NULL);
	evas_object_smart_callback_add(view.list, "scroll,edge,top", _scroll_top, NULL);
	evas_object_smart_callback_add(view.list, "selected", _selected_changed, NULL);

	view.top_pb = elm_progressbar_add(win);
	elm_object_style_set(view.top_pb, "wheel");
	elm_object_text_set(view.top_pb, D_("Loading..."));
	elm_progressbar_pulse(view.top_pb, EINA_TRUE);
	evas_object_size_hint_align_set(view.top_pb, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(view.top_pb, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	ui_utils_view_swallow(VIEW_PTR(view), "loading_top_indicator",
			      view.top_pb);
	evas_object_show(view.top_pb);

	view.bottom_pb = elm_progressbar_add(win);
	elm_object_style_set(view.bottom_pb, "wheel");
	elm_object_text_set(view.bottom_pb, D_("Loading..."));
	elm_progressbar_pulse(view.bottom_pb, EINA_TRUE);
	evas_object_size_hint_align_set(view.bottom_pb, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(view.bottom_pb, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	ui_utils_view_swallow(VIEW_PTR(view), "loading_bottom_indicator",
			      view.bottom_pb);
	evas_object_show(view.bottom_pb);

	view.msg_start = 0;
	view.msg_end = 0;
	view.latest_it = NULL;

	edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
				"start_bottom_loading","");
	phoneui_utils_messages_get_full("Timestamp", TRUE, 0, MSG_PAGE_SIZE, TRUE, NULL, _process_messages, GINT_TO_POINTER(LIST_INSERT_APPEND));
	phoneui_info_register_message_changes(_message_changed_cb, NULL);

	return 0;
}

int
message_list_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

void
message_list_view_deinit()
{
	ui_utils_view_deinit(VIEW_PTR(view));
}

void
message_list_view_show()
{
	evas_object_hide(view.hv);
	ui_utils_view_show(VIEW_PTR(view));
}

void
message_list_view_hide()
{
	ui_utils_view_hide(VIEW_PTR(view));
}


/* --- evas callbacks ------------------------------------------------------- */

static void
_new_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;
	phoneui_messages_message_new(NULL);
}

static void
_show_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;
	Elm_Object_Item *glit;
	GHashTable *message;
	GVariant *tmp;

	glit = elm_genlist_selected_item_get(view.list);
	if (glit) {
		g_debug("found the selected item");
		message = (GHashTable *)elm_object_item_data_get(glit);
		if (!message) {
			g_warning("message has NO PROPERTIES!!!!");
			return;
		}
		tmp = g_hash_table_lookup(message, "Path");
		if (tmp) {
			phoneui_messages_message_show
					(g_variant_get_string(tmp, NULL));
		}
		else {
			g_warning("No path for message found!!!");
		}
	}
}

static void
_answer_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;
	Elm_Object_Item *glit;
	GVariant *tmp;
	GHashTable *options, *message;

	evas_object_hide(view.hv);

	glit = elm_genlist_selected_item_get(view.list);
	if (glit) {
		message = (GHashTable *)elm_object_item_data_get(glit);

		options = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, common_utils_variant_unref);
		tmp = g_hash_table_lookup(message, "Name");
		if (tmp) {
			g_hash_table_insert(options, "Name", g_variant_ref(tmp));
		}

		tmp = g_hash_table_lookup(message, "Phone");
		if (tmp) {
			g_hash_table_insert(options, "Phone", g_variant_ref(tmp));
		}
		phoneui_messages_message_new(options);
	}
}

static void
_call_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	(void) _data;

	const char *number;
	GVariant *tmp;
	GHashTable *message;
	Elm_Object_Item *glit;

	evas_object_hide(view.hv);

	glit = elm_genlist_selected_item_get(view.list);
	if (glit) {
		message = (GHashTable *)elm_object_item_data_get(glit);

		tmp = g_hash_table_lookup(message, "Phone");
		if (tmp) {
			number = g_variant_get_string(tmp, NULL);
			phoneui_utils_dial(number, NULL, NULL);
		}
	}
}

static void
_forward_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;
	Elm_Object_Item *glit;
	GVariant *tmp;
	GHashTable *options, *message;

	evas_object_hide(view.hv);

	glit = elm_genlist_selected_item_get(view.list);
	if (glit) {
		message = (GHashTable *)elm_object_item_data_get(glit);

		options = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, common_utils_variant_unref);
		tmp = g_hash_table_lookup(message, "Content");
		if (tmp) {
			g_hash_table_insert(options, "Content", g_variant_ref(tmp));
		}

		phoneui_messages_message_new(options);
	}
}

static void
_edit_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	_forward_clicked(_data, obj, event_info);
}

static void
_delete_result_cb(GError *error, gpointer data)
{
	(void)data;
	if (error) {
		ui_utils_dialog(VIEW_PTR(view),
				D_("Deleting the message failed!"), DIALOG_OK,
				NULL, NULL);
		g_warning("Deleting the message failed: %s", error->message);
		return;
	}
}

static void
_delete_confirm_cb(int result, void *data)
{
	Elm_Object_Item *glit;
	GHashTable *message;
	GVariant *tmp;

	glit = (Elm_Object_Item *)data;
	if (result == DIALOG_YES) {
		message = (GHashTable *)elm_object_item_data_get(glit);
		tmp = g_hash_table_lookup(message, "Path");
		if (tmp) {
			phoneui_utils_message_delete
				(g_variant_get_string(tmp, NULL), _delete_result_cb, NULL);
		}
	}
}

static void
_delete_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;
	Elm_Object_Item *glit;

	g_debug("_delete_clicked()");

	evas_object_hide(view.hv);

	glit = elm_genlist_selected_item_get(view.list);
	if (glit != NULL) {
		g_debug("found a selected row to delete...");
		ui_utils_dialog(VIEW_PTR(view),
				D_("Really delete this message?"),
				DIALOG_YES|DIALOG_NO,
				_delete_confirm_cb, glit);
	}
}

static void
_hover_bt_1(void *_data, Evas_Object * obj, void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;
	evas_object_show(view.hv);
}

static Eina_Bool _release_scroll_lock(void *_data)
{
	(void) _data;

	view.scroll_lock = FALSE;

	return ECORE_CALLBACK_CANCEL;
}

static void _scroll_bottom(void *_data, Evas_Object * obj, void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;

	if (view.scroll_lock) return;
	view.scroll_lock = TRUE;

	elm_progressbar_pulse(view.bottom_pb, EINA_TRUE);
	edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
				"start_bottom_loading","");

	view.latest_it = elm_genlist_last_item_get(view.list);
	phoneui_utils_messages_get_full("Timestamp", TRUE, view.msg_end, MSG_PER_UPDATE, TRUE, NULL, _process_messages, GINT_TO_POINTER(LIST_INSERT_APPEND));
}

static void _scroll_top(void *_data, Evas_Object * obj, void *event_info)
{
	(void) _data;
	(void) obj;
	(void) event_info;
	if (view.msg_start == 0)
		return;

	if (view.scroll_lock) return;
	view.scroll_lock = TRUE;

	elm_progressbar_pulse(view.top_pb, EINA_TRUE);
	edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
				"start_top_loading","");

	unsigned int start = view.msg_start > MSG_PER_UPDATE ? view.msg_start-MSG_PER_UPDATE : 0;

	view.latest_it = elm_genlist_first_item_get(view.list);
	phoneui_utils_messages_get_full("Timestamp", TRUE, start, MSG_PER_UPDATE, TRUE, NULL, _process_messages, GINT_TO_POINTER(LIST_INSERT_SORTED));
}

static void
_selected_changed(void *_data, Evas_Object * obj, void *event_info) {
	(void)_data;
	(void)obj;
	GVariant *tmp;
	Eina_Bool new = EINA_FALSE;
	Eina_Bool out = EINA_FALSE;

	Elm_Object_Item *glit = (Elm_Object_Item *)event_info;
	if (!glit) return;

	GHashTable *message = (GHashTable *)elm_object_item_data_get(glit);
	if (!message) return;

	if ((tmp = g_hash_table_lookup(message, "Direction"))) {
		if (!strcmp(g_variant_get_string(tmp, NULL), "out")) {
			out = EINA_TRUE;
			evas_object_hide(view.answer_bt);
			evas_object_hide(view.call_bt);
		} else {
			evas_object_show(view.answer_bt);
			evas_object_show(view.call_bt);
		}
	}

	if ((tmp = g_hash_table_lookup(message, "New"))) {
		// FIXME: shouldn't this be boolean ?
		if (g_variant_get_int32(tmp) == 1) {
			new = EINA_TRUE;
		}
	}

	if (new && out) {
		elm_object_text_set(view.right_bt, D_("Edit"));
		evas_object_smart_callback_del(view.right_bt, "clicked", _edit_clicked);
		evas_object_smart_callback_del(view.right_bt, "clicked", _show_clicked);
		evas_object_smart_callback_add(view.right_bt, "clicked", _edit_clicked, NULL);

		elm_object_text_set(view.forward_bt, D_("Edit"));
		evas_object_size_hint_min_set(view.forward_bt, 130, 80);
		evas_object_smart_callback_del(view.forward_bt, "clicked", _edit_clicked);
		evas_object_smart_callback_del(view.forward_bt, "clicked", _forward_clicked);
		evas_object_smart_callback_add(view.forward_bt, "clicked", _edit_clicked, NULL);
	} else {
		elm_object_text_set(view.right_bt, D_("Show"));
		evas_object_smart_callback_del(view.right_bt, "clicked", _edit_clicked);
		evas_object_smart_callback_del(view.right_bt, "clicked", _show_clicked);
		evas_object_smart_callback_add(view.right_bt, "clicked", _show_clicked, NULL);

		elm_object_text_set(view.forward_bt, D_("Forward"));
		evas_object_size_hint_min_set(view.forward_bt, 130, 80);
		evas_object_smart_callback_del(view.forward_bt, "clicked", _edit_clicked);
		evas_object_smart_callback_del(view.forward_bt, "clicked", _forward_clicked);
		evas_object_smart_callback_add(view.forward_bt, "clicked", _forward_clicked, NULL);
	}
}

static void
_contact_lookup(GError *error, GHashTable *contact, gpointer data)
{
	Elm_Object_Item *glit;
	GHashTable *message;
	char *tmp;

	if (error) {
		ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
				D_("Error while trying to resolve the number"),
				10);
		g_warning("Error resolving number: (%d) %s",
			  error->code, error->message);
		return;
	}

	if (!contact) {
		return;
	}

	glit = (Elm_Object_Item *)data;
	tmp = phoneui_utils_contact_display_name_get(contact);
	if (tmp) {
		message = (GHashTable *)elm_object_item_data_get(glit);
		g_hash_table_insert(message, "Name",
				      g_variant_ref_sink(g_variant_new_string(tmp)));
		Evas_Object *obj = (Evas_Object *)elm_object_item_widget_get(glit);
		edje_object_part_text_set(obj, "elm.name", tmp);
		free(tmp);
	}
}

static void
_process_messages(GError* error, GHashTable** messages, int count, gpointer data)
{
	int i;

	g_debug("got %d messages", count);

	if (error) {
		ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
				D_("Error while retrieving messages"), 10);
		g_warning("Error retrieving messages: (%d) %s",
			  error->code, error->message);
		goto close;
		return;
	}
	if (!messages) {
		ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
				D_("There are no messages"), 5);
		g_debug("No messages to load");
		return;
	}

	for (i = 0; i < count; i++) {
		g_debug("processing message %d", i);
		_process_message(messages[i], data);
	}

	if (view.latest_it) {
		elm_genlist_item_middle_show(view.latest_it);
		view.latest_it = NULL;
	}

close:
	ecore_timer_add(0.75, _release_scroll_lock, NULL);
	edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
				"stop_loading","");
	elm_progressbar_pulse(view.top_pb, EINA_FALSE);
	elm_progressbar_pulse(view.bottom_pb, EINA_FALSE);
}

static void
_process_message_get(GError *error, GHashTable *message, gpointer data)
{
	Elm_Object_Item *glit;
	GHashTable *it_data;
	GVariant *tmp;

	if (error) {
		ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
				D_("Error while retrieving a message"), 10);
		g_warning("Error retrieving a message: (%d) %s",
			  error->code, error->message);
		return;
	}
	if (!message) {
		return;
	}

	long new_timestamp = 0;
	long first_timestamp = 0;
	long last_timestamp = 0;

	if ((tmp = g_hash_table_lookup(message, "Timestamp"))) {
		new_timestamp = (long) g_variant_get_int32(tmp);
	}

	glit = elm_genlist_first_item_get(view.list);
	if (glit) {
		it_data = elm_object_item_data_get(glit);
		if (it_data && (tmp = g_hash_table_lookup(it_data, "Timestamp"))) {
			first_timestamp = (long) g_variant_get_int32(tmp);
		}
	}

	glit = elm_genlist_last_item_get(view.list);
	if (glit) {
		it_data = elm_object_item_data_get(glit);
		if (it_data && (tmp = g_hash_table_lookup(it_data, "Timestamp"))) {
			last_timestamp = (long) g_variant_get_int32(tmp);
		}
	}

	if ((view.msg_start == 0 && new_timestamp >= first_timestamp) ||
	    (new_timestamp > last_timestamp && new_timestamp <= first_timestamp)) {
		_process_message(message, data);
	}
}

static void
_process_message(gpointer _message, gpointer _data)
{
	GHashTable *message, *rowdata, *other;
	GVariant *tmp;
	long timestamp = 0, other_timestamp = 0;
	char datestr[35];
	Elm_Object_Item *glit = NULL;
	InsertMode insert_mode;

	if (!_message) {
		return;
	}
	message = (GHashTable *)_message;

	tmp = g_hash_table_lookup(message, "Path");
	if (!tmp) {
		g_critical("Message without Path?!?");
		return;
	}

	insert_mode = GPOINTER_TO_INT(_data);

	rowdata = g_hash_table_new_full(g_str_hash, g_str_equal,
					    NULL, common_utils_variant_unref);
	g_hash_table_insert(rowdata, "Path", g_variant_ref(tmp));

	tmp = g_hash_table_lookup(message, "Timestamp");
	if (tmp) {
		timestamp = (long) g_variant_get_int32(tmp);
	}
	strftime(datestr, 31, "%d.%m.%Y %H:%M" LTR_STRING, localtime(&timestamp));
	g_hash_table_insert(rowdata, "Date",
			    g_variant_ref_sink(g_variant_new_string(datestr)));
	g_hash_table_insert(rowdata, "Timestamp",
			    g_variant_ref_sink(g_variant_new_int32(timestamp)));

	tmp = g_hash_table_lookup(message, "Direction");
	if (tmp) {
		g_hash_table_insert(rowdata, "Direction", g_variant_ref(tmp));
	}

	tmp = g_hash_table_lookup(message, "Peer");
	if (!tmp) {
		tmp = g_hash_table_lookup(message, "Sender");
	}
	if (!tmp) {
		tmp = g_hash_table_lookup(message, "Recipient");
	}
	if (tmp) {
		g_hash_table_insert(rowdata, "Phone", g_variant_ref(tmp));
	}

	tmp = g_hash_table_lookup(message, "Content");
	if (tmp) {
		g_hash_table_insert(rowdata, "Content", g_variant_ref(tmp));
	}

	tmp = g_hash_table_lookup(message, "New");
	if (tmp) {
		g_hash_table_insert(rowdata, "New", g_variant_ref(tmp));
	}

	if (insert_mode == LIST_INSERT_SORTED) {
		glit = elm_genlist_first_item_get(view.list);
		while (glit) {
			other = (GHashTable *)elm_object_item_data_get(glit);
			tmp = g_hash_table_lookup(other, "Timestamp");
			if (tmp) {
				other_timestamp =
					(long)g_variant_get_int32(tmp);
			}
			if (timestamp > other_timestamp)
				break;
			glit = elm_genlist_item_next_get(glit);
		}
		if (glit) {
			if (glit == elm_genlist_first_item_get(view.list))
				insert_mode = LIST_INSERT_PREPEND;
			else if (glit == elm_genlist_last_item_get(view.list))
				insert_mode = LIST_INSERT_APPEND;

			glit = elm_genlist_item_insert_before(view.list, &itc,
						rowdata, NULL, glit,
						ELM_GENLIST_ITEM_NONE,
						NULL, NULL);
		}
	}

	if (!glit) {
		if (insert_mode == LIST_INSERT_PREPEND) {
			glit = elm_genlist_item_prepend(view.list, &itc, rowdata, NULL,
					     ELM_GENLIST_ITEM_NONE, NULL, NULL);
		} else {
			glit = elm_genlist_item_append(view.list, &itc, rowdata, NULL,
					     ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	}

	/* Save also the genlist item pointer in the messsage data, to get them dobule-linked */
	g_hash_table_insert(rowdata, "_GenlistItem",
			      common_utils_new_variant_from_pointer(glit));

	tmp = g_hash_table_lookup(message, "@Contacts");
	if (tmp) {
	        if (g_variant_type_is_array(g_variant_get_type(tmp))) {
		        /*use last the last contact*/
			char *path = phoneui_utils_contact_get_dbus_path
			            (g_variant_get_int32
			            (g_variant_get_child_value
			            (tmp,g_variant_n_children(tmp)-1)));
			phoneui_utils_contact_get(path, _contact_lookup, glit);
	                free(path);	    
		} else if (g_variant_is_of_type(tmp,G_VARIANT_TYPE_INT32)) {
		        char *path = phoneui_utils_contact_get_dbus_path
				    (g_variant_get_int32(tmp));
		        phoneui_utils_contact_get(path, _contact_lookup, glit);
	                free(path);
		}
		
	}

	g_hash_table_destroy(message);

	view.msg_end++;

	if ((view.msg_end - view.msg_start) > MSG_PAGE_SIZE) {
		if (insert_mode == LIST_INSERT_APPEND) {
			glit = elm_genlist_first_item_get(view.list);
			view.msg_start++;
		} else {
			glit = elm_genlist_last_item_get(view.list);
			view.msg_start = view.msg_start > 1 ? view.msg_start-1 : 0;
			view.msg_end = view.msg_end > 2 ? view.msg_end-2 : 0;
		}

		if (glit) {
			elm_object_item_del(glit);
		}
	}
}

static char *
gl_text_get(void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	GHashTable *message = (GHashTable *)data;
	GVariant *tmp;

	if (!strcmp(part, "elm.name")) {
		tmp = g_hash_table_lookup(message, "Name");
		if (tmp) {
			return g_variant_dup_string(tmp, NULL);
		}
		else {
			tmp = g_hash_table_lookup(message, "Phone");
			if (tmp) {
				char *number = g_variant_dup_string(tmp, NULL);

				/* FIXME this is a bad workaround needed for reloading the
				 * message contact name when the user has showed the message.
				 * It seems to be a genlist bug, which cause to use different data */
				if ((tmp = g_hash_table_lookup(message, "_GenlistItem"))) {
					Elm_Object_Item *glit;
					glit = (Elm_Object_Item *)
						common_utils_pointer_from_variant(tmp);
					phoneui_utils_contact_lookup(number, _contact_lookup, glit);
				}

				return number;
			}
		}
	}
	else if (!strcmp(part, "elm.date")) {
		tmp = g_hash_table_lookup(message, "Date");
		if (tmp) {
			return g_variant_dup_string(tmp, NULL);
		}
	}
	else if (!strcmp(part, "elm.content")) {
		tmp = g_hash_table_lookup(message, "Content");
		if (tmp) {
			return common_utils_string_strip_newline
						(g_variant_dup_string(tmp, NULL));
		}
	}

	return NULL;
}

static Evas_Object *
gl_content_get(void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	GVariant *tmp;
	GHashTable *message = (GHashTable *)data;

	return NULL;
	if (!strcmp(part, "elm.photo")) {
		// TODO
		return NULL;
	}
	else if (!strcmp(part, "elm.content")) {
		tmp = g_hash_table_lookup(message, "Content");
		if (tmp) {
			Evas_Object *win =
				ui_utils_view_window_get(VIEW_PTR(view));
			Evas_Object *txt = elm_anchorblock_add(win);
			elm_anchorblock_hover_style_set(txt, "popout");
			elm_anchorblock_hover_parent_set(txt, win);
			elm_object_text_set(txt, g_variant_get_string(tmp, NULL));
			evas_object_show(txt);
			Evas_Object *bubble = elm_bubble_add(win);
			elm_object_content_set(bubble, txt);
			tmp = g_hash_table_lookup(message, "Date");
			if (tmp) {
				elm_object_text_set(bubble,
						     g_variant_get_string(tmp, NULL));
			}
			tmp = g_hash_table_lookup(message, "Phone");
			if (tmp) {
				elm_object_part_text_set(bubble, "info",
						    g_variant_get_string(tmp, NULL));
			}
			return bubble;
		}
	}
	return NULL;
}

static Eina_Bool
gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	(void) part;
	(void) obj;
	GHashTable *message;
	GVariant *tmp;
	Eina_Bool msg_out;
	Eina_Bool new;
	Eina_Bool unconfirmed;

	message = (GHashTable *)data;
	msg_out = EINA_FALSE;
	new = EINA_FALSE;
	unconfirmed = EINA_FALSE;

	if ((tmp = g_hash_table_lookup(message, "Direction"))) {
		msg_out = !strcmp(g_variant_get_string(tmp, NULL), "out");
	}

	if ((tmp = g_hash_table_lookup(message, "New"))) {
		// FIXME: shouldn't be boolean?
		new = (g_variant_get_int32(tmp) == 1);
	}
	
	if ((tmp = g_hash_table_lookup(message, "SMS-delivered"))) {
		unconfirmed = (g_variant_get_int32(tmp) == 0);
	}

	if (new && !msg_out && !unconfirmed && !strcmp(part, "new_incoming")) {
		return EINA_TRUE;
	}
	else if (new && msg_out && unconfirmed && !strcmp(part, "new_outgoing")) {
		return EINA_TRUE;
	}
	else if (new && msg_out && !strcmp(part, "new_outgoing_unconfirmed")) {
		return EINA_TRUE;
	}
	else if (!strcmp(part, "direction_out")) {
		return msg_out;
	}

	return EINA_FALSE;
}

static void
gl_del(void *data, Evas_Object * obj)
{
	(void)obj;
	g_hash_table_destroy((GHashTable *)data);
}

static void
_add_message(const char *path)
{
	// insert sorted
	phoneui_utils_message_get(path, _process_message_get, GINT_TO_POINTER(LIST_INSERT_SORTED));
}

static void
_remove_message(const char *path)
{
	Elm_Object_Item *glit;
	GHashTable *properties;
	GVariant *tmp;

	g_debug("Removing message %s from list", path);
	glit = elm_genlist_first_item_get(view.list);
	while (glit) {
		properties = (GHashTable *)elm_object_item_data_get(glit);
		tmp = g_hash_table_lookup(properties, "Path");
		if (tmp) {
			if (!strcmp(path, g_variant_get_string(tmp, NULL))) {
				g_debug("found him - removing");
				elm_object_item_del(glit);
				view.msg_end = view.msg_end > 1 ? view.msg_end-1 : 0;
				break;
			}
		}
		glit = elm_genlist_item_next_get(glit);
	}
}

static void
_message_changed_cb(void *data, const char *path, enum PhoneuiInfoChangeType type)
{
	g_debug("contact %s got changed", path);
	(void)data;
	switch (type) {
	case PHONEUI_INFO_CHANGE_UPDATE:
		_remove_message(path);
		_add_message(path);
		break;
	case PHONEUI_INFO_CHANGE_NEW:
		_add_message(path);
		break;
	case PHONEUI_INFO_CHANGE_DELETE:
		_remove_message(path);
		break;
	}
}

static void
_hide_cb(struct View *view)
{
	g_debug("_hide_cb");
	elm_genlist_item_bring_in(elm_genlist_first_item_get(
		((struct MessageListViewData *)view)->list));
}

static void
_delete_cb(struct View *data, Evas_Object *obj, void *event_info)
{
	(void)data;
	(void)obj;
	(void)event_info;
	g_debug("_delete_cb");
	message_list_view_hide();
}
