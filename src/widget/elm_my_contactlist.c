#include "elm_my_contactlist.h"
#include <glib.h>
#include <glib-object.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <Etk.h>
#include "async.h"
#include "elm_config.h"

// TODO: Talk about it with raster
// got it from elm_priv.h
#define ELM_NEW(t) calloc(1, sizeof(t))

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
	Evas_Object *widget, *keypad;
	Etk_Widget *container, *tree;
	Etk_Tree_Col *col1;
	GPtrArray *messages;
	Evas_Object **widget_pointer;
};

DBusGProxy *GQuery = NULL;

static void 
_del_hook(Evas_Object *obj);
static void 
_sizing_eval(Evas_Object *obj);
static void 
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
//static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void 
_signal_clicked(void *data, Evas_Object *o, const char *emission, const char *source);
static void 
_retrieve_callback2(void *data);
static void 
_process_entry(gpointer _entry, gpointer data);




static void 
_del_hook(Evas_Object *obj)
{

	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(obj);
	*(wd->widget_pointer) = NULL;
	evas_object_del(wd->keypad);
	free(wd);
}

static void 
_sizing_eval(Evas_Object *obj)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(obj);
	Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

	edje_object_size_min_calc(wd->keypad, &minw, &minh);
	evas_object_size_hint_min_set(obj, minw, minh);
	evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void 
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	return; // TODO: Confirm this line

	//Widget_Data *wd = elm_widget_data_get(data);
	//if (obj != wd->icon) return;
	//edje_object_part_swallow(wd->widget, "elm.swallow.content", obj);
	//_sizing_eval(data);
}


/*
   static void _sub_del(void *data, Evas_Object *obj, void *event_info)
   {
   g_debug("SUB DEL CALLED!");
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (sub == wd->icon)
   {
   edje_object_signal_emit(wd->keypad, "elm,state,icon,hidden", "elm");
   evas_object_event_callback_del
   (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
   wd->icon = NULL;
   _sizing_eval(obj);
   }
   }
   */ 


static void 
_signal_clicked(void *data, Evas_Object *o, const char *emission, const char *source)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(data);
	evas_object_smart_callback_call(wd->widget, "clicked", emission[0]);
}


static gint 
_compare_entries(gconstpointer _a, gconstpointer _b)
{
	GHashTable **a = (GHashTable **)_a;
	GHashTable **b = (GHashTable **)_b;
	gpointer p; 
	const char *name_a, *name_b;

	p = g_hash_table_lookup(*a, "Name");
	if (!p) {
		name_a = "";
		g_debug("name a not found!!!!");
	}
	else
		name_a = g_value_get_string(p);

	p = g_hash_table_lookup(*b, "Name");
	if (!p) {
		name_b = "";
		g_debug("name b not found!!!!");
	}
	else
		name_b = g_value_get_string(p);

	return (strcasecmp(name_a, name_b));
}


static void 
_row_delete_callback(void *row_data)
{
	g_hash_table_destroy(row_data);
}


static void 
_process_entry(gpointer _entry, gpointer data)
{
	GHashTable *entry = (GHashTable *)_entry;
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(data);
	const char *number = g_value_get_string(g_hash_table_lookup(entry, "Phone"));
	if (number[0] == 't' && number[1] == 'e' && number[2] == 'l' && number[3] == ':') {
		g_debug("stripping !!!");
		number += 4;
	}
	g_debug("number is '%s'", number);

	GHashTable *parameters = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	g_hash_table_insert(parameters, "path", strdup(g_value_get_string(g_hash_table_lookup(entry, "Path"))));
	g_hash_table_insert(parameters, "name", strdup(g_value_get_string(g_hash_table_lookup(entry, "Name"))));
	g_hash_table_insert(parameters, "number", strdup(number));
	Etk_Tree_Row *row = etk_tree_row_append(ETK_TREE(wd->tree), NULL, wd->col1, parameters, NULL);

	etk_tree_row_data_set_full(row, parameters, _row_delete_callback);
}


static void 
_retrieve_callback(GError *error, GPtrArray *messages, void *data)
{
	Evas_Object **widget = (Evas_Object **) data;

	g_debug("_retrieve_callback");

	if (error != NULL || messages == NULL) {
		g_debug("dbus error !!!");
		return;
	}

	if (*widget == NULL)
		free(widget);
	else {
		Widget_Data *wd = (Widget_Data *)elm_widget_data_get(*widget);
		wd->messages = messages;
		async_trigger(_retrieve_callback2, data);
	}
}



