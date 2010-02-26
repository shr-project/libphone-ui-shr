/*
 * vim:ts=4
 * 
 * Copyright © 2009 Rui Miguel Silva Seabra <rms@1407.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Elementary.h>

#include <glib.h>

#include "util/common-utils.h"
#include "util/ui-utils.h"
#include "views.h"
#include "quick-settings-view.h"

struct QuickSettingsViewData {
	struct View parent;
	char *profile_str;
	Evas_Object *airplane_slide, *profiles_combo, *dimming_slide, *suspend_slide;
	Evas_Object *button_lock, *button_suspend, *button_shutdown;
};

static struct QuickSettingsViewData view;


static void _delete_cb(struct View *view, Evas_Object * win, void *event_info);
int
quick_settings_view_init()
{
	g_debug("Initializing the dialer screen");
	Evas_Object *win;
	int ret;
	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("Quick-settings"),
				NULL, NULL, NULL);

	if (ret) {
		g_critical("Failed to init dialer view");
		return ret;
	}

	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);
	ui_utils_view_layout_set(VIEW_PTR(view), DEFAULT_THEME, "phoneui/settings/quick-settings");

	view.airplane_slide = elm_toggle_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "profiles-frame-airplane-slide", view.airplane_slide);
	evas_object_show(view.airplane_slide);

	view.dimming_slide = elm_toggle_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-auto-frame-dimming-slide", view.dimming_slide);
	evas_object_show(view.dimming_slide);

	view.suspend_slide = elm_toggle_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-auto-frame-suspend-slide", view.suspend_slide);
	evas_object_show(view.suspend_slide);

	view.profiles_combo = elm_hoversel_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "profiles-frame-profiles-combo", view.profiles_combo);
	evas_object_show(view.profiles_combo);

	view.button_lock = elm_button_add(win);
	elm_button_label_set(view.button_lock, D_("Lock"));
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-lock-button", view.button_lock);
	evas_object_show(view.button_lock);

	view.button_suspend = elm_button_add(win);
	elm_button_label_set(view.button_suspend, D_("suspend"));
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-suspend-button", view.button_suspend);
	evas_object_show(view.button_suspend);

	view.button_shutdown = elm_button_add(win);
	elm_button_label_set(view.button_shutdown, D_("shutdown"));
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-shutdown-button", view.button_shutdown);
	evas_object_show(view.button_shutdown);
	
	
	elm_layout_sizing_eval(view.parent.layout);
#if 0
	view.text_number_info = elm_label_add(win);
	elm_label_label_set(view.text_number_info,
			    D_("Click to open contactlist."));
	ui_utils_view_swallow(VIEW_PTR(view), "text_number_info", view.text_number_info);
	evas_object_show(view.text_number_info);

	view.delete_text_icon = elm_icon_add(win);
	evas_object_size_hint_aspect_set(view.delete_text_icon,
					 EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_icon_file_set(view.delete_text_icon, DELETE_TEXT_ICON, NULL);

	view.delete_text_button = elm_button_add(win);
	elm_button_icon_set(view.delete_text_button, view.delete_text_icon);
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
	elm_button_label_set(view.bt_exit, D_("Close"));
	evas_object_smart_callback_add(view.bt_exit, "clicked",
				       _dialer_exit_clicked_cb, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_exit", view.bt_exit);
	evas_object_show(view.bt_exit);

	view.bt_options = elm_button_add(win);
	elm_button_label_set(view.bt_options, D_("More"));
	evas_object_smart_callback_add(view.bt_options, "clicked",
				       _dialer_options_clicked_cb, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_options", view.bt_options);
	evas_object_show(view.bt_options);

	view.bt_call = elm_button_add(win);
	elm_button_label_set(view.bt_call, D_("Call"));
	evas_object_smart_callback_add(view.bt_call, "clicked",
				       _dialer_call_clicked_cb, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_call", view.bt_call);
	evas_object_show(view.bt_call);

	edje_object_signal_callback_add(ui_utils_view_layout_get(VIEW_PTR(view)), "click",
					"number",
					_dialer_number_clicked_cb,
					NULL);

	/* Options */
	view.hv = elm_hover_add(win);
	elm_hover_parent_set(view.hv, win);
	elm_hover_target_set(view.hv, view.bt_options);

	view.bx = elm_box_add(win);
	elm_box_horizontal_set(view.bx, 0);
	elm_box_homogenous_set(view.bx, 1);
	evas_object_show(view.bx);

	view.bt_save = elm_button_add(win);
	elm_button_label_set(view.bt_save, D_("Save"));
	evas_object_size_hint_min_set(view.bt_save, 130, 80);
	evas_object_smart_callback_add(view.bt_save, "clicked",
				       _dialer_contact_add_clicked_cb, NULL);
	evas_object_show(view.bt_save);
	elm_box_pack_end(view.bx, view.bt_save);

	view.bt_message = elm_button_add(win);
	elm_button_label_set(view.bt_message, D_("Send SMS"));
	evas_object_size_hint_min_set(view.bt_message, 130, 80);
	evas_object_smart_callback_add(view.bt_message, "clicked",
				       _dialer_message_clicked_cb, NULL);
	evas_object_show(view.bt_message);
	elm_box_pack_end(view.bx, view.bt_message);

	elm_hover_content_set(view.hv, "top", view.bx);

	view.number[0] = '\0';
	view.length = 0;
#endif
	return 0;
}

void
quick_settings_view_deinit()
{
        ui_utils_view_deinit(VIEW_PTR(view));
}

int
quick_settings_view_is_init()
{
        return ui_utils_view_is_init(VIEW_PTR(view));
}

void
quick_settings_view_show()
{
        ui_utils_view_show(VIEW_PTR(view));
}

void
quick_settings_view_hide()
{
        ui_utils_view_hide(VIEW_PTR(view));
}


static void
_delete_cb(struct View *view, Evas_Object * win, void *event_info)
{
        (void) view;
        (void) win;
        (void) event_info;
        quick_settings_view_hide();
}
