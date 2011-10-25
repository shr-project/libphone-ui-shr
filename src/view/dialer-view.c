/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *		David Kozub <zub@linux.fjfi.cvut.cz>
 *		Christ van Willegen <x@ik.nu>
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
#include <phoneui/phoneui-utils-calls.h>
#include <phoneui-utils-contacts.h>
#include <string.h>
#include <glib.h>

#include "views.h"
#include "widget/elm_keypad.h"
#include "common-utils.h"
#include "util/ui-utils.h"
#include "contact-list-common.h"


/*TODO: remove the many hacks here
 * Fix the scale hack, shouldn't exist :|
 */
#include "phoneui-shr.h"

void fill_suggest(int *contacts, GList *suggest_data);
static void _process_entry(void *_entry, void *_data);

#define MAX_SUGGEST 7

struct DialerViewData {
	struct View parent;
	GList *suggest_data;
	int contacts;
	char number[65]; /*FIXME this hackish copy */
	char suggestions[MAX_SUGGEST][65];
	int length; /*At least while this hack exists, cache length */
	Evas_Object *keypad, *bt_options, *bt_suggest, *bt_call, *bt_exit, *hv, *hv2, *bx, *bx2,
		*bt_save, *bt_message, *bt_suggest_contact[MAX_SUGGEST];
	Evas_Object *text_number, *text_number_info, *delete_text_icon,
		*delete_text_button;
};

static struct DialerViewData view;

static void _dialer_number_update();
static int _dialer_number_clear();
static void _dialer_delete_clicked_cb(void *_data, Evas_Object * o, void *event_info);
static void _dialer_keypad_clicked_cb(void *data, Evas_Object * obj, void *event_info);
static void _dialer_exit_clicked_cb(void *data, Evas_Object * obj, void *event_info);
static void _dialer_number_clicked_cb(void *_data, Evas_Object * o, const char *emission, const char *source);
static void _dialer_suggest_clicked_cb(void *data, Evas_Object * obj, void *event_info);
static void _dialer_options_clicked_cb(void *data, Evas_Object * obj, void *event_info);
static void _dialer_call_clicked_cb(void *data, Evas_Object * obj, void *event_info);
static void _dialer_suggest_contact_clicked_cb(void *data, Evas_Object * obj, void *event_info);
static void _dialer_contact_add_clicked_cb(void *data, Evas_Object * obj, void *event_info);
static void _dialer_message_clicked_cb(void *data, Evas_Object * obj, void *event_info);
static void _dialer_call_initiated_cb(GError *error, int call_id, void *userdata);
static void _delete_cb(struct View *view, Evas_Object * win, void *event_info);

