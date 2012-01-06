/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *		Martin Jansa <Martin.Jansa@gmail.com>
 *		David Kozub <zub@linux.fjfi.cvut.cz>
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


#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui-utils-contacts.h>
#include <phoneui/phoneui-info.h>

#include "views.h"
#include "ui-utils.h"
#include "common-utils.h"
#include "phoneui-shr.h"

struct PhoneLogViewData {
	struct View parent;
	Evas_Object *toolbar, *pager;
	Evas_Object *list_in, *list_out, *list_missed, *list_all;
	Elm_Object_Item *toolbar_missed;
	int count;
	GPtrArray *calls;
};

static struct PhoneLogViewData view;
static Elm_Genlist_Item_Class itc;

static void _toolbar_changed(void *data, Evas_Object *obj, void *event_info);
static Evas_Object *_add_genlist(Evas_Object *win);
static void _add_entry(GHashTable *entry);
static void _contact_lookup(GError *error, GHashTable *contact, gpointer data);
static void _get_one_callback(GError *error, GHashTable *entry, gpointer data);
static void _get_callback(GError *error, GHashTable **entry, int count, gpointer data);
static void _hide_cb(struct View *view);
static void _delete_cb(struct View *data, Evas_Object *obj, void *event_info);
static void _call_changed_handler(void * data, const char *path, enum PhoneuiInfoChangeType);
static void _contact_changed_handler(void *data, const char *path, enum PhoneuiInfoChangeType);


static char *gl_text_get(void *data, Evas_Object * obj, const char *part);
static Evas_Object *gl_content_get(void *data, Evas_Object * obj, const char *part);
static Eina_Bool gl_state_get(void *data, Evas_Object * obj, const char *part);
static void gl_del(void *data, Evas_Object * obj);

void phone_log_view_show()
{
	ui_utils_view_show(VIEW_PTR(view));
}

void phone_log_view_hide()
{
	ui_utils_view_hide(VIEW_PTR(view));
}

int phone_log_view_init()
{
	g_debug("Initializing the phonelog screen");
	Evas_Object *win;
	int ret;
	//char buf[PATH_MAX];

	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("Phonelog"),
			NULL, _hide_cb, NULL);

	if (ret) {
		g_critical("Failed to init phonelog view");
		return ret;
	}

	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);

	ui_utils_view_layout_set(VIEW_PTR(view), phoneui_theme,
				 "phoneui/phonelog/phonelog");
	elm_theme_extension_add(NULL, phoneui_theme);

	view.pager = elm_pager_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "pager", view.pager);
	evas_object_show(view.pager);

	view.list_in = _add_genlist(win);
	elm_pager_content_push(view.pager, view.list_in);
	view.list_out = _add_genlist(win);
	elm_pager_content_push(view.pager, view.list_out);
	view.list_all = _add_genlist(win);
	elm_pager_content_push(view.pager, view.list_all);
	view.list_missed = _add_genlist(win);
	elm_pager_content_push(view.pager, view.list_missed);

	itc.item_style = "phonelog";
	itc.func.text_get = gl_text_get;
	itc.func.content_get = gl_content_get;
	itc.func.state_get = gl_state_get;
	itc.func.del = gl_del;

	view.toolbar = elm_toolbar_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "toolbar", view.toolbar);
	elm_toolbar_homogeneous_set(view.toolbar, 1);
	elm_toolbar_shrink_mode_set(view.toolbar, ELM_TOOLBAR_SHRINK_NONE);
	elm_toolbar_align_set(view.toolbar, 0.0);
	elm_toolbar_icon_size_set(view.toolbar, 16);
	evas_object_size_hint_weight_set(view.toolbar, 0.0, 0.0);
	evas_object_size_hint_align_set(view.toolbar, EVAS_HINT_FILL, 0.0);

	elm_toolbar_item_append(view.toolbar, "icon/phonelog-incoming", D_("received"),
			     _toolbar_changed, view.list_in);

	elm_toolbar_item_append(view.toolbar, "icon/phonelog-outgoing", D_("outgoing"),
			     _toolbar_changed, view.list_out);

	view.toolbar_missed = elm_toolbar_item_append(view.toolbar, "icon/phonelog-missed",
						   D_("missed"),
						   _toolbar_changed,
						   view.list_missed);

	elm_toolbar_item_append(view.toolbar, "icon/phonelog-all", D_("all"), _toolbar_changed,
			     view.list_all);

	evas_object_show(view.toolbar);
	elm_toolbar_item_selected_set(view.toolbar_missed, EINA_TRUE);

	view.calls = g_ptr_array_new();

	view.count = 25; // FIXME: make the limit configurable !!! */
	phoneui_utils_calls_get(&view.count,_get_callback, NULL);

	phoneui_info_register_call_changes(_call_changed_handler, NULL);
	phoneui_info_register_contact_changes(_contact_changed_handler, NULL);

	return 0;
}

