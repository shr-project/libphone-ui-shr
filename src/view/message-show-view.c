#include "views.h"
#include "common-utils.h"
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>

struct MessageShowViewData {
	struct Window *win;
	char *status, *number, *content, *name, *date, *path;
	int id;
	GHashTable *properties;
	GHashTable *query;
	GValueArray *message;
	Evas_Object *lbl_content, *sc_content, *bt1, *bt2, *bt3, *hv, *bx,
		    *hbt1, *hbt2, *hbt3;

	void (*callback) ();
	void *callback_data;
};

/* FIXME: should be removed? */
typedef enum {
	MODE_FOLDERS,
	MODE_LIST,
	MODE_MESSAGE,
	MODE_NEW1,
	MODE_NEW2,
	MODE_DELETE
} MessagesModes;



void
*message_show_view_show(struct Window *win, void *_options);
void
message_show_view_hide(void *_data);

static void
message_show_view_close_clicked(void *_data, Evas_Object * obj,
				void *event_info);
static void
message_show_view_answer_clicked(void *_data, Evas_Object * obj,
				 void *event_info);
static void
message_show_view_delete_clicked(void *_data, Evas_Object * obj,
				 void *event_info);
static void
  message_show_view_delete_callback(void *_data);
static void
message_show_view_call_clicked(void *_data, Evas_Object * obj,
			       void *event_info);
static void
message_show_view_new_contact_clicked(struct MessageShowViewData *data,
				      Evas_Object * obj, void *event_info);
static void
  my_hover_bt_1(void *_data, Evas_Object * obj, void *event_info);

static void
  name_callback(GError * error, char *name, gpointer _data);
static void
  retrieve_callback(GHashTable * properties, gpointer _data);



/* --- main message show view functions ------------------------------------- */

void *
message_show_view_show(struct Window *win, void *_options)
{
	GHashTable *options = (GHashTable *) _options;
	if (!win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	if (!options) {
		g_critical("Passed NULL options (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}

	g_debug("message_show_view_show()");

	struct MessageShowViewData *data =
		common_utils_object_ref(calloc(1, sizeof(struct MessageShowViewData)));
	data->win = win;

	char *direction, *status, *tmp;
	status = g_hash_table_lookup(options, "status");
	direction = g_hash_table_lookup(options, "direction");
	tmp = malloc(strlen(status) + strlen(direction) + 4);
	sprintf(tmp, "%s (%s)", status, direction);

	data->path = g_hash_table_lookup(options, "path");
	data->name = g_hash_table_lookup(options, "name");
	data->number = g_hash_table_lookup(options, "number");
	data->content = g_hash_table_lookup(options, "content");
	data->status = tmp;
	data->date = g_hash_table_lookup(options, "date");

	if (options != NULL) {
		data->callback =
			g_hash_table_lookup(options, "delete_callback");
		data->callback_data =
			g_hash_table_lookup(options, "delete_callback_data");
	}

	/*FIXME: no need to do it if already read */
	phoneui_utils_message_set_read_status(data->path, 1, NULL, NULL);

	retrieve_callback(options, data);

	return data;
}

void
message_show_view_hide(void *_data)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;
	struct Window *win = data->win;

	g_debug("message_show_view_hide()");

	evas_object_del(data->lbl_content);
	evas_object_del(data->sc_content);
	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->bt3);
	evas_object_del(data->hbt1);
	evas_object_del(data->hbt2);
	evas_object_del(data->hbt3);
	evas_object_del(data->bx);
	evas_object_del(data->hv);

	/*FIXME shouldn't win be freed as well? */
	data->win = NULL;
	common_utils_object_unref(data);
}


/* --- evas callbacks ------------------------------------------------------- */

static void
message_show_view_close_clicked(void *_data, Evas_Object * obj,
				void *event_info)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_close_clicked()");

	window_destroy(data->win, NULL);
}

static void
message_show_view_answer_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_answer_clicked()");

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "name", data->name);
	g_hash_table_insert(options, "number", data->number);

	phoneui_messages_message_new(options);
	g_hash_table_destroy(options);
}

static void
message_show_view_call_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;
	char *number = common_utils_skip_prefix(data->number, "tel:");
	g_debug("message_show_view_call_clicked()");
	
	phoneui_utils_dial(number, NULL, NULL);
}

static void
message_show_view_delete_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_delete_clicked()");

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "path", data->path);
	g_hash_table_insert(options, "delete_callback",
			    message_show_view_delete_callback);
	g_hash_table_insert(options, "delete_callback_data", data);

	struct Window *win = window_new(D_("Delete Message"));
	window_init(win);
	window_view_show(win, options, message_delete_view_show,
			 message_delete_view_hide, NULL);
}

static void
message_show_view_delete_callback(void *_data)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_delete_callback()");

	window_destroy(data->win, NULL);

	if (data->callback != NULL)
		data->callback(data->callback_data);
}