int
dialer_view_init()
{
	g_debug("Initializing the dialer view");
	Evas_Object *win;
	int ret;
	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("Dialer"),
				NULL, NULL, NULL);

	if (ret) {
		g_critical("Failed to init dialer view");
		return ret;
	}

	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);

	ui_utils_view_layout_set(VIEW_PTR(view), phoneui_theme,
				 "phoneui/dialer/dialer");

	view.text_number = elm_label_add(win);
	elm_object_text_set(view.text_number, "");
	evas_object_size_hint_align_set(view.text_number, 0.0, 0.5);
	ui_utils_view_swallow(VIEW_PTR(view), "text_number", view.text_number);
	evas_object_show(view.text_number);

	view.text_number_info = elm_label_add(win);
	elm_object_text_set(view.text_number_info,
			    D_("Click to open contact list."));
	ui_utils_view_swallow(VIEW_PTR(view), "text_number_info", view.text_number_info);
	evas_object_show(view.text_number_info);

	view.delete_text_icon = elm_icon_add(win);
	evas_object_size_hint_aspect_set(view.delete_text_icon,
					 EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_file_set(view.delete_text_icon, DELETE_TEXT_ICON, NULL);

	view.delete_text_button = elm_button_add(win);
	elm_object_content_set(view.delete_text_button, view.delete_text_icon);
	evas_object_smart_callback_add(view.delete_text_button, "clicked",
				       _dialer_delete_clicked_cb, NULL);

	ui_utils_view_swallow(VIEW_PTR(view), "button_delete", view.delete_text_button);
	evas_object_show(view.delete_text_button);
	evas_object_show(view.delete_text_icon);


	view.keypad =
		(Evas_Object *) elm_keypad_add(win);
	evas_object_smart_callback_add(view.keypad, "clicked",
				       _dialer_keypad_clicked_cb, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "keypad", view.keypad);
	evas_object_show(view.keypad);

	view.bt_exit = elm_button_add(win);
	elm_object_text_set(view.bt_exit, D_("Close"));
	evas_object_smart_callback_add(view.bt_exit, "clicked",
				       _dialer_exit_clicked_cb, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_exit", view.bt_exit);
	evas_object_show(view.bt_exit);

	view.bt_suggest = elm_button_add(win);
	elm_object_text_set(view.bt_suggest, D_("Suggest"));
	evas_object_smart_callback_add(view.bt_suggest, "clicked",
				       _dialer_suggest_clicked_cb, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_suggest", view.bt_suggest);
	evas_object_show(view.bt_suggest);

	view.bt_options = elm_button_add(win);
	elm_object_text_set(view.bt_options, D_("More"));
	evas_object_smart_callback_add(view.bt_options, "clicked",
				       _dialer_options_clicked_cb, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_options", view.bt_options);
	evas_object_show(view.bt_options);

	view.bt_call = elm_button_add(win);
	elm_object_text_set(view.bt_call, D_("Call"));
	evas_object_smart_callback_add(view.bt_call, "clicked",
				       _dialer_call_clicked_cb, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_call", view.bt_call);
	evas_object_show(view.bt_call);

	edje_object_signal_callback_add(ui_utils_view_layout_get(VIEW_PTR(view)), "click",
					"number",
					_dialer_number_clicked_cb,
					NULL);

	view.contacts = 0;
	view.suggest_data = NULL;
	fill_suggest(&view.contacts, view.suggest_data);

	/* Suggest */
	view.hv2 = elm_hover_add(win);
	elm_hover_parent_set(view.hv2, win);
	elm_hover_target_set(view.hv2, view.bt_suggest);

	view.bx2 = elm_box_add(win);
	elm_box_horizontal_set(view.bx2, 0);
	elm_box_homogeneous_set(view.bx2, 1);
	evas_object_show(view.bx2);

	int iSuggest;

	for (iSuggest = MAX_SUGGEST - 1; iSuggest >= 0; --iSuggest) {
		view.bt_suggest_contact[iSuggest] = elm_button_add(win);
		elm_object_text_set(view.bt_suggest_contact[iSuggest], D_("Contact"));
		evas_object_size_hint_min_set(view.bt_suggest_contact[iSuggest], 130, 80);
		evas_object_smart_callback_add(view.bt_suggest_contact[iSuggest], "clicked",
				_dialer_suggest_contact_clicked_cb, GINT_TO_POINTER(iSuggest));
		evas_object_show(view.bt_suggest_contact[iSuggest]);
		elm_box_pack_end(view.bx2, view.bt_suggest_contact[iSuggest]);
	}

	elm_hover_content_set(view.hv2, "top", view.bx2);

	/* Options */
	view.hv = elm_hover_add(win);
	elm_hover_parent_set(view.hv, win);
	elm_hover_target_set(view.hv, view.bt_options);

	view.bx = elm_box_add(win);
	elm_box_horizontal_set(view.bx, 0);
	elm_box_homogeneous_set(view.bx, 1);
	evas_object_show(view.bx);

	view.bt_save = elm_button_add(win);
	elm_object_text_set(view.bt_save, D_("Save"));
	evas_object_size_hint_min_set(view.bt_save, 130, 80);
	evas_object_smart_callback_add(view.bt_save, "clicked",
				       _dialer_contact_add_clicked_cb, NULL);
	evas_object_show(view.bt_save);
	elm_box_pack_end(view.bx, view.bt_save);

	view.bt_message = elm_button_add(win);
	elm_object_text_set(view.bt_message, D_("Send SMS"));
	evas_object_size_hint_min_set(view.bt_message, 130, 80);
	evas_object_smart_callback_add(view.bt_message, "clicked",
				       _dialer_message_clicked_cb, NULL);
	evas_object_show(view.bt_message);
	elm_box_pack_end(view.bx, view.bt_message);

	elm_hover_content_set(view.hv, "top", view.bx);

	view.number[0] = '\0';
	view.length = 0;

	return 0;
}

int
dialer_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

void
fill_suggest(int *contacts, GList *suggest_data)
{
	phoneui_utils_contacts_get(contacts, _process_entry, suggest_data);
}

static void
_process_entry(void *_entry, void *_data)
{
	(void) _data;
	GHashTable *entry = (GHashTable *)_entry;

	view.suggest_data = g_list_prepend(view.suggest_data, entry);
}

void
dialer_view_deinit()
{
	ui_utils_view_deinit(VIEW_PTR(view));

	evas_object_smart_callback_del(view.keypad, "clicked",
				       _dialer_keypad_clicked_cb);
	evas_object_del(view.keypad);
	evas_object_del(view.bt_suggest);
	evas_object_del(view.bt_options);
	evas_object_del(view.bt_call);
	evas_object_del(view.bt_exit);
	evas_object_del(view.bt_message);
	evas_object_del(view.bt_save);
	int iSuggest;
	for (iSuggest = 0; iSuggest < MAX_SUGGEST; ++iSuggest) {
		evas_object_del(view.bt_suggest_contact[iSuggest]);
	}
	evas_object_del(view.bx);
	evas_object_del(view.bx2);
	evas_object_del(view.hv);
	evas_object_del(view.hv2);
	evas_object_del(view.text_number);
	evas_object_del(view.text_number_info);
	evas_object_del(view.delete_text_button);
	g_list_free(view.suggest_data);
}

void
dialer_view_show()
{
	evas_object_hide(view.hv);
	evas_object_hide(view.hv2);
	ui_utils_view_show(VIEW_PTR(view));
}

void
dialer_view_hide()
{
	_dialer_number_clear();
	ui_utils_view_hide(VIEW_PTR(view));
}


static void
_delete_cb(struct View *view, Evas_Object * win, void *event_info)
{
	(void)win;
	(void)event_info;
	(void)view;
	dialer_view_hide();
}

static void
_dialer_suggest_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;

	int iSuggest;
	for (iSuggest = 0; iSuggest < MAX_SUGGEST; ++iSuggest) {
		elm_object_text_set(view.bt_suggest_contact[iSuggest], D_(""));
		evas_object_hide(view.bt_suggest_contact[iSuggest]);
	}

	GList *it = g_list_first(view.suggest_data);
	int cSuggest = 0;
	while (it && cSuggest < MAX_SUGGEST) {
		GHashTable *properties = (GHashTable *) it->data;
		if (properties) {
			GVariant *tmp;
			const gchar *sPhone = NULL;
			const gchar *sName = NULL;

			tmp = g_hash_table_lookup(properties, "Phone");
			if(tmp) {
				sPhone = g_variant_get_string(tmp, NULL);
			} else {
				tmp = g_hash_table_lookup(properties, "Mobile phone");
				if(tmp) {
					sPhone = g_variant_get_string(tmp, NULL);
				} else {
					tmp = g_hash_table_lookup(properties, "Home phone");
					if(tmp) {
						sPhone = g_variant_get_string(tmp, NULL);
					} else {
						tmp = g_hash_table_lookup(properties, "Work phone");
						if(tmp) {
							sPhone = g_variant_get_string(tmp, NULL);
						}
					}
				}
			}

			if (sPhone) {
				if (strstr(sPhone, view.number) != NULL) {
					strcpy(view.suggestions[cSuggest], sPhone);

					tmp = g_hash_table_lookup(properties, "Name");
					if(tmp) {
						sName = g_variant_get_string(tmp, NULL);
					}
				}
			}

			if (sName) {
				elm_object_text_set(view.bt_suggest_contact[cSuggest], sName);
				evas_object_show(view.bt_suggest_contact[cSuggest]);
				++cSuggest;
			}
		}

		it = g_list_next(it);
	}

	if (cSuggest) {
		evas_object_show(view.hv2);
	}
}

static void
_dialer_options_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	evas_object_show(view.hv);
}

static void
_dialer_exit_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	dialer_view_hide();
}

static void
_dialer_suggest_contact_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	int contact = GPOINTER_TO_INT(data);

	strcpy(view.number, view.suggestions[contact]);
	view.length = strlen(view.number);

	_dialer_number_update();
	evas_object_hide(view.hv2);
}

