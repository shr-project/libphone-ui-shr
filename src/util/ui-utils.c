
#include <Elementary.h>
#include <Evas.h>

#include "ui-utils.h"
#include "views.h"

/* FIXME: hackish, remove */
#include "phoneui-shr.h"

/* already defined in window.h - reactivate
when that beast got finally removed */
// struct InwinButton {
// 	char *label;
// 	void (*callback) (void *, Evas_Object *, void *);
// };


struct View *
ui_utils_view_new(const char *title)
{
	struct View *view;

	view = calloc(1, sizeof(struct View));
	if (!view) {
		g_critical("Failed to allocate memory (%s:%d)", __FUNCTION__, __LINE__);
		return NULL;
	}

	return view;
}

int
ui_utils_view_init(struct View *view, Elm_Win_Type type, const char *title,
			void (*show_cb) (struct View *view),
		 	void (*hide_cb) (struct View *view),
		 	void (*destroy_cb)(struct View *view))
{
	int ret = 0;
	g_debug("Initializing window with title: %s", title);

	if (!view) {
		g_critical("struct View is NULL (%s:%d)", __FUNCTION__, __LINE__);
		ret = 1;
		goto end;
	}

	/* Window */
	view->win = elm_win_add(NULL, "main", type);
	if (!view->win) {
		g_critical("Wasn't able to create a window (%s:%d)", __FUNCTION__, __LINE__);
		ret = 1;
		goto end;
	}

	elm_win_title_set(view->win, title);

	elm_win_autodel_set(view->win, 0);
	if (phoneui_theme) {
		elm_theme_overlay_add(phoneui_theme);
	}


	/* Background */
	view->background = elm_bg_add(view->win);
	if (!view->background) {
		g_critical("elm_bg_add failed (%s:%d)", __FUNCTION__, __LINE__);
		ret = 1;
		goto free_win;
	}
	evas_object_size_hint_weight_set(view->background, 1.0, 1.0);
	elm_win_resize_object_add(view->win, view->background);
	evas_object_show(view->background);

	/* Layout */
	view->layout = elm_layout_add(view->win);
	if (!view->layout) {
		g_critical("elm_layout_add failed (%s:%d)", __FUNCTION__, __LINE__);
		ret = 1;
		goto free_bg;
	}
	elm_win_resize_object_add(view->win, view->layout);
	evas_object_show(view->layout);

	// TODO: use fullscreen ?
	evas_object_resize(view->win, 480, 600);
	view->show_cb = show_cb;
	view->hide_cb = hide_cb;
	view->destroy_cb = destroy_cb;
	/* END of wtf */
end:
	return ret;

/* Error handling */
free_bg:
	evas_object_del(view->background);
free_win:
	evas_object_del(view->win);

	goto end;
}

int
ui_utils_view_is_visible(struct View *view)
{
	return evas_object_visible_get(view->win);
}

void
ui_utils_view_show(struct View *view)
{
	if (view->win) {
		if (view->show_cb) {
			view->show_cb(view);
		}
		evas_object_show(view->win);
		elm_win_activate(view->win);
	}
	else {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
	}
}

void
ui_utils_view_hide(struct View *view)
{
	if (!view->win) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	if (view->hide_cb) {
		view->hide_cb(view);
	}
	evas_object_hide(view->win);
}

void
ui_utils_view_toggle(struct View *view)
{
	if (ui_utils_view_is_visible(view)) {
		ui_utils_view_hide(view);
	}
	else {
		ui_utils_view_show(view);
	}
}

void
ui_utils_view_layout_set(struct View *view, const char *file, const char *part)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	g_debug("setting layout from file '%s' (%s)", file, part);
	elm_layout_file_set(view->layout, file, part);
}

Evas_Object *
ui_utils_view_layout_get(struct View *view)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	return elm_layout_edje_get(view->layout);
}

Evas_Object *
ui_utils_view_window_get(struct View *view)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	return view->win;
}

void
ui_utils_view_delete_callback_set(struct View *view,
			   void (*cb) (struct View *viewdow, Evas_Object *view,
				       void *event_info))
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	if (!cb) {
		g_critical("Tried to set cb to NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	evas_object_smart_callback_add(view->win, "delete,request", (Evas_Smart_Cb) cb, view);
}

void
ui_utils_view_text_set(struct View *view, const char *key, const char *value)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	edje_object_part_text_set(elm_layout_edje_get(view->layout), key, value);
}

void
ui_utils_view_swallow(struct View *view, const char *key, Evas_Object * object)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	/* What's this: ?
	 * edje_object_part_swallow(elm_layout_edje_get(view->layout), key, object);
	 */
	elm_layout_content_set(view->layout, key, object);
}

