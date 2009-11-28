#include "views.h"
#include "common-utils.h"
#include <phoneui/phoneui-utils.h>
/* FIXME: HACKS FROM elm_priv.h that should be removed */
#if 1
void         elm_widget_scale_set(Evas_Object *obj, double scale);
#endif


struct MessageListViewData {
	struct Window *win;
	char *path;
	Evas_Object *list, *bt1, *bt2, *bt3, *hv, *bx, *button_answer,
		*button_delete;

	Elm_Genlist_Item *selected_row;

	GPtrArray *messages;
};


static Elm_Genlist_Item_Class itc;

static void
message_list_view_new_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
message_list_view_show_clicked(void *_data, Evas_Object * obj,
			       void *event_info);
static void
message_list_view_answer_clicked(void *_data, Evas_Object * obj,
				 void *event_info);
static void
message_list_view_delete_clicked(void *_data, Evas_Object * obj,
				 void *event_info);
static void
  my_hover_bt_1(void *_data, Evas_Object * obj, void *event_info);

static void
retrieve_messagebook_callback(GError * error, GPtrArray * messages,
			      void *_data);
static void
  process_message(gpointer _message, gpointer _data);

static
gint compare_messages(gconstpointer _a, gconstpointer _b);
static void
  message_list_view_message_deleted(void *_data);


/* --- message list view ---------------------------------------------------- */

static char *
gl_label_get(const void *data, Evas_Object * obj, const char *part)
{
	GHashTable *parameters = (GHashTable *) data;
	char *label = NULL;

	//g_debug("getting label for %s", part);
	if (!strcmp(part, "elm.text")) {
		label = g_hash_table_lookup(parameters, "name");
		if (!label) {
			label = g_hash_table_lookup(parameters, "number");
		}
		return (g_strdup_printf
			("%s %s", g_hash_table_lookup(parameters, "date"),
			 label));
	}
	else if (!strcmp(part, "elm.text.sub"))
		return (g_strdup(g_hash_table_lookup(parameters, "content")));

	return (NULL);
}

static Evas_Object *
gl_icon_get(const void *data, Evas_Object * obj, const char *part)
{
	return (NULL);
}


static Eina_Bool
gl_state_get(const void *data, Evas_Object * obj, const char *part)
{
	return (EINA_FALSE);
}

static void
gl_del(const void *data, Evas_Object * obj)
{
}

void *
message_list_view_show(struct Window *win, void *_options)
{
	g_debug("message_list_view_show()");

	struct MessageListViewData *data =
		common_utils_object_ref(calloc(1, sizeof(struct MessageListViewData)));
	data->win = win;

	window_layout_set(win, DEFAULT_THEME, "phoneui/messages/list");
	window_text_set(win, "title", D_("Inbox"));

	data->bt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt1, D_("New"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       message_list_view_new_clicked, data);
	window_swallow(win, "button_new", data->bt1);
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

	data->button_answer = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->button_answer, D_("Answer"));
	evas_object_size_hint_min_set(data->button_answer, 130, 80);
	evas_object_smart_callback_add(data->button_answer, "clicked",
				       message_list_view_answer_clicked, data);
	evas_object_show(data->button_answer);
	elm_box_pack_end(data->bx, data->button_answer);

	data->button_delete = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->button_delete, D_("Delete"));
	evas_object_size_hint_min_set(data->button_delete, 130, 80);
	evas_object_smart_callback_add(data->button_delete, "clicked",
				       message_list_view_delete_clicked, data);
	evas_object_show(data->button_delete);
	elm_box_pack_end(data->bx, data->button_delete);

	elm_hover_content_set(data->hv, "top", data->bx);


	data->bt3 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt3, D_("Show"));
	evas_object_smart_callback_add(data->bt3, "clicked",
				       message_list_view_show_clicked, data);
	window_swallow(win, "button_show", data->bt3);
	evas_object_show(data->bt3);

	elm_theme_extension_add(DEFAULT_THEME);

	data->list = elm_genlist_add(window_evas_object_get(data->win));
	//elm_genlist_horizontal_mode_set(data->list, ELM_LIST_LIMIT);
	elm_widget_scale_set(data->list, 1.0);
	window_swallow(data->win, "list", data->list);
	//itc.item_style     = "double_label";
	itc.item_style = "message";
	itc.func.label_get = gl_label_get;
	itc.func.icon_get = gl_icon_get;
	itc.func.state_get = gl_state_get;
	itc.func.del = gl_del;
	//elm_scroller_policy_set(data->list, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);
	//evas_object_size_hint_align_set(data->list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	//evas_object_size_hint_weight_set(data->list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(data->list);

	phoneui_utils_messages_get(retrieve_messagebook_callback, data);

	window_show(win);

	return data;
}