void phone_log_view_deinit()
{
	ui_utils_view_deinit(VIEW_PTR(view));
}

int phone_log_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

static void
_call_changed_handler(void *data, const char *path,
		      enum PhoneuiInfoChangeType type)
{
	(void) type;
	(void) data;
	g_debug("New call: %s", path);
	phoneui_utils_call_get(path, _get_one_callback, NULL);
}

static void
_contact_changed_handler(void *data, const char *path,
			 enum PhoneuiInfoChangeType type)
{
	(void) path;
	(void) type;
	(void) data;
}

static void
_toolbar_changed(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	elm_pager_content_promote(view.pager, data);
}

static Evas_Object *
_add_genlist(Evas_Object *win)
{
	Evas_Object *list = elm_genlist_add(win);
	elm_genlist_horizontal_set(list, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(list, 0.0, 0.0);
	evas_object_show(list);

	return list;
}

static void
_add_entry(GHashTable *entry)
{
	Elm_Genlist_Item *it;
	GVariant *val;
	int received = 0, answered = 0;

	val = g_hash_table_lookup(entry, "Direction");
	if (val) {
		const char *dir = g_variant_get_string(val, NULL);
		if (!strcmp(dir, "in")) {
			received = 1;
		}
	}
	else {
		g_warning("ignoring call without Direction field!!");
		return;
	}

	val = g_hash_table_lookup(entry, "Answered");
	if (val) {
		if (g_variant_get_boolean(val)) {
			answered = 1;
		}
	}

	it = elm_genlist_item_append(view.list_all, &itc,
			g_hash_table_ref(entry),
			NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	val = g_variant_new_int32(GPOINTER_TO_INT(it));
	g_hash_table_insert(entry, "_item_all", g_variant_ref_sink(val));
	if (received) {
		if (answered) {
			it = elm_genlist_item_append(view.list_in, &itc,
					g_hash_table_ref(entry),
					NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			val = g_variant_new_int32(GPOINTER_TO_INT(it));
			g_hash_table_insert(entry, "_item_in", g_variant_ref_sink(val));
		}
		else {
			it = elm_genlist_item_append(view.list_missed, &itc,
					g_hash_table_ref(entry),
					NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			val = g_variant_new_int32(GPOINTER_TO_INT(it));
			g_hash_table_insert(entry, "_item_missed",
					g_variant_ref_sink(val));
		}
	}
	else {
		it = elm_genlist_item_append(view.list_out, &itc,
				g_hash_table_ref(entry),
				NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		val = g_variant_new_int32(GPOINTER_TO_INT(it));
		g_hash_table_insert(entry, "_item_out",
				g_variant_ref_sink(val));
	}
}

static void
_update_entry(GHashTable *entry)
{
	Elm_Genlist_Item *it;
	GVariant *val;

	val = g_hash_table_lookup(entry, "_item_all");
	if (val) {
		it = (Elm_Genlist_Item *)GINT_TO_POINTER(g_variant_get_int32(val));
		elm_genlist_item_update(it);
	}
	val = g_hash_table_lookup(entry, "_item_missed");
	if (val) {
		it = (Elm_Genlist_Item *)GINT_TO_POINTER(g_variant_get_int32(val));
		elm_genlist_item_update(it);
	}
	val = g_hash_table_lookup(entry, "_item_in");
	if (val) {
		it = (Elm_Genlist_Item *)GINT_TO_POINTER(g_variant_get_int32(val));
		elm_genlist_item_update(it);
	}
	val = g_hash_table_lookup(entry, "_item_out");
	if (val) {
		it = (Elm_Genlist_Item *)GINT_TO_POINTER(g_variant_get_int32(val));
		elm_genlist_item_update(it);
	}
}

static void
_contact_lookup(GError *error, GHashTable *contact, gpointer data)
{
	if (error) {
		g_warning("Contact lookup error: (%d) %s", error->code, error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Contact lookup error."), error);
		return;
	}
	GHashTable *entry = data;
	if (contact) {
		char *s = phoneui_utils_contact_display_name_get(contact);
		g_hash_table_insert(entry, "Name",
				g_variant_ref_sink(g_variant_new_string(s)));
		free(s);
	}
	else {
		g_hash_table_insert(entry, "Name",
				g_variant_ref_sink(g_variant_new_string(CONTACT_NAME_UNDEFINED_STRING)));
	}
	_update_entry(entry);
}

static void
_get_one_callback(GError *error, GHashTable *entry, gpointer data)
{
	(void) error;
	(void) data;
	// FIXME: !!!!!!!!!!!!1
	g_ptr_array_add(view.calls, entry);
}

static void
_get_callback(GError* error, GHashTable** entry, int count, gpointer data)
{
	(void) data;
	GVariant *val;
        int i;

	if (error) {
		g_warning("Cannot retrieve calls list: (%d) %s", error->code, error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Cannot retrieve calls list."), error);
		return;
	}

	for (i = 0; i < count; i++) {
		g_ptr_array_add(view.calls, entry[i]);

		val = g_hash_table_lookup(entry[i], "Peer");
		if (val) {
			_add_entry(entry[i]);
			phoneui_utils_contact_lookup(g_variant_get_string(val, NULL),
						     _contact_lookup, entry[i]);
		}
		else {
			g_message("ignoring call without Peer attribute");
		}
	}
}

/* --- genlist callbacks --- */
static char *
gl_text_get(void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	GHashTable *entry = (GHashTable *) data;
	GVariant *val;

	g_debug("gl_text_get: %s", part);
	if (!strcmp(part, "elm.text")) {
		val = g_hash_table_lookup(entry, "Name");
		if (val) {
			return g_variant_dup_string(val, NULL);
		}
		return strdup("");
	}

	if (!strcmp(part, "elm.text.sub")) {
		val = g_hash_table_lookup(entry, "Peer");
		if (val) {
			return g_variant_dup_string(val, NULL);
		}
		return strdup(CONTACT_PHONE_UNDEFINED_STRING);
	}

	if (!strcmp(part, "elm.text.2")) {
		val = g_hash_table_lookup(entry, "Timestamp");
		if (val) {
			return common_utils_timestamp_to_date
						(g_variant_get_int32(val));
		}
	}

	if (!strcmp(part, "elm.text.sub.2")) {
		val = g_hash_table_lookup(entry, "Duration");
		if (val) {
			float duration = strtof(g_variant_get_string(val, NULL), NULL);
			int h = duration / 3600;
			int m = duration / 60 - h * 60;
			int s = duration - h * 3600 - m * 60;
			char durstr[10];
			if (h > 0) {
				snprintf(durstr, 10, "%02d:%02d:%02d", h, m, s);
			}
			else {
				snprintf(durstr, 10, "%02d:%02d", m, s);
			}
			return strdup(durstr);
		}
		return strdup("00:00");
	}

	return strdup("");
}

static Evas_Object *
gl_content_get(void *data, Evas_Object * obj, const char *part)
{
	(void) data;
	if (!strcmp(part,"elm.swallow.end")) {
		Evas_Object *btn = elm_button_add(obj);
		elm_object_text_set(btn, "Action");
		return (btn);
	}
	return (NULL);
}

static Eina_Bool
gl_state_get(void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	(void) part;
	GHashTable *entry = (GHashTable *) data;
	GVariant *tmp = g_hash_table_lookup(entry, "Direction");
	if (tmp && g_variant_get_int32(tmp))
		return (EINA_TRUE);
	return (EINA_FALSE);
}

static void
gl_del(void *data, Evas_Object * obj)
{
	(void) obj;
	if (data)
		g_hash_table_destroy((GHashTable *)data);
}

static void
_hide_cb(struct View *_view)
{
	struct PhoneLogViewData *view = (struct PhoneLogViewData *)_view;
	elm_pager_content_promote(view->pager, view->list_missed);
	elm_toolbar_item_selected_set(view->toolbar_missed, EINA_TRUE);
}

static void
_delete_cb(struct View *data, Evas_Object *obj, void *event_info)
{
	(void)data;
	(void)obj;
	(void)event_info;
	phone_log_view_hide();
}