void
ui_utils_view_unswallow(struct View *view, Evas_Object * object)
{
	if (!view) {
		g_critical("Window is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}
	edje_object_part_unswallow(elm_layout_edje_get(view->layout), object);
}

void
ui_utils_view_deinit(struct View *view)
{
	if (!view) {
		g_critical("struct View is NULL (%s:%d)", __FUNCTION__, __LINE__);
		return;
	}

	g_debug("Calling view destroy callback");
	if (view->destroy_cb)
		view->destroy_cb(view);
	g_debug("View destroy callback DONE");

	ui_utils_view_hide(view);
 	evas_object_del(view->layout);
	evas_object_del(view->background);
	evas_object_del(view->win);

	/* This is enough to mark it's empty. */
	view->win = NULL;
	view->show_cb = NULL;
	view->hide_cb = NULL;
	view->destroy_cb = NULL;
	g_debug("Deinit of View done");
}

/* Think about this a bit more... Looks sane */
Evas_Object *
ui_utils_view_inwin_dialog(struct View *view, const char *label, GList *buttons,
		    		void *data)
{
	Evas_Object *inwin = elm_win_inwin_add(ui_utils_view_window_get(view));

	Evas_Object *bx = elm_box_add(ui_utils_view_window_get(view));
	elm_box_homogenous_set(bx, 1);

	Evas_Object *e = elm_label_add(ui_utils_view_window_get(view));
	elm_label_label_set(e, label);
	evas_object_show(e);
	elm_box_pack_end(bx, e);

	Evas_Object *bx2 = elm_box_add(ui_utils_view_window_get(view));
	elm_box_horizontal_set(bx2, 1);
	elm_box_homogenous_set(bx2, 1);

	for (buttons = g_list_first(buttons); buttons;
	     buttons = g_list_next(buttons)) {
		e = elm_button_add(ui_utils_view_window_get(view));
		elm_button_label_set(e,
				     ((struct InwinButton *) buttons->data)->
				     label);
		evas_object_smart_callback_add(e, "clicked",
					       ((struct InwinButton *)
						buttons->data)->callback, data);
		evas_object_show(e);
		elm_box_pack_end(bx2, e);
		g_free(buttons->data);
	}

	g_list_free(buttons);

	evas_object_show(bx2);
	elm_box_pack_end(bx, bx2);
	evas_object_show(bx);

	elm_win_inwin_content_set(inwin, bx);

	evas_object_show(inwin);

	return (inwin);
}

struct _dialog_pack {
	void (*callback)(int, void *);
	void *data;
	Evas_Object *inwin;
};

static void
_inwin_dialog_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct _dialog_pack *pack = (struct _dialog_pack *)data;
	if (pack->callback)
		pack->callback(DIALOG_CANCEL, pack->data);
	evas_object_del(pack->inwin);
}

static void
_inwin_dialog_no_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct _dialog_pack *pack = (struct _dialog_pack *)data;
	if (pack->callback)
		pack->callback(DIALOG_NO, pack->data);
	evas_object_del(pack->inwin);
}

static void
_inwin_dialog_yes_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct _dialog_pack *pack = (struct _dialog_pack *)data;
	if (pack->callback)
		pack->callback(DIALOG_YES, pack->data);
	evas_object_del(pack->inwin);
}

static void
_inwin_dialog_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct _dialog_pack *pack = (struct _dialog_pack *)data;
	if (pack->callback)
		pack->callback(DIALOG_OK, pack->data);
	evas_object_del(pack->inwin);
}

void
ui_utils_dialog(struct View *view, const char *label, int buttonflags,
		      void (*callback)(int, void *), void *data)
{
	Evas_Object *win, *box, *box2, *lbl, *btn;
	struct _dialog_pack *pack = malloc(sizeof(struct _dialog_pack));
	win = ui_utils_view_window_get(view);
	pack->callback = callback;
	pack->data = data;
	pack->inwin = elm_win_inwin_add(win);
	box = elm_box_add(win);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	lbl = elm_label_add(win);
	elm_label_label_set(lbl, label);
	evas_object_size_hint_align_set(lbl, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(lbl);
	elm_box_pack_end(box, lbl);

	box2 = elm_box_add(win);
	elm_box_horizontal_set(box2, EINA_TRUE);
	elm_box_homogenous_set(box2, EINA_TRUE);
	evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, 0);

	/* ok - yes - no - cancel */
	if (buttonflags & DIALOG_OK) {
		btn = elm_button_add(win);
		elm_button_label_set(btn, D_("Ok"));
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
		evas_object_smart_callback_add(btn, "clicked",
					_inwin_dialog_ok_cb, pack);
		evas_object_show(btn);
		elm_box_pack_end(box2, btn);
	}
	if (buttonflags & DIALOG_YES) {
		btn = elm_button_add(win);
		elm_button_label_set(btn, D_("Yes"));
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
		evas_object_smart_callback_add(btn, "clicked",
					_inwin_dialog_yes_cb, pack);
		evas_object_show(btn);
		elm_box_pack_end(box2, btn);
	}
	if (buttonflags & DIALOG_NO) {
		btn = elm_button_add(win);
		elm_button_label_set(btn, D_("No"));
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
		evas_object_smart_callback_add(btn, "clicked",
					_inwin_dialog_no_cb, pack);
		evas_object_show(btn);
		elm_box_pack_end(box2, btn);
	}
	if (buttonflags & DIALOG_CANCEL) {
		btn = elm_button_add(win);
		elm_button_label_set(btn, D_("Cancel"));
		evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
		evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
		evas_object_smart_callback_add(btn, "clicked",
					_inwin_dialog_cancel_cb, pack);
		evas_object_show(btn);
		elm_box_pack_end(box2, btn);
	}
	evas_object_show(box2);
	elm_box_pack_end(box, box2);

	elm_win_resize_object_add(win, box);
 	evas_object_show(box);
	elm_win_inwin_content_set(pack->inwin, box);

	elm_win_inwin_activate(pack->inwin);
}