static void
my_hover_bt_1(void *_data, Evas_Object * obj, void *event_info)
{
	Evas_Object *hv = (Evas_Object *) _data;

	g_debug("my_hover_bt_1()");

	evas_object_show(hv);
}



/* callbacks */


static void
message_common_name_callback(GHashTable *contact, void *_data)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	if (!data->win) {
		common_utils_object_unref_free(data);
		return;
	}
	
	if (contact) {
		data->name = phoneui_utils_contact_display_name_get(contact);
	}

	if (data->name) {
		window_text_set(data->win, "text_number", data->name);
	}
	else {
		window_text_set(data->win, "text_number", data->number);
	}
	common_utils_object_unref_free(data);
}


static void
retrieve_callback(GHashTable * properties, gpointer _data)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("retrieve_callback()");

	data->properties = properties;	// TODO: copy
	//data->query = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);

	phoneui_utils_contact_lookup(data->number, message_common_name_callback,
				common_utils_object_ref(data));

	struct Window *win = data->win;

	window_layout_set(win, DEFAULT_THEME, "phoneui/messages/show");


	char *content = elm_entry_utf8_to_markup(data->content);

	window_text_set(win, "text_status", data->status);
	window_text_set(win, "text_number", data->number);
	window_text_set(win, "text_date", data->date);
	window_text_set(win, "label_number", D_("From:"));
	window_text_set(win, "label_date", D_("Date:"));
	window_text_set(win, "label_status", D_("Status:"));

	data->lbl_content = elm_entry_add(data->win->win);
	elm_entry_editable_set(data->lbl_content, EINA_FALSE);
	elm_entry_entry_set(data->lbl_content, content);
	evas_object_size_hint_weight_set(data->lbl_content, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(data->lbl_content, EVAS_HINT_FILL,
			EVAS_HINT_FILL);

	data->sc_content = elm_scroller_add(data->win->win);
	elm_scroller_bounce_set(data->sc_content, EINA_FALSE, EINA_FALSE);
	//elm_scroller_policy_set(data->sc_content, ELM_SCROLLER_POLICY_OFF,
	//		ELM_SCROLLER_POLICY_ON);
	elm_scroller_content_set(data->sc_content, data->lbl_content);
	evas_object_show(data->lbl_content);
	window_swallow(data->win, "text_content", data->sc_content);
	evas_object_show(data->sc_content);


	data->bt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt1, D_("Close"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       message_show_view_close_clicked, data);
	window_swallow(win, "button_close", data->bt1);
	evas_object_show(data->bt1);


	// Options button with hover
	data->hv = elm_hover_add(window_evas_object_get(win));

	data->bt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt2, D_("Options"));
	evas_object_smart_callback_add(data->bt2, "clicked", my_hover_bt_1,
				       data->hv);
	window_swallow(win, "button_options", data->bt2);
	evas_object_show(data->bt2);

	elm_hover_parent_set(data->hv, window_evas_object_get(win));
	elm_hover_target_set(data->hv, data->bt2);

	data->bx = elm_box_add(window_evas_object_get(win));
	elm_box_horizontal_set(data->bx, 0);
	elm_box_homogenous_set(data->bx, 1);
	evas_object_show(data->bx);

	data->hbt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->hbt1, D_("Delete"));
	evas_object_size_hint_min_set(data->hbt1, 140, 80);
	evas_object_smart_callback_add(data->hbt1, "clicked",
				       message_show_view_delete_clicked, data);
	evas_object_show(data->hbt1);
	elm_box_pack_end(data->bx, data->hbt1);

	data->hbt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->hbt2, D_("Call"));
	evas_object_size_hint_min_set(data->hbt2, 140, 80);
	evas_object_smart_callback_add(data->hbt2, "clicked",
				       message_show_view_call_clicked, data);
	evas_object_show(data->hbt2);
	elm_box_pack_end(data->bx, data->hbt2);

	data->hbt3 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->hbt3, D_("Add Contact"));
	evas_object_size_hint_min_set(data->hbt3, 140, 80);
	evas_object_smart_callback_add(data->hbt3, "clicked",
				       message_show_view_new_contact_clicked,
				       data);
	evas_object_show(data->hbt3);
	elm_box_pack_end(data->bx, data->hbt3);

	elm_hover_content_set(data->hv, "top", data->bx);


	data->bt3 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt3, D_("Answer"));
	evas_object_smart_callback_add(data->bt3, "clicked",
				       message_show_view_answer_clicked, data);
	window_swallow(win, "button_answer", data->bt3);
	evas_object_show(data->bt3);


	window_show(win);
}

static void
message_show_view_new_contact_clicked(struct MessageShowViewData *data,
				      Evas_Object * obj, void *event_info)
{
	/*FIXME should free this hashtable*/
	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "Phone",
			common_utils_new_gvalue_string(data->number));

	phoneui_contacts_contact_new(options);
}