static void
_dialer_contact_add_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	GHashTable *contact = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, common_utils_variant_unref);
	g_hash_table_insert(contact, "Phone",
			g_variant_ref_sink(g_variant_new_string(view.number)));
	phoneui_contacts_contact_new(contact);
	g_hash_table_destroy(contact);

	dialer_view_hide();
}

static void
_dialer_call_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	if (*view.number) {
		phoneui_utils_dial(view.number, _dialer_call_initiated_cb,
					    data);
	}
}

static void
_dialer_message_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "Phone",
			      g_variant_ref_sink(g_variant_new_string(view.number)));

	phoneui_messages_message_new(options);
	//g_hash_table_destroy(options);

	dialer_view_hide();
}

static void
_dialer_keypad_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	char input = ((char *) event_info)[0];

	if (view.length < 64) {
		view.number[view.length] = input;
		view.number[view.length + 1] = '\0';
		view.length++;
		_dialer_number_update();
	}
}

static void
_dialer_delete_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	int length = view.length;

	if (length) {
		view.number[length - 1] = '\0';
		view.length--;
		_dialer_number_update();
	}
}


static void
_dialer_number_clicked_cb(void *data, Evas_Object * obj, const char *emission,
			    const char *source)
{
	(void) data;
	(void) obj;
	(void) emission;
	(void) source;
	if (!*view.number) {
		phoneui_contacts_show();
		dialer_view_hide();
	}
}


