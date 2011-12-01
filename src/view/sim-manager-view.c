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

#include <glib.h>
#include <glib-object.h>

#include <freesmartphone.h>

#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils-contacts.h>
#include <phoneui/phoneui-utils-sim.h>
#include <phoneui/phoneui-info.h>

#include "common-utils.h"
#include "contact-list-common.h"
#include "sim-manager-view.h"
#include "ui-utils.h"
#include "ui-utils-contacts.h"
#include "phoneui-shr.h"
#include "views.h"

struct SimManagerListData {
	Evas_Object *layout;
	Evas_Object *list;
	Evas_Object *index;
	int count;
	int current;
};

struct SimManagerViewData {
	struct View view;
	struct SimManagerListData list_data;
	Evas_Object *bx, *hv;
	Evas_Object *bt_import_all, *bt_options, *bt_message, *bt_edit;
	Evas_Object *bt_delete, *bt_copy_to_sim, *pb;
	Evas_Object *notify;
	Eina_Bool pb_run;
	Ecore_Timer *pb_timer;
	Eina_Bool import_error;
};
static struct SimManagerViewData view;

struct SimContactData {
	FreeSmartphoneGSMSIMEntry *entry;
	int state;
};

static Elm_Genlist_Item_Class itc;


void sim_manager_list_fill(struct SimManagerListData *list_data);

/* progressbar functions taken from elementary_test*/
static Eina_Bool
_loading_indicator_value_set (void *data)
{
	(void) data;
	double progress;

	progress = elm_progressbar_value_get (view.pb);
	if (progress < 1.0) progress += 0.0123;
	else progress = 0.0;
	elm_progressbar_value_set(view.pb, progress);
	if (progress < 1.0) return ECORE_CALLBACK_RENEW;
	view.pb_run = 0;
	return ECORE_CALLBACK_CANCEL;
}

static void
loading_indicator_start()
{
	edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
				"start_loading","");
	elm_progressbar_pulse(view.pb, EINA_TRUE);
	if (!view.pb_run) {
		view.pb_timer = ecore_timer_add(0.1,
					_loading_indicator_value_set, NULL);
		view.pb_run = EINA_TRUE;
	}
}

static void
loading_indicator_stop()
{
	edje_object_signal_emit(elm_layout_edje_get(view.view.layout),
				"stop_loading","");
	elm_progressbar_pulse(view.pb, EINA_FALSE);
	if (view.pb_run) {
		ecore_timer_del(view.pb_timer);
		view.pb_run = EINA_FALSE;
	}
}

/* --- select contact functions --- */
typedef struct {
	struct View view;
	void (*callback)(const char *, void *);
	void *data;
	Evas_Object *inwin;
	Evas_Object *list, *layout, *cbutton, *sbutton;
	struct ContactListData contact_list_data;
	GList *numbers;
} _contact_select_pack;

static void
_contact_select_delete_cb(struct View *selview, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	ui_utils_view_deinit(selview);
}

static void
_contact_select_cancel(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	_contact_select_pack *pack = data;
	pack->callback(NULL, pack->data);
	ui_utils_view_deinit(&pack->view);
}

static void
_contact_select_add(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	_contact_select_pack *pack = data;
	Elm_Genlist_Item *it;
	GHashTable *properties;
	char *path = NULL;
	GVariant *p;

	it = elm_genlist_selected_item_get(pack->contact_list_data.list);
	properties = it ? (GHashTable *) elm_genlist_item_data_get(it) : NULL;

	if (properties) {
		p = g_hash_table_lookup(properties, "Path");
		if (p) {
			path = g_variant_dup_string(p, NULL);
		}
	}

	pack->callback(path, pack->data);
	ui_utils_view_deinit(&pack->view);
}