void
message_list_view_hide(void *_data)
{
	struct MessageListViewData *data = (struct MessageListViewData *) _data;
	struct Window *win = data->win;

	g_debug("hiding the message list");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->bt3);

	evas_object_del(data->button_answer);
	evas_object_del(data->button_delete);
	evas_object_del(data->bx);
	evas_object_del(data->hv);
	data->win = NULL;
	
	if (common_utils_object_get_ref(data) == 1) {
		evas_object_del(data->list);
	}
	common_utils_object_unref_free(data);
}



/* --- evas callbacks ------------------------------------------------------- */

static void
message_list_view_new_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	g_debug("message_list_view_new_clicked()");

	struct Window *win = window_new(D_("Compose SMS"));
	window_init(win);
	window_view_show(win, NULL, message_new_view_show,
			 message_new_view_hide, NULL);
}

static void
message_list_view_show_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageListViewData *data = (struct MessageListViewData *) _data;

	g_debug("message_list_view_show_clicked()");

	data->selected_row = elm_genlist_selected_item_get(data->list);
	if (data->selected_row != NULL) {
		GHashTable *parameters = (GHashTable *)
			elm_genlist_item_data_get(data->selected_row);

		GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(options, "number",
				    g_hash_table_lookup(parameters, "number"));
		g_hash_table_insert(options, "content",
				    g_hash_table_lookup(parameters, "content"));
		g_hash_table_insert(options, "direction",
				    g_hash_table_lookup(parameters,
							"direction"));
		g_hash_table_insert(options, "status",
				    g_hash_table_lookup(parameters, "status"));
		g_hash_table_insert(options, "date",
				    g_hash_table_lookup(parameters, "date"));
		g_hash_table_insert(options, "path",
				    g_hash_table_lookup(parameters, "path"));

		g_hash_table_insert(options, "delete_callback",
				    message_list_view_message_deleted);
		g_hash_table_insert(options, "delete_callback_data", data);


		struct Window *win = window_new(D_("Show Message"));
		window_init(win);
		window_view_show(win, options, message_show_view_show,
				 message_show_view_hide, NULL);
	}
}

static void
message_list_view_answer_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	struct MessageListViewData *data = (struct MessageListViewData *) _data;

	g_debug("message_list_view_answer_clicked()");

	evas_object_hide(data->hv);

	data->selected_row = elm_genlist_selected_item_get(data->list);
	if (data->selected_row != NULL) {
		GHashTable *parameters = (GHashTable *)
			elm_genlist_item_data_get(data->selected_row);

		GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(options, "name",
				    g_hash_table_lookup(parameters, "name"));
		g_hash_table_insert(options, "number",
				    g_hash_table_lookup(parameters, "number"));

		struct Window *win = window_new(D_("SMS Answer"));
		window_init(win);
		window_view_show(win, options, message_new_view_show,
				 message_new_view_hide, NULL);
	}
}

static void
message_list_view_delete_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	struct MessageListViewData *data = (struct MessageListViewData *) _data;

	g_debug("message_list_view_delete_clicked()");

	evas_object_hide(data->hv);

	data->selected_row = elm_genlist_selected_item_get(data->list);
	if (data->selected_row != NULL) {
		g_debug("found a selected row to delete...");
		GHashTable *parameters = (GHashTable *)
			elm_genlist_item_data_get(data->selected_row);

		g_debug("filling options...");
		GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(options, "path",
				    g_hash_table_lookup(parameters, "path"));
		g_hash_table_insert(options, "delete_callback",
				    message_list_view_message_deleted);
		g_hash_table_insert(options, "delete_callback_data", data);

		g_debug("calling confirmation window...");
		struct Window *win = window_new(D_("Delete Message"));
		window_init(win);
		window_view_show(win, options, message_delete_view_show,
				 message_delete_view_hide, NULL);
	}
}

static void
my_hover_bt_1(void *_data, Evas_Object * obj, void *event_info)
{
	Evas_Object *hv = (Evas_Object *) _data;
	evas_object_show(hv);
}



/* callbacks */

static void
retrieve_messagebook_callback(GError * error, GPtrArray * messages, void *_data)
{
	struct MessageListViewData *data = (struct MessageListViewData *) _data;

	g_debug("retrieve messagebook callback(error=%d)", error);

	data->messages = messages;
	//g_ptr_array_foreach(data->messages, add_integer_timestamp_to_message, NULL);
	//g_ptr_array_sort(data->messages, compare_messages);

	g_ptr_array_foreach(data->messages, process_message, data);
}

void
_remove_tel(char *number)
{
	if (!strncmp("tel:", number, 4)) {
		char *tmp = strdup(number);
		strcpy(number, &tmp[4]);
		free(tmp);
	}
}

struct _contact_lookup_pack {
	struct MessageListViewData *data;
	Elm_Genlist_Item *param;
};

