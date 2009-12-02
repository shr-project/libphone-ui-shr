#include <Elementary.h>
#include <Evas.h>

#include "ui_utils.h"

/* FIXME: hackish, remove */
#include "phoneui-shr.h"

struct InwinButton {
	char *label;
	void (*callback) (void *, Evas_Object *, void *);
};

static void
_view_delete_callback(struct View *view, Evas_Object * win, void *event_info);

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
			void *(*show_cb) (struct View *view),
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
	evas_object_smart_callback_add(view->win, "delete-request",
				      (Evas_Smart_Cb) _view_delete_callback, view);


	/*FIXME: check what the heck is this and probably fix it */
	/*elm_win_autodel_set(view->win, 1);*/
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

	evas_object_resize(view->win, 480, 600);
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
	evas_object_smart_callback_add(view->win, "delete-request", (Evas_Smart_Cb) cb, view);
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
	
	if (view->destroy_cb)
		view->destroy_cb(view);
		
	evas_object_del(view->layout);
	evas_object_del(view->background);
	evas_object_del(view->win);

	

	/* This is enough to mark it's empty. */
	view->win = NULL;
	view->show_cb = NULL;
	view->hide_cb = NULL;
	view->destroy_cb = NULL;
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



static void
_view_delete_callback(struct View *view, Evas_Object * win, void *event_info)
{
	g_debug("_window_delete_callback");
	ui_utils_view_deinit(view);
}

int
ui_utils_view_is_init(struct View *view)
{
	return (view->win);
}