/* FIXME: haven't reviewd this yet, though looks too hackish to believe */
static void
_dialer_number_update()
{
	int length = view.length;
	char *number = view.number;
	static char tmp[73];

	if (length < 7)
		elm_object_scale_set(view.text_number, 4.0);
	else if (length < 9)
		elm_object_scale_set(view.text_number, 3.0);
	else if (length < 24) {
		elm_object_scale_set(view.text_number, 2.0);
		if (length > 12) {
			tmp[0] = 0;
			strncat(tmp, number, 12);
			strcat(tmp, "<br>");
			strcat(tmp, number + 12);
			number = tmp;
		}
	}
	else {
		elm_object_scale_set(view.text_number, 1.0);
		if (length > 52) {
			tmp[0] = 0;
			strncat(tmp, number, 26);
			strcat(tmp, "<br>");
			strncat(tmp, number + 26, 26);
			strcat(tmp, "<br>");
			strcat(tmp, number + 52);
			number = tmp;
		}
		else if (length > 26) {
			tmp[0] = 0;
			strncat(tmp, number, 26);
			strcat(tmp, "<br>");
			strcat(tmp, number + 26);
			number = tmp;
		}
	}

	elm_object_text_set(view.text_number, number);
	if (length == 0) {
		edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
					"number_empty", "elm");
	}
	else {
		edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
					"number_available", "elm");
	}
}


static int
_dialer_number_clear()
{
	view.number[0] = '\0';
	view.length = 0;
	_dialer_number_update();

	return 0;
}

static void
_dialer_call_initiated_cb(GError * error, int call_id, void *userdata)
{
	(void) userdata;
	(void) call_id;

	if (error)
	{
		g_warning("Cannot initiate call: (%d) %s", error->code,
			error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Cannot initiate call."), error);
	}
	else
		dialer_view_hide();
}

