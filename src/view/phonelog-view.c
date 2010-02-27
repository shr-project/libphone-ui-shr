#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>

#include "views.h"
#include "util/ui-utils.h"
#include "util/common-utils.h"

struct PhonelogViewData {
	struct View parent;
	Evas_Object *toolbar, *pager;
	Evas_Object *list_in, *list_out, *list_missed, *list_all;
	int count;
	GPtrArray *calls;
};

static struct PhonelogViewData view;
static Elm_Genlist_Item_Class itc;

static void _phonelog_destroy_cb(struct View *_view);
static void _toolbar_changed(void *data, Evas_Object *obj, void *event_info);
static Evas_Object *_add_genlist(Evas_Object *win);
static void _add_entry(GHashTable *entry);
static void _contact_lookup(GHashTable *contact, GHashTable *entry);
static void _get_callback(GHashTable *entry, gpointer data);

static char *
gl_label_get(const void *data, Evas_Object * obj, const char *part);
static Evas_Object *
gl_icon_get(const void *data, Evas_Object * obj, const char *part);
static Eina_Bool
gl_state_get(const void *data, Evas_Object * obj, const char *part);
static void
gl_del(const void *data, Evas_Object * obj);

void phonelog_view_show()
{
	ui_utils_view_show(&view.parent);
}

void phonelog_view_hide()
{
	ui_utils_view_hide(&view.parent);
}

int phonelog_view_init()
{
	g_debug("Initializing the phonelog screen");
	Evas_Object *win, *icon;
	Elm_Toolbar_Item *item;
	int ret;
	//char buf[PATH_MAX];

	ret = ui_utils_view_init(&view.parent, ELM_WIN_BASIC, D_("Phonelog"),
			NULL, NULL, _phonelog_destroy_cb);

	if (ret) {
		g_critical("Failed to init phonelog view");
		return ret;
	}

	win = ui_utils_view_window_get(&view.parent);

	ui_utils_view_layout_set(&view.parent, DEFAULT_THEME, "phoneui/phonelog/phonelog");
	elm_theme_extension_add(DEFAULT_THEME);

	view.pager = elm_pager_add(win);
	ui_utils_view_swallow(&view.parent, "pager", view.pager);
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
	itc.func.label_get = gl_label_get;
	itc.func.icon_get = gl_icon_get;
	itc.func.state_get = gl_state_get;
	itc.func.del = gl_del;

	view.toolbar = elm_toolbar_add(win);
	ui_utils_view_swallow(&view.parent, "toolbar", view.toolbar);
	elm_toolbar_homogenous_set(view.toolbar, 1);
	elm_toolbar_scrollable_set(view.toolbar, EINA_FALSE);
	elm_toolbar_align_set(view.toolbar, 0.0);
	elm_toolbar_icon_size_set(view.toolbar, 16);
	evas_object_size_hint_weight_set(view.toolbar, 0.0, 0.0);
	evas_object_size_hint_align_set(view.toolbar, EVAS_HINT_FILL, 0.0);

	icon = elm_icon_add(win);
	elm_icon_file_set(icon, DEFAULT_THEME, "icon/phonelog-incoming");
	elm_toolbar_item_add(view.toolbar, icon, D_("received"), _toolbar_changed, view.list_in);
	evas_object_show(icon);

	icon = elm_icon_add(win);
	elm_icon_file_set(icon, DEFAULT_THEME, "icon/phonelog-outgoing");
	elm_toolbar_item_add(view.toolbar, icon, D_("outgoing"), _toolbar_changed, view.list_out);
	evas_object_show(icon);

	icon = elm_icon_add(win);
	elm_icon_file_set(icon, DEFAULT_THEME, "icon/phonelog-missed");
	elm_toolbar_item_add(view.toolbar, icon, D_("missed"), _toolbar_changed, view.list_missed);
	evas_object_show(icon);

	icon = elm_icon_add(win);
	elm_icon_file_set(icon, DEFAULT_THEME, "icon/phonelog-all");
	elm_toolbar_item_add(view.toolbar, icon, D_("all"), _toolbar_changed, view.list_all);
	evas_object_show(icon);

	evas_object_show(view.toolbar);

	view.calls = g_ptr_array_new();

	g_debug("querying calls...");
	phoneui_utils_calls_get(&view.count, _get_callback, NULL);

	return 0;
}