struct _inwin_list_pack {
	void (*callback)(const char *, void *);
	void *data;
	Evas_Object *inwin;
	Evas_Object *list;
};

static gboolean
_inwin_list_destruct(gpointer data)
{
	struct _inwin_list_pack *pack = (struct _inwin_list_pack *)data;
	evas_object_del(pack->inwin);
	free (pack);
	return FALSE;
}

static void
_inwin_list_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *sel = NULL;
	struct _inwin_list_pack *pack = (struct _inwin_list_pack *)data;
	g_debug("Get the selected one");
	Elm_List_Item *it = elm_list_selected_item_get(obj);
	g_debug("Got item [%X]", it);
	if (it) {
		sel = strdup(elm_list_item_label_get(it));
		g_debug("Which is '%s'", sel);
	}
	if (pack->callback) {
		pack->callback(sel, pack->data);
	}
	g_timeout_add(0, _inwin_list_destruct, pack);
}

static void
_inwin_list_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct _inwin_list_pack *pack = (struct _inwin_list_pack *)data;
	g_debug("Cancelled selection");
	if (pack->callback) {
		pack->callback(NULL, pack->data);
	}
	g_timeout_add(0, _inwin_list_destruct, pack);
}

Evas_Object *
ui_utils_view_inwin_list(struct View *view, GList *list,
	void (*callback) (const char *, void *), void *userdata)
{
	Evas_Object *win, *btn, *box;
	GList *l;

	struct _inwin_list_pack *pack = malloc(sizeof(struct _inwin_list_pack));
	pack->callback = callback;
	pack->data = userdata;
	win = ui_utils_view_window_get(view);
	pack->inwin = elm_win_inwin_add(win);

	box = elm_box_add(win);
// 	elm_box_homogenous_set(box, EINA_TRUE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	pack->list = elm_list_add(win);
	elm_object_style_set(pack->list, "fieldlist");
 	elm_win_resize_object_add(win, pack->list);
	evas_object_size_hint_align_set(pack->list, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pack->list, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
// 	evas_object_size_hint_align_set(pack->list, 0.5, 0.5);
	elm_list_horizontal_mode_set(pack->list, ELM_LIST_COMPRESS);
	for (l = g_list_first(list); l; l = g_list_next(l)) {
		elm_list_item_append(pack->list, strdup(l->data),
				     NULL, NULL, NULL, NULL);
	}
	g_list_free(list);
	evas_object_smart_callback_add(pack->list, "selected",
				       _inwin_list_selected_cb, pack);
	elm_list_go(pack->list);
	evas_object_show(pack->list);

 	elm_box_pack_end(box, pack->list);

	btn = elm_button_add(win);
	elm_button_label_set(btn, D_("Cancel"));
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
	evas_object_smart_callback_add(btn, "clicked",
				       _inwin_list_cancel_cb, pack);
	evas_object_show(btn);
	elm_box_pack_end(box, btn);

	elm_win_resize_object_add(win, box);
 	evas_object_show(box);
	elm_win_inwin_content_set(pack->inwin, box);

	elm_win_inwin_activate(pack->inwin);

	return pack->inwin;
}

struct _field_select_pack {
	void (*callback)(const char *, void *);
	void *data;
	struct View *view;
};

static void
_field_select_cb(GHashTable *fields, gpointer data)
{
	struct _field_select_pack *pack = (struct _field_select_pack *)data;
	if (!fields) {
		g_warning("No fields for contacts?");
		// TODO: show a user visible message
		return;
	}

	ui_utils_view_inwin_list(pack->view, g_hash_table_get_keys(fields),
				 pack->callback, pack->data);
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

int
ui_utils_view_is_init(struct View *view)
{
	return (view->win);
}

char *
ui_utils_entry_utf8_get(Evas_Object *entry)
{
	if (!entry)
		return NULL;
	/* this allocates a new char * */
	char *s = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));
	if (s)
		return g_strstrip(s);
	return NULL;
}
