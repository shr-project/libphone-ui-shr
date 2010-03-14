
#include <glib.h>
#include <phoneui/phoneui-utils.h>
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
	void (*callback)(char *, void *);
	void *data;
	const char *path;
	char **phone_fields;
	GList *numbers;
};

static void
_add_number_to_list(gpointer key, gpointer value, gpointer data)
{
	char **fld;
	struct _number_select_pack *pack = data;
	for (fld = pack->phone_fields; *fld; fld++) {
		if (!strcmp(*fld, key)) {
			pack->numbers = g_list_append(pack->numbers, value);
		}
	}
}

static void
_contact_get_cb(GHashTable *contact, gpointer data)
{
	struct _number_select_pack *pack = data;
	g_hash_table_foreach(contact, _add_number_to_list, pack);

}

static void
_fields_get_cb(char ** fields, gpointer data)
{
	struct _number_select_pack *pack = data;
	if (!fields) {
		g_warning("No phonenumber fields defined!!!");
		return;
	}
	pack->phone_fields = fields;
	phoneui_utils_contact_get(pack->path, _contact_get_cb, pack);
}

void
ui_utils_contacts_contact_number_select(const char* path,
					void (*cb)(char *, void *),
					void *data)
{
	struct _number_select_pack *pack =
		malloc(sizeof(struct _number_select_pack));
	pack->callback = cb;
	pack->data = data;
	pack->path = path;
	pack->phone_fields = NULL;
	pack->numbers = NULL;
	phoneui_utils_contacts_fields_get_with_type("phonenumber",
						    _fields_get_cb, pack);
}