static void
contact_select_view(void (*cb)(const char *, void *), void *data)
{
	Evas_Object *win;
	int ret;
	_contact_select_pack *pack =
			g_malloc(sizeof(_contact_select_pack));
	pack->callback = cb;
	pack->data = data;

	ret = ui_utils_view_init(VIEW_PTR(*pack), ELM_WIN_BASIC,
				 D_("Select Contact"), NULL, NULL, NULL);

	if (ret) {
		g_critical("Failed to init sim manager view");
		return;
	}

	win = ui_utils_view_window_get(VIEW_PTR(*pack));
	ui_utils_view_delete_callback_set(VIEW_PTR(*pack), _contact_select_delete_cb);
	pack->view.win = win;

	ui_utils_view_layout_set(VIEW_PTR(*pack), phoneui_theme,
				 "phoneui/messages/new/contacts");
	elm_theme_extension_add(NULL, phoneui_theme);
	pack->contact_list_data.view = &pack->view;
	pack->contact_list_data.layout = pack->view.layout;
	contact_list_add(&pack->contact_list_data);

	pack->cbutton = elm_button_add(win);
	elm_object_text_set(pack->cbutton, D_("Cancel"));
	evas_object_smart_callback_add(pack->cbutton, "clicked",
				       _contact_select_cancel, pack);
	ui_utils_view_swallow(VIEW_PTR(pack), "contacts_button_back",
			      pack->cbutton);
	evas_object_show(pack->cbutton);

	pack->sbutton = elm_button_add(win);
	elm_object_text_set(pack->sbutton, D_("Select"));
	evas_object_smart_callback_add(pack->sbutton, "clicked",
				       _contact_select_add, pack);
	ui_utils_view_swallow(VIEW_PTR(*pack), "contacts_button_add",
			      pack->sbutton);
	evas_object_show(pack->sbutton);

	g_debug("fill contact list");
	contact_list_fill(&pack->contact_list_data);

	ui_utils_view_show(VIEW_PTR(*pack));
}

/* --- genlist callbacks --- */
static char *
gl_text_get(void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	const struct SimContactData *cdata = data;

	if (!strcmp(part, "elm.text")) {
		if (cdata->entry->name && *cdata->entry->name) {
			return strdup(cdata->entry->name);
		}
	}
	else if (!strcmp(part, "elm.text.sub")) {
		if (cdata->entry->number && *cdata->entry->number) {
			return strdup(cdata->entry->number);
		}
	}
	return NULL;
}

static Eina_Bool
gl_state_get(void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	(void) data;
	(void) part;
	return 0;
}

static void
gl_del(void *data, Evas_Object * obj)
{
	(void) obj;
	(void) data;
	// FIXME: how to free if it is const?
// 	FreeSmartphoneGSMSIMEntry *entry =  data;
// 	free_smartphone_gsm_sim_entry_free(entry);
}

static void
_list_edit_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	evas_object_hide(view.hv);
}

/* add contact functions */
typedef struct {
	struct View* view;
	const char *path;
	const char *name;
	const char *number;
	int index;
	Evas_Object *inwin;
	Evas_Object *name_entry;
	Evas_Object *number_entry;
} _number_add_pack;

static gboolean
_number_add_destruct(gpointer data)
{
	_number_add_pack *pack = (_number_add_pack *) data;
	g_debug("Destructing number add inwin");
	evas_object_del(pack->inwin);
	free (pack);
	return FALSE;
}