static void 
_retrieve_callback2(void *data)
{
	Evas_Object **widget = (Evas_Object **) data;

	g_debug("_retrieve_callback2");

	if (*widget == NULL) {
		g_debug("no widget... freeing");
		free(widget);
	} 
	else {
		g_debug("feeding contacts");
		Widget_Data *wd = (Widget_Data *)elm_widget_data_get(*widget);
		GPtrArray *messages = wd->messages;

		g_ptr_array_sort(messages, _compare_entries);
		g_ptr_array_foreach(messages, _process_entry, *widget);
	}
	//if (GQuery) {
	//    g_debug("disposing the query proxy object");
	//    opimd_contact_query_dispose(GQuery, NULL, NULL);
	//    g_free(GQuery);
	//}
}


static void 
_result_callback(GError *error, int count, void *data)
{
	g_debug("_result_callback");
	if (error == NULL) {
		g_debug("result gave %d entries --> retrieving", count);
		opimd_contact_query_get_multiple_results(GQuery, count, _retrieve_callback, data);
	}
}



static void 
_query_callback(GError *error, char *query_path, void *data)
{
	g_debug("_query_callback");
	if (error == NULL) {
		g_debug("query path is %s", query_path);
		GQuery = dbus_connect_to_opimd_contact_query (query_path);
		opimd_contact_query_get_result_count (GQuery, _result_callback, data);
	}
}



Evas_Object *
elm_my_contactlist_add(Evas_Object *parent)
{
	// Evas_Object *obj; Instead I'm using the wd->widget variable
	Evas *e;
	Widget_Data *wd;

	wd = ELM_NEW(Widget_Data);
	e = evas_object_evas_get(parent);
	wd->widget = elm_widget_add(e);
	elm_widget_data_set(wd->widget, wd);

	wd->widget_pointer = malloc(sizeof(Evas_Object **));
	*(wd->widget_pointer) = wd->widget;
	elm_widget_del_hook_set(wd->widget, _del_hook);

	/* wd->keypad = edje_object_add(e);
	   g_debug("keypad evas object: %d", wd->keypad);
	   edje_object_file_set(wd->keypad, KEYPAD_FILE, "keypad");
	   edje_object_signal_callback_add(wd->keypad, "*", "input", _signal_clicked, wd->widget);*/


	wd->tree = etk_tree_new();
	etk_tree_rows_height_set(ETK_TREE(wd->tree), 80);
	etk_tree_mode_set(ETK_TREE(wd->tree), ETK_TREE_MODE_LIST);
	etk_tree_headers_visible_set(ETK_TREE(wd->tree), ETK_FALSE);
	etk_tree_multiple_select_set(ETK_TREE(wd->tree), ETK_FALSE);

	wd->col1 = etk_tree_col_new(ETK_TREE(wd->tree), "Title", 300, 0.0);
	etk_tree_col_model_add(wd->col1, etk_tree_model_edje_new(CONTACTS_FILE, "row"));
	etk_tree_build(ETK_TREE(wd->tree));

	Etk_Scrolled_View *scrolled_view = etk_tree_scrolled_view_get(ETK_TREE(wd->tree));
	etk_scrolled_view_dragable_set(ETK_SCROLLED_VIEW(scrolled_view), ETK_TRUE);
	etk_scrolled_view_drag_bouncy_set(ETK_SCROLLED_VIEW(scrolled_view), ETK_FALSE);
	etk_scrolled_view_policy_set(ETK_SCROLLED_VIEW(scrolled_view), ETK_POLICY_HIDE, ETK_POLICY_SHOW);
	etk_scrolled_view_drag_damping_set(ETK_SCROLLED_VIEW(scrolled_view), 800);

	wd->container = etk_embed_new(e);
	etk_container_add(ETK_CONTAINER(wd->container), wd->tree);
	etk_widget_show_all(wd->container);
	Evas_Object *object = etk_embed_object_get(ETK_EMBED(wd->container));

	GHashTable *qry = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	opimd_contacts_query(qry, _query_callback, wd->widget_pointer);
	//g_hash_table_destroy(qry);


	elm_widget_resize_object_set(wd->widget, object);
	//evas_object_smart_callback_add(wd->widget, "sub-object-del", _sub_del, wd->widget);

	_sizing_eval(wd->widget);

	return (wd->widget);
}



GHashTable *
elm_my_contactlist_selected_row_get(void *data)
{
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(data);
	Etk_Tree_Row *row = (Etk_Tree_Row *)etk_tree_selected_row_get((Etk_Tree *)wd->tree);
	if (row != NULL) {
		GHashTable *parameters = etk_tree_row_data_get(row);
		return (parameters);
	}
	return (NULL);
}



void 
elm_my_contactlist_refresh(void *data)
{
	g_debug("elm_my_contactlist_refresh");
	Widget_Data *wd = (Widget_Data *)elm_widget_data_get(data);
	etk_tree_clear((Etk_Tree *)wd->tree);
	g_debug("tree is cleared");
	GHashTable *qry = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	opimd_contacts_query(qry, _query_callback, wd->widget_pointer);
	g_debug("query triggered");
	//g_hash_table_destroy(qry);
}