static void
_contact_lookup(GHashTable *contact, gpointer _pack)
{
	struct _contact_lookup_pack *pack = _pack;
	int count;

	count = common_utils_object_get_ref(pack->param);
	if (!contact || count < 1 || !pack->data->win) {
		if (common_utils_object_get_ref(pack->data) == 1) {
			evas_object_del(pack->data->list);
		}
		goto end;
	}

	GValue *gval_tmp = g_hash_table_lookup(contact, "_Name");
	if (gval_tmp) {
		GHashTable *parameters = elm_genlist_item_data_get(pack->param);
		g_hash_table_insert(parameters, "name",
				strdup(g_value_get_string(gval_tmp)));
		elm_genlist_item_update(pack->param);
	}
	if (common_utils_object_get_ref(pack->data) == 1) {
		evas_object_del(pack->data->list);
	}
end:
	common_utils_object_unref(pack->param);
	common_utils_object_unref_free(pack->data);
	free(pack);
}


static void
process_message(gpointer _entry, gpointer _data)
{
	GHashTable *entry = (GHashTable *) _entry;
	GValue *gval_tmp;
	struct MessageListViewData *data = (struct MessageListViewData *) _data;
	char *number = NULL;

	long timestamp;
	gval_tmp = g_hash_table_lookup(entry, "Timestamp");
	if (gval_tmp) {
		timestamp = (long) g_value_get_int(gval_tmp);
	}
	else {
		timestamp = 0;
	}


	char datestr[35];

	g_debug("processing entry");
	strftime(datestr, 31, "%d.%m.%Y %H:%M" LTR_STRING, localtime(&timestamp)); 

	char *tmp;



	GHashTable *parameters =
		g_hash_table_new_full(g_str_hash, g_str_equal, NULL, free);

	gval_tmp = g_hash_table_lookup(entry, "Direction");
	if (gval_tmp) {
		tmp = strdup(g_value_get_string(gval_tmp));
	}
	else {
		tmp = strdup("");
	}
	g_hash_table_insert(parameters, "direction", tmp);


	if (!strncmp(tmp, "in", 2)) {
		gval_tmp = g_hash_table_lookup(entry, "Sender");
	}
	else {
		gval_tmp = g_hash_table_lookup(entry, "Recipient");
	}
	if (gval_tmp) {
		tmp = strdup(g_value_get_string(gval_tmp));
		number = tmp;
		_remove_tel(tmp);
	}
	else {
		tmp = strdup("Missing sender");
	}
	g_hash_table_insert(parameters, "number", tmp);

	gval_tmp = g_hash_table_lookup(entry, "Content");
	if (gval_tmp) {
		tmp = strdup(g_value_get_string(gval_tmp));
	}
	else {
		tmp = strdup("Missing content");
	}
	g_hash_table_insert(parameters, "content", tmp);

	gval_tmp = g_hash_table_lookup(entry, "MessageRead");
	if (gval_tmp && g_value_get_boolean(gval_tmp)) {
		g_hash_table_insert(parameters, "status", strdup("Read"));
	}
	else {
		g_hash_table_insert(parameters, "status", strdup("Unread"));
	}
	g_hash_table_insert(parameters, "date", strdup(datestr));
	g_hash_table_insert(parameters, "path",
			    strdup(g_value_get_string
				   (g_hash_table_lookup(entry, "Path"))));

	Elm_Genlist_Item *it = elm_genlist_item_append(data->list, &itc, parameters, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
	if (number) {
		struct _contact_lookup_pack *pack;
		pack = malloc(sizeof(*pack));
		pack->data = common_utils_object_ref(data);
		pack->param = common_utils_object_ref(it);
		phoneui_utils_contact_lookup(number, _contact_lookup, pack);
	}
}






/* --- helper functions ----------------------------------------------------- */

static gint
compare_messages(gconstpointer _a, gconstpointer _b)
{
	GValueArray **a = (GValueArray **) _a;
	GValueArray **b = (GValueArray **) _b;
	GHashTable *h1 = g_value_get_boxed(g_value_array_get_nth(*a, 4));
	GHashTable *h2 = g_value_get_boxed(g_value_array_get_nth(*b, 4));

	long la = g_value_get_long(g_hash_table_lookup(h1, "timestamp_int"));
	long lb = g_value_get_long(g_hash_table_lookup(h2, "timestamp_int"));

	if (la > lb)
		return -1;
	else if (la < lb)
		return 1;
	else
		return 0;
}

static void
message_list_view_message_deleted(void *_data)
{
	struct MessageListViewData *data = (struct MessageListViewData *)_data;
	int count;
	count = common_utils_object_unref(data->selected_row);
	if (data->selected_row != NULL) {
		elm_genlist_item_del(data->selected_row);
		data->selected_row = NULL;
	}
}