void
_number_add_add_to_sim(GError *error, gpointer pack)
{
	if (error) {
		g_warning("Failed to write to SIM: (%d) %s", error->code,
			error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Failed to write to SIM."), error);
	} else {
		/*
		elm_genlist_clear(view.list_data.list);
		sim_manager_list_fill(&view.list_data);
		*/
		_number_add_pack *cpack = (_number_add_pack *) pack;

		FreeSmartphoneGSMSIMEntry *entry;
		entry = malloc(sizeof(FreeSmartphoneGSMSIMEntry));
		entry->index = cpack->index;
		entry->name = g_strdup(cpack->name);
		entry->number = g_strdup(cpack->number);

		struct SimContactData *data;
		data = malloc(sizeof(*data));
		data->entry = entry;
		data->state = 0;

		elm_genlist_item_append(view.list_data.list, &itc, data,
				     NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}
	g_timeout_add(0, _number_add_destruct, pack);
}

int
_find_next_free_index(int max_index) {
	int i, found;
	Elm_Genlist_Item *it;
	const struct SimContactData *entry;

	for (i = 1; i <= max_index; i++) {
		found = 0;
		it = elm_genlist_first_item_get(view.list_data.list);
		entry = elm_genlist_item_data_get(it);
		while (entry) {
			if (entry->entry->index == i) found = 1;
			it = elm_genlist_item_next_get(it);
			entry = elm_genlist_item_data_get(it);
		}
		if (found == 0) {
			return i;
		}
	}

	return -1;
}

void
_number_add_find_index_cb(GError *error, int max_index, int number_length,
			   int name_length, gpointer data)
{
	(void)number_length;
	(void)name_length;
	int min_index;
	_number_add_pack *pack = (_number_add_pack *) data;

	if (error) {
		g_warning("Failed retrieving Phonebook Info: (%d) %s",
			  error->code, error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Failed retrieving Phonebook Info."), error);
		g_timeout_add(0, _number_add_destruct, pack);
		return;
	}

	min_index = _find_next_free_index(max_index);

	if (min_index > 0) {
		pack->index = min_index;
		g_debug("Save contact: %s (%s) to index %d", pack->name,
			pack->number, min_index);
		phoneui_utils_sim_contact_store(SIM_CONTACTS_CATEGORY,
					min_index, pack->name, pack->number,
					_number_add_add_to_sim, pack);
	} else {
		// FIXME: show notification
		g_warning("Failed to find an empty index on SIM!");
		g_timeout_add(0, _number_add_destruct, pack);
	}
}

static void
_number_add_save(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	_number_add_pack *pack = (_number_add_pack *) data;
	pack->name = elm_entry_markup_to_utf8(
				elm_entry_entry_get(pack->name_entry));
	pack->number = elm_entry_markup_to_utf8(
				elm_entry_entry_get(pack->number_entry));
	phoneui_utils_sim_phonebook_info_get(SIM_CONTACTS_CATEGORY,
				       _number_add_find_index_cb, pack);
}

static void
_number_add_cancel(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	_number_add_pack *pack = (_number_add_pack *) data;
	g_timeout_add(0, _number_add_destruct, pack);
}

void
_number_add_contact_get_cb(GError *error, GHashTable *properties, gpointer data)
{
	_number_add_pack *pack = (_number_add_pack *) data;
	pack->name = phoneui_utils_contact_display_name_get(properties);

	if (error || !properties) {
		g_warning("Failed to retrieve contact: %s [error (%d) %s]",
			pack->path,	(error)? error->code : 0,
			(error)? error->message : "NULL");
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Failed to retrieve contact."), error);
		g_timeout_add(0, _number_add_destruct, pack);
		return;
	}

	Evas_Object *name_lb, *sbtn, *win, *box, *number_lb, *fr0, *fr1;
	Evas_Object *cbtn, *box0;

	win = ui_utils_view_window_get(VIEW_PTR(view));
	g_debug("add the inwin");
	pack->inwin = elm_win_inwin_add(win);

	g_debug("add the box");
	box = elm_box_add(win);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
	elm_win_resize_object_add(win, box);
	evas_object_show(box);

	g_debug("add name label");
	name_lb = elm_label_add(win);
	elm_win_resize_object_add(win, name_lb);
	evas_object_size_hint_weight_set(name_lb, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(name_lb, EVAS_HINT_FILL, 0.0);
	elm_object_text_set(name_lb, D_("Name:"));
	evas_object_show(name_lb);
	elm_box_pack_end(box, name_lb);

	fr0 = elm_frame_add(win);
	elm_object_style_set(fr0, "outdent_bottom");
	evas_object_size_hint_weight_set(fr0, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(fr0, EVAS_HINT_FILL, 0.0);
	elm_box_pack_end(box, fr0);
	evas_object_show(fr0);

	// FIXME: entry expands too much on focus
	g_debug("add name entry: %s", pack->name);
	pack->name_entry = elm_entry_add(win);
	elm_entry_single_line_set(pack->name_entry, EINA_TRUE);
	elm_entry_editable_set(pack->name_entry, EINA_TRUE);
	elm_entry_entry_set(pack->name_entry,
			    elm_entry_utf8_to_markup(pack->name));
	elm_object_content_set(fr0, pack->name_entry);
	evas_object_show(pack->name_entry);

	g_debug("add number label");
	number_lb = elm_label_add(win);
	elm_win_resize_object_add(win, number_lb);
	evas_object_size_hint_weight_set(number_lb, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(number_lb, EVAS_HINT_FILL, 0.0);
	elm_object_text_set(number_lb, D_("Number:"));
	evas_object_show(number_lb);
	elm_box_pack_end(box, number_lb);

	fr1 = elm_frame_add(win);
	elm_object_style_set(fr1, "outdent_bottom");
	evas_object_size_hint_weight_set(fr1, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(fr1, EVAS_HINT_FILL, 0.0);
	elm_box_pack_end(box, fr1);
	evas_object_show(fr1);

	g_debug("add number entry: %s", pack->number);
	pack->number_entry = elm_entry_add(win);
	elm_entry_single_line_set(pack->number_entry, EINA_TRUE);
	elm_entry_editable_set(pack->number_entry, EINA_TRUE);
	elm_entry_entry_set(pack->number_entry,
			    elm_entry_utf8_to_markup(pack->number));
	elm_object_content_set(fr1, pack->number_entry);
	evas_object_show(pack->number_entry);

	g_debug("add the bottom box");
	box0 = elm_box_add(win);
	elm_box_horizontal_set(box0, EINA_TRUE);
	evas_object_size_hint_weight_set(box0, EVAS_HINT_EXPAND, 0.0);
	elm_box_pack_end(box, box0);
	evas_object_show(box0);

	sbtn = elm_button_add(win);
	elm_win_resize_object_add(win, sbtn);
	evas_object_size_hint_weight_set(sbtn, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(sbtn, EVAS_HINT_FILL, 0.0);
	evas_object_smart_callback_add(sbtn, "clicked",
				       _number_add_save, pack);
	elm_object_text_set(sbtn, D_("Save"));
	evas_object_show(sbtn);
	elm_box_pack_start(box0, sbtn);

	cbtn = elm_button_add(win);
	elm_win_resize_object_add(win, cbtn);
	evas_object_size_hint_weight_set(cbtn, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(cbtn, EVAS_HINT_FILL, 0.0);
	evas_object_smart_callback_add(cbtn, "clicked",
				       _number_add_cancel, pack);
	elm_object_text_set(cbtn, D_("Cancel"));
	evas_object_show(cbtn);
	elm_box_pack_end(box0, cbtn);

	elm_win_inwin_content_set(pack->inwin, box);
	elm_win_inwin_activate(pack->inwin);
}

void
_number_add_cb(const char *number, void *data)
{
	g_debug("sim-manager: selected number: %s", number);
	_number_add_pack *pack = (_number_add_pack *) data;
	pack->number = g_strdup(number);
	phoneui_utils_contact_get(pack->path, _number_add_contact_get_cb, pack);
}

void
_list_copy_to_sim_cb(const char *path, void *data)
{
	g_debug("sim-manager: selected contact: %s", path);
	_number_add_pack *pack = (_number_add_pack *) data;
	pack->path = g_strdup(path);
	ui_utils_contacts_contact_number_select(VIEW_PTR(view), path,
						_number_add_cb, data);
}

static void
_list_copy_to_sim_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	(void) data;
	_number_add_pack *pack = g_malloc(sizeof(_number_add_pack));
	contact_select_view( _list_copy_to_sim_cb, pack);
}

/* import functions */
static void
_import_one_contact_cb(GError *error, char *path, void *data)
{
	(void) path;
	(void) data;
	loading_indicator_stop();
	Elm_Genlist_Item *it = data;
	struct SimContactData *cdata =
			(struct SimContactData *)elm_genlist_item_data_get(it);
	if (error) {
		g_warning("importing one contact failed: (%d) %s",
			  error->code, error->message);
		view.notify = ui_utils_notify(
				ui_utils_view_window_get(VIEW_PTR(view)),
				D_("Importing contact failed"), 10);
		cdata->state = 1;
	}
	else {
		g_debug("contact imported ok");
		view.notify =
			ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
					D_("Contact successfully imported"), 10);
		cdata->state = 0;
	}
	evas_object_show(view.notify);
}

static void
_import_all_contacts_cb(GError *error, char *path, void *data)
{
	(void) path;
	Elm_Genlist_Item *it = data;
	struct SimContactData *cdata =
			(struct SimContactData *)elm_genlist_item_data_get(it);
	if (error) {
		cdata->state = 1;
		view.import_error = EINA_TRUE;
		g_warning("Adding the contact failed: (%d) %s",
			  error->code, error->message);
	}
	else {
		cdata->state = 0;
	}
	if (it == elm_genlist_last_item_get(view.list_data.list)) {
		g_debug("import finished");
		loading_indicator_stop();
		if (view.import_error) {
			ui_utils_dialog(VIEW_PTR(view),
				D_("Import had errors! Failed entries are marked."),
				DIALOG_OK, NULL, NULL);
		}
		else {
			view.notify = ui_utils_notify(ui_utils_view_window_get
					(VIEW_PTR(view)),
					D_("All contacts added succesfully"), 10);
	                evas_object_show(view.notify);
		}
	}
	free(cdata);
}

static void
_import_contact(Elm_Genlist_Item *it,
		void (*callback)(GError *, char *, gpointer))
{
	g_debug("_import_contact()");
	GVariant *tmp;

	if (!it) {
		return;
	}
	const struct SimContactData *cdata = elm_genlist_item_data_get(it);
	if (cdata->entry) {
		GHashTable *qry = g_hash_table_new_full
		     (g_str_hash, g_str_equal, NULL, common_utils_variant_unref);
		tmp = g_variant_new_string(cdata->entry->name);
		g_hash_table_insert(qry, "Name", g_variant_ref_sink(tmp));
		tmp = g_variant_new_string(cdata->entry->number);
		g_hash_table_insert(qry, "Phone", g_variant_ref_sink(tmp));
		phoneui_utils_contact_add(qry, callback, it);
		g_hash_table_unref(qry);
	}
}

static void
_list_import_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;

	evas_object_hide(view.hv);
	loading_indicator_start();
	_import_contact(elm_genlist_selected_item_get(view.list_data.list),
			_import_one_contact_cb);
}

static void
_list_import_all_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	Elm_Genlist_Item *it;

	loading_indicator_start();
	view.import_error = EINA_FALSE;
	it = elm_genlist_first_item_get(view.list_data.list);
	while (it) {
		_import_contact(it, _import_all_contacts_cb);
		it = elm_genlist_item_next_get(it);
	}
}

/* delete functions */
static void
_contact_delete_confirmiation_cb(GError *error, gpointer data)
{
	if (error) {
		g_warning("Error while deleting entry!");
		ui_utils_dialog(VIEW_PTR(view),
			D_("Error while deleting entry!"),
			DIALOG_OK, NULL, NULL);
	}
	else {
		Elm_Genlist_Item *it = data;
		elm_genlist_item_del(it);
	}
}

static void
_contact_delete_confirm_cb(int result, void *data)
{
	if (!data || result != DIALOG_YES)
		return;

	Elm_Genlist_Item *it = data;
	const struct SimContactData *cdata = elm_genlist_item_data_get(it);
	if (cdata->entry) {
		phoneui_utils_sim_contact_delete(SIM_CONTACTS_CATEGORY,
			cdata->entry->index, _contact_delete_confirmiation_cb, it);
	}
}

static void
_list_delete_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	Elm_Genlist_Item *it;

	evas_object_hide(view.hv);

	it = elm_genlist_selected_item_get(view.list_data.list);
	if (it) {
		ui_utils_dialog(VIEW_PTR(view),
			D_("Really delete this contact?"),
			DIALOG_YES|DIALOG_NO, _contact_delete_confirm_cb, it);
	}
}

/* main ui functions */
static void
_list_options_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	evas_object_show(view.hv);
}

void
sim_manager_list_add(struct SimManagerListData *list_data)
{
	Evas_Object *win;
	win = ui_utils_view_window_get(VIEW_PTR(view));
	list_data->index = NULL;
	list_data->list = elm_genlist_add(win);
	elm_genlist_horizontal_set(list_data->list, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(list_data->list, 0.0, 0.0);
	elm_object_scale_set(list_data->list, 1.0);
	itc.item_style = "sim-contact";
	itc.func.text_get = gl_text_get;
	itc.func.state_get = gl_state_get;
	itc.func.del = gl_del;
	evas_object_show(list_data->list);
	if (list_data->layout) {
		elm_object_part_content_set(list_data->layout, "contacts_list",
				list_data->list);
	}
}

Elm_Genlist_Item *
sim_manager_list_item_add(struct SimManagerListData *list_data,
			  FreeSmartphoneGSMSIMEntry *entry)
{
	struct SimContactData *data;

	data = malloc(sizeof(*data));
	data->entry = entry;
	data->state = 0;

	g_debug("sim_manager_list_item_add(%s)", entry ? entry->name : "NULL");
	return elm_genlist_item_append(list_data->list, &itc, data,
				     NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}

static void
_sim_contacts_get_callback(GError *error, FreeSmartphoneGSMSIMEntry *entry,
			   int count, gpointer data)
{
	int i;
	struct SimManagerListData *list_data = data;

	if (error) {
		g_warning("Failed retrieving SIM Phonebook: (%d) %s",
			  error->code, error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Failed retrieving SIM Phonebook."), error);
		// FIXME: destroy SIM Manager
		return;
	}

	for (i = 0; i < count; i++) {
		g_debug("%d [%d]: %s / %s", i, entry->index,
			entry->name, entry->number);
		sim_manager_list_item_add(list_data,
				free_smartphone_gsm_sim_entry_dup(entry));
		entry++;
	}
	loading_indicator_stop();
}

void
sim_manager_list_fill(struct SimManagerListData *list_data)
{
	loading_indicator_start();
	g_debug("sim_manager_list_fill()");
	list_data->current = 0;
	phoneui_utils_sim_contacts_get(SIM_CONTACTS_CATEGORY,
				       _sim_contacts_get_callback, list_data);
}

static void
_delete_cb(struct View *view, Evas_Object *obj, void *event_info)
{
	(void)view;
	(void)obj;
	(void)event_info;
	loading_indicator_stop();
	sim_manager_view_deinit();
}

int
sim_manager_view_init()
{
	Evas_Object *win;
	int ret;
	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC,
				 D_("SIM Manager"), NULL, NULL, NULL);

	if (ret) {
		g_critical("Failed to init sim manager view");
		return ret;
	}

	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);
	view.view.win = win;

	ui_utils_view_layout_set(VIEW_PTR(view), phoneui_theme,
				 "phoneui/settings/sim-manager");
	elm_theme_extension_add(NULL, phoneui_theme);
	view.list_data.layout = view.view.layout;
	sim_manager_list_add(&view.list_data);

	view.bt_import_all = elm_button_add(win);
	elm_object_text_set(view.bt_import_all, D_("Import all"));
	evas_object_smart_callback_add(view.bt_import_all, "clicked",
				       _list_import_all_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_import_all",
			      view.bt_import_all);
	evas_object_show(view.bt_import_all);

	view.bt_copy_to_sim = elm_button_add(win);
	elm_object_text_set(view.bt_copy_to_sim, D_("Copy to SIM"));
	evas_object_smart_callback_add(view.bt_copy_to_sim, "clicked",
				       _list_copy_to_sim_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_new",
			      view.bt_copy_to_sim);
	evas_object_show(view.bt_copy_to_sim);

	view.bt_options = elm_button_add(win);
	elm_object_text_set(view.bt_options, D_("Options"));
	evas_object_smart_callback_add(view.bt_options, "clicked",
				       _list_options_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_options",
			      view.bt_options);
	evas_object_show(view.bt_options);

	/* Options */
	view.hv = elm_hover_add(win);
	elm_hover_parent_set(view.hv, win);
	elm_hover_target_set(view.hv, view.bt_options);

	view.bx = elm_box_add(win);
	elm_box_horizontal_set(view.bx, 0);
	elm_box_homogeneous_set(view.bx, 1);
	evas_object_show(view.bx);

	view.bt_message = elm_button_add(win);
	elm_object_text_set(view.bt_message, D_("Import"));
	evas_object_size_hint_min_set(view.bt_message, 130, 80);
	evas_object_smart_callback_add(view.bt_message, "clicked",
				       _list_import_clicked, NULL);
	evas_object_show(view.bt_message);
	elm_box_pack_end(view.bx, view.bt_message);

	view.bt_edit = elm_button_add(win);
	elm_object_text_set(view.bt_edit, D_("Edit"));
	evas_object_size_hint_min_set(view.bt_edit, 130, 80);
	evas_object_smart_callback_add(view.bt_edit, "clicked",
				       _list_edit_clicked, NULL);
	evas_object_show(view.bt_edit);
	elm_box_pack_end(view.bx, view.bt_edit);

	view.bt_delete = elm_button_add(win);
	elm_object_text_set(view.bt_delete, D_("Delete"));
	evas_object_size_hint_min_set(view.bt_delete, 130, 80);
	evas_object_smart_callback_add(view.bt_delete, "clicked",
				       _list_delete_clicked, NULL);
	evas_object_show(view.bt_delete);
	elm_box_pack_end(view.bx, view.bt_delete);

	elm_hover_content_set(view.hv, "top", view.bx);

	/* loading indicator */
	view.pb = elm_progressbar_add(win);
	elm_object_style_set(view.pb, "wheel");
	elm_object_text_set(view.pb, D_("Loading..."));
	evas_object_size_hint_align_set(view.pb, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(view.pb, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	ui_utils_view_swallow(VIEW_PTR(view), "loading_indicator",
			      view.pb);
	evas_object_show(view.pb);

	sim_manager_list_fill(&view.list_data);

	return 0;
}

int
sim_manager_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

void
sim_manager_view_deinit()
{
	ui_utils_view_deinit(VIEW_PTR(view));
}

void
sim_manager_view_show()
{
	evas_object_hide(view.hv);
	ui_utils_view_show(VIEW_PTR(view));
}

void
sim_manager_view_hide()
{
	ui_utils_view_hide(VIEW_PTR(view));
}