void phonelog_view_deinit()
{
	ui_utils_view_deinit(&view.parent);

	evas_object_del(view.list_in);
	evas_object_del(view.list_out);
	evas_object_del(view.list_missed);
	evas_object_del(view.list_all);
	evas_object_del(view.pager);
	evas_object_del(view.toolbar);
}

int phonelog_view_is_init()
{
	return ui_utils_view_is_init(&view.parent);
}

void phonelog_view_new_call(char *path)
{
	g_debug("New call: %s", path);
	phoneui_utils_call_get(path, _get_callback, NULL);
}


static void
_phonelog_destroy_cb(struct View *_view)
{
	struct PhonelogViewData *view = (struct PhonelogViewData *)_view;
	phonelog_view_hide();
}

static void _toolbar_changed(void *data, Evas_Object *obj, void *event_info)
{
	g_debug("promoting %d to top", data);
	elm_pager_content_promote(view.pager, data);
}

static Evas_Object *
_add_genlist(Evas_Object *win)
{
	Evas_Object *list = elm_genlist_add(win);
	elm_genlist_horizontal_mode_set(list, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(list, 0.0, 0.0);
	evas_object_show(list);

	return list;
}

static void
_add_entry(GHashTable *entry)
{
	Elm_Genlist_Item *it;
	GValue *val;
	int received = 0, answered = 0;

	val = g_hash_table_lookup(entry, "Direction");
	if (val) {
		char *dir = g_value_get_string(val);
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
		if (g_value_get_boolean(val)) {
			answered = 1;
		}
	}

	it = elm_genlist_item_append(view.list_all, &itc,
			g_hash_table_ref(entry),
			NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	g_hash_table_insert(entry, "_item_all",
			common_utils_new_gvalue_pointer(it));
	if (received) {
		if (answered) {
			it = elm_genlist_item_append(view.list_in, &itc,
					g_hash_table_ref(entry),
					NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			g_hash_table_insert(entry, "_item_in",
					common_utils_new_gvalue_pointer(it));
		}
		else {
			it = elm_genlist_item_append(view.list_missed, &itc,
					g_hash_table_ref(entry),
					NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			g_hash_table_insert(entry, "_item_missed",
					common_utils_new_gvalue_pointer(it));
		}
	}
	else {
		it = elm_genlist_item_append(view.list_out, &itc,
				g_hash_table_ref(entry),
				NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		g_hash_table_insert(entry, "_item_out",
				common_utils_new_gvalue_pointer(it));
	}
}

static void
_update_entry(GHashTable *entry)
{
	Elm_Genlist_Item *it;
	GValue *val;

	val = g_hash_table_lookup(entry, "_item_all");
	if (val) {
		it = (Elm_Genlist_Item *)g_value_get_pointer(val);
		elm_genlist_item_update(it);
	}
	val = g_hash_table_lookup(entry, "_item_missed");
	if (val) {
		it = (Elm_Genlist_Item *)g_value_get_pointer(val);
		elm_genlist_item_update(it);
	}
	val = g_hash_table_lookup(entry, "_item_in");
	if (val) {
		it = (Elm_Genlist_Item *)g_value_get_pointer(val);
		elm_genlist_item_update(it);
	}
	val = g_hash_table_lookup(entry, "_item_out");
	if (val) {
		it = (Elm_Genlist_Item *)g_value_get_pointer(val);
		elm_genlist_item_update(it);
	}
}

static void
_contact_lookup(GHashTable *contact, GHashTable *entry)
{
	if (contact) {
		char *s = phoneui_utils_contact_display_name_get(contact);
		g_hash_table_insert(entry, "Name",
				common_utils_new_gvalue_string(s));
		free(s);
	}
	else {
		g_hash_table_insert(entry, "Name",
				common_utils_new_gvalue_string(
					CONTACT_NAME_UNDEFINED_STRING));
	}
	_update_entry(entry);
}

static void
_get_callback(GHashTable *entry, gpointer data)
{
	GValue *val;
	Elm_Genlist_Item *it;

	g_ptr_array_add(view.calls, entry);

	val = g_hash_table_lookup(entry, "Peer");
	if (val) {
		_add_entry(entry);
		phoneui_utils_contact_lookup(
				g_value_get_string(val),
				_contact_lookup, entry);
	}
	else {
		g_message("ignoring call without Peer attribute");
	}
}

/* --- genlist callbacks --- */
static char *
gl_label_get(const void *data, Evas_Object * obj, const char *part)
{
	GHashTable *entry = (GHashTable *) data;
	char *s = NULL;
	GValue *val;

	g_debug("gl_label_get: %s", part);
	if (!strcmp(part, "elm.text")) {
		val = g_hash_table_lookup(entry, "Name");
		if (val) {
			return g_value_dup_string(val);
		}
		return strdup("");
	}

	if (!strcmp(part, "elm.text.sub")) {
		val = g_hash_table_lookup(entry, "Peer");
		if (val) {
			return g_value_dup_string(val);
		}
		return strdup(CONTACT_PHONE_UNDEFINED_STRING);
	}

	if (!strcmp(part, "elm.text.2")) {
		val = g_hash_table_lookup(entry, "Timestamp");
		if (val) {
			int timestamp = g_value_get_int(val);
			char datestr[35];
			strftime(datestr, 31, "%d.%m.%Y %H:%M" LTR_STRING, localtime(&timestamp));
			return strdup(datestr);
		}
	}

	if (!strcmp(part, "elm.text.sub.2")) {
		val = g_hash_table_lookup(entry, "Duration");
		if (val) {
			int duration = g_value_get_int(val);
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
gl_icon_get(const void *data, Evas_Object * obj, const char *part)
{
	//GHashTable *entry = (GHashTable *) data;
	//if (!strcmp(part, "elm.swallow.icon")) {
	//	const char *icon;
	//	GValue *tmp = g_hash_table_lookup(entry, "Direction");
	//	if (tmp) {
	//		if (!strcmp("in", g_value_get_string(tmp))) {
	//			tmp = g_hash_table_lookup(entry, "Answered");
	//			if (tmp && g_value_get_int(tmp)) {
	//				icon = "icon/phonelog-incoming";
	//			}
	//			else {
	//				icon = "icon/phonelog-missed";
	//			}
	//		}
	//		else {
	//			icon = "icon/phonelog-outgoing";
	//		}
	//	}
	//	Evas_Object *ico = elm_icon_add(obj);
	//	elm_icon_file_set(ico, DEFAULT_THEME, icon);
	//	elm_icon_no_scale_set(ico, EINA_TRUE);
	//	//evas_object_size_hint_aspect_set(ico,
	//	//				 EVAS_ASPECT_CONTROL_VERTICAL,
	//	//				 1, 1);
	//	return (ico);
	//}
	if (!strcmp(part,"elm.swallow.end")) {
		Evas_Object *btn = elm_button_add(obj);
		elm_button_label_set(btn, "Action");
		return (btn);
	}
	return (NULL);
}

static Eina_Bool
gl_state_get(const void *data, Evas_Object * obj, const char *part)
{
	GHashTable *entry = (GHashTable *) data;
	GValue *tmp = g_hash_table_lookup(entry, "Direction");
	if (tmp && g_value_get_int(tmp))
		return (EINA_TRUE);
	return (EINA_FALSE);
}

static void
gl_del(const void *data, Evas_Object * obj)
{
	if (data)
		g_hash_table_destroy((GHashTable *)data);
}

