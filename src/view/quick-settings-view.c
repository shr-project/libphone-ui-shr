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
	
	elm_layout_sizing_eval(view.parent.layout);
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
