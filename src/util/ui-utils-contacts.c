
#include <glib.h>
#include <glib-object.h>
#include <phoneui/phoneui-utils.h>
#include "phoneui-shr.h"
#include "common-utils.h"
#include "ui-utils.h"
#include "ui-utils-contacts.h"

struct _field_select_pack {
	void (*callback)(const char *, void *);
	void *data;
	struct View *view;
};

static void
_field_select_cb(GHashTable *fields, gpointer data)
{
	GList *keys;
	struct _field_select_pack *pack = (struct _field_select_pack *)data;
	if (!fields) {
		g_warning("No fields for contacts?");
		// TODO: show a user visible message
		return;
	}
	keys = g_hash_table_get_keys(fields);
	keys = g_list_sort(keys, (GCompareFunc) strcmp);
	ui_utils_view_inwin_list(pack->view, keys, pack->callback, pack->data);
	free(pack);
}

void
ui_utils_contacts_field_select(struct View *view,
			void (*callback)(const char *, void *), void *data)
{
	struct _field_select_pack *pack =
		malloc(sizeof(struct _field_select_pack));
	pack->callback = callback;
	pack->data = data;
	pack->view = view;
	phoneui_utils_contacts_fields_get(_field_select_cb, pack);
}

struct _number_select_pack {
	void (*callback)(const char *, void *);
	void *data;
	const char *path;
	struct View* view;
	Evas_Object *inwin;
	Evas_Object *list;
	GList *numbers;
};

struct _number_entry {
	char *field;
	char *number;
};


static gboolean _number_select_destruct(gpointer data)
{
	struct _number_select_pack *pack = data;
	g_debug("Destructing number selection inwin");
	evas_object_del(pack->inwin);
	free (pack);
	return FALSE;
}

static void
_number_select_cancel(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct _number_select_pack *pack = data;
	pack->callback(NULL, pack->data);
	g_timeout_add(0, _number_select_destruct, pack);
}

static void
_number_select_select(void *data, Evas_Object *obj, void *event_info)
{
	(void) event_info;
	struct _number_entry *entry = NULL;
	struct _number_select_pack *pack = data;
	Elm_List_Item *it = elm_list_selected_item_get(obj);
	if (it) {
		entry = elm_list_item_data_get(it);
	}

	if (entry) {
		g_debug("Calling callback with number %s", entry->number);
		if (pack) {
			pack->callback(entry->number, pack->data);
		}
		else {
			g_warning("NO pack... NO callback!!!");
		}
		g_debug("callback done");
	}
	g_debug("Scheduling inwin destruction");
	g_timeout_add(0, _number_select_destruct, pack);
}

static void
_add_number_to_list(gpointer _key, gpointer _value, gpointer data)
{
	Evas_Object *ico;
	Elm_List_Item *it;
	struct _number_select_pack *pack = (struct _number_select_pack *)data;
	struct _number_entry *entry = malloc(sizeof(struct _number_entry));
	entry->field = strdup(_key);
	entry->number = strdup(g_value_get_string(_value));
	g_debug("Adding %s=%s", entry->field, entry->number);
	ico = elm_icon_add(ui_utils_view_window_get(pack->view));
	elm_icon_scale_set(ico, 1, 1);
	elm_icon_file_set(ico, phoneui_theme, "icon/phone");
	it = elm_list_item_append(pack->list, entry->number, ico, NULL, NULL, entry);
}
static void
_fields_get_cb(GHashTable *contact, gpointer data)
{
	Evas_Object *win, *box, *btn;
	struct _number_select_pack *pack = data;
	const char *number;

	/* there is no number - pass the callback a NULL for notification */
	if (!contact) {
		g_message("No phonenumber fields defined for contact %s!!!",
			  pack->path);
		pack->callback(NULL, pack->data);
		return;
	}

	common_utils_debug_dump_hashtable(contact);

	/* if there is just one phonenumber field return it directly */
	if (g_hash_table_size(contact) == 1) {
		g_debug("Contact has exactly one phone number... passing");
		// FIXME: handle multiple numbers of same field type
		GList *l;
		l = g_hash_table_get_values(contact);
		if (l && l->data) {
			number = g_value_get_string(l->data);
			g_debug("Calling callback with number=%s", number);
			pack->callback(number, pack->data);
		}
		g_hash_table_unref(contact);
		free(pack);
		return;
	}

	g_debug("There is more numbers... pop up the dialog");
	/* there are more numbers... show them in a dialog */
	win = ui_utils_view_window_get(pack->view);
	g_debug("add the inwin");
	pack->inwin = elm_win_inwin_add(win);

	g_debug("add the box");
	box = elm_box_add(win);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, box);
	evas_object_show(box);

	g_debug("add the list");
	pack->list = elm_list_add(win);
	evas_object_size_hint_weight_set(pack->list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pack->list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_list_horizontal_mode_set(pack->list, ELM_LIST_LIMIT);
	evas_object_smart_callback_add(pack->list, "selected",
				       _number_select_select, pack);
// 	elm_object_scale_set(pack->list, 1.0);
	g_hash_table_foreach(contact, _add_number_to_list, pack);
	elm_list_go(pack->list);
        evas_object_show(pack->list);
	elm_box_pack_end(box, pack->list);

	g_debug("add the button");
	btn = elm_button_add(win);
	elm_win_resize_object_add(win, btn);
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0.0);
	evas_object_smart_callback_add(btn, "clicked",
				       _number_select_cancel, pack);
	elm_button_label_set(btn, D_("Cancel"));
	evas_object_show(btn);
	elm_box_pack_end(box, btn);

	elm_win_inwin_content_set(pack->inwin, box);
	elm_win_inwin_activate(pack->inwin);
}

void
ui_utils_contacts_contact_number_select(struct View *view, const char* path,
					void (*cb)(const char *, void *),
					void *data)
{
	struct _number_select_pack *pack =
		malloc(sizeof(struct _number_select_pack));
	g_debug("Starting phone number selection");
	pack->callback = cb;
	pack->data = data;
	pack->path = path;
	pack->view = view;
	phoneui_utils_contact_get_fields_for_type(path, "phonenumber",
						  _fields_get_cb, pack);
}
