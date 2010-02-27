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
#include <glib-object.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui-utils-sound.h>
#include <phoneui/phoneui-info.h>

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
static void _profiles_list_cb(GError *error, char **list, gpointer userdata);
static void _profile_get_cb(GError *error, char *profile, gpointer userdata);
static void _profile_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _button_lock_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _button_shutdown_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _button_suspend_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _airplane_slide_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _dimming_slide_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _suspend_slide_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _profile_changed_signal_cb(void *userdata, const char *profile);
static void _resource_changed_signal_cb(void *userdata, char *resource, gboolean state, GHashTable *attributes);
static void _cpu_get_policy_cb(GError *error, char *policy, gpointer userdata);
static void _display_get_policy_cb(GError *error, char *policy, gpointer userdata);

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
	evas_object_smart_callback_add(view.airplane_slide, "changed",
				       _airplane_slide_changed_cb, NULL);
	evas_object_show(view.airplane_slide);

	view.dimming_slide = elm_toggle_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-auto-frame-dimming-slide", view.dimming_slide);
	evas_object_smart_callback_add(view.dimming_slide, "changed",
				       _dimming_slide_changed_cb, NULL);
	evas_object_show(view.dimming_slide);

	view.suspend_slide = elm_toggle_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-auto-frame-suspend-slide", view.suspend_slide);
	evas_object_smart_callback_add(view.suspend_slide, "changed",
				       _suspend_slide_changed_cb, NULL);
	evas_object_show(view.suspend_slide);

	view.profiles_combo = elm_hoversel_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "profiles-frame-profiles-combo", view.profiles_combo);
	elm_hoversel_hover_parent_set(view.profiles_combo, win);
	evas_object_show(view.profiles_combo);
	evas_object_smart_callback_add(view.profiles_combo, "selected", _profile_selected_cb, NULL);

	view.button_lock = elm_button_add(win);
	elm_button_label_set(view.button_lock, D_("Lock"));
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-lock-button", view.button_lock);
	evas_object_smart_callback_add(view.button_lock, "clicked",
				       _button_lock_clicked_cb, NULL);
	evas_object_show(view.button_lock);

	view.button_suspend = elm_button_add(win);
	elm_button_label_set(view.button_suspend, D_("Suspend"));
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-suspend-button", view.button_suspend);
	evas_object_smart_callback_add(view.button_suspend, "clicked",
				       _button_suspend_clicked_cb, NULL);
	evas_object_show(view.button_suspend);

	view.button_shutdown = elm_button_add(win);
	elm_button_label_set(view.button_shutdown, D_("Shutdown"));
	ui_utils_view_swallow(VIEW_PTR(view), "power-frame-shutdown-button", view.button_shutdown);
	evas_object_smart_callback_add(view.button_shutdown, "clicked",
				       _button_shutdown_clicked_cb, NULL);
	evas_object_show(view.button_shutdown);
	
	phoneui_utils_sound_profile_list(_profiles_list_cb, NULL);
	phoneui_utils_sound_profile_get(_profile_get_cb, NULL);
	phoneui_utils_resources_get_resource_policy("CPU", _cpu_get_policy_cb, NULL);
	phoneui_utils_resources_get_resource_policy("Display", _display_get_policy_cb, NULL);

	elm_layout_sizing_eval(view.parent.layout);

	/*Register to all signals*/
	phoneui_info_register_profile_changes(_profile_changed_signal_cb, NULL);
	phoneui_info_register_resource_changes(_resource_changed_signal_cb, NULL);

	/*FIXME: until we implement it*/
	elm_object_disabled_set(view.airplane_slide, 1);
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

static void
_profile_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *profile;
	profile = elm_hoversel_item_label_get(event_info);
	/*FIXME: add a callback to handle errors - setting hoversel label should
	 * also be done in signal callback, should probably add a rollback if failed here*/
	elm_hoversel_label_set(view.profiles_combo, profile);
	phoneui_utils_sound_profile_set(profile, NULL, NULL);
}

static void
_profiles_list_cb(GError *error, char **list, gpointer userdata)
{
	/*FIXME: I should probably free this list, but how?, CHECK DBUS*/
	(void) userdata;
	char *profile;
	Elm_Hoversel_Item *item;

	if (error || !list) {
		g_warning("Failed to retrieve profiles list");
		return;
	}

	elm_hoversel_hover_begin(view.profiles_combo);
	for (profile = *list ; profile ; profile = *(++list)) {
		elm_hoversel_item_add(view.profiles_combo, profile, NULL,
			ELM_ICON_NONE, NULL, NULL);
	}
}

static void
_profile_changed_signal_cb(void *userdata, const char *profile)
{
	_profile_get_cb(NULL, profile, userdata);
}

static void
_profile_get_cb(GError *error, char *profile, gpointer userdata)
{
	/*FIXME: I should probably free this profile, but how?, CHECK DBUS*/
	(void) userdata;

	if (error) {
		g_warning("Failed to retrieve active profile");
		return;
	}
	elm_hoversel_label_set(view.profiles_combo, profile);
}

static void
_button_lock_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	(void) obj;
	/*FIXME: Add error handling */
	phoneui_utils_idle_set_state(PHONEUI_DEVICE_IDLE_STATE_LOCK, NULL, NULL);
}

static void
_button_shutdown_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	(void) obj;
	/*FIXME: Add error handling */
	phoneui_utils_usage_shutdown(NULL, NULL);
}

static void
_button_suspend_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	(void) obj;
	/*FIXME: Add error handling */
	phoneui_utils_usage_suspend(NULL, NULL);
}

static void
_airplane_slide_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	int state = elm_toggle_state_get(obj);
	if (state) {
		
	}
	else {
		
	}
}
static void
_resource_changed_signal_cb(void *userdata, char *resource, gboolean state, GHashTable *attributes)
{
	const GValue *tmp;
	int policy;
	Evas_Object *toggle = NULL;
	int pol = 0;
	(void) userdata;
	(void) state;
	if (!strcmp(resource, "Display")) {
		toggle = view.dimming_slide;
	}
	else if (!strcmp(resource, "CPU")) {
		toggle = view.suspend_slide;
	}
	else {
		goto clean;
	}
	tmp = g_hash_table_lookup(attributes, "policy");
	if (!tmp) {
		goto clean;
	}
	policy = g_value_get_int(tmp);

	/* policy enabled = 2 auto = 0 */
	if (policy == 2) {
		elm_toggle_state_set(toggle, 1);
	}
	else if (policy == 0) {
		elm_toggle_state_set(toggle, 0);
	}
	
clean:
	/*FIXME: how should I clean it?! */
	return;
	g_free(resource);
	g_hash_table_destroy(attributes);
}

static void
_cpu_get_policy_cb(GError *error, char *policy, gpointer userdata)
{
	/*FIXME: I should probably free this profile, but how?, CHECK DBUS*/
	(void) userdata;

	if (error || !policy) {
		g_warning("Failed to get CPU policy");
		elm_object_disabled_set(view.suspend_slide, 1);
		return;
	}
	else {
		elm_object_disabled_set(view.suspend_slide, 0);
	}
	
	if (!strcmp(policy, "enabled")) {
		elm_toggle_state_set(view.suspend_slide, 1);
	}
	else if (!strcmp(policy, "auto")) {
		elm_toggle_state_set(view.suspend_slide, 0);
	}
}

static void
_display_get_policy_cb(GError *error, char *policy, gpointer userdata)
{
	/*FIXME: I should probably free this profile, but how?, CHECK DBUS*/
	(void) userdata;

	if (error || !policy) {
		g_warning("Failed to get Display policy");
		elm_object_disabled_set(view.dimming_slide, 1);
		return;
	}
	else {
		elm_object_disabled_set(view.dimming_slide, 0);
	}
	
	if (!strcmp(policy, "enabled")) {
		elm_toggle_state_set(view.dimming_slide, 1);
	}
	else if (!strcmp(policy, "auto")) {
		elm_toggle_state_set(view.dimming_slide, 0);
	}
}

static void
_dimming_slide_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	int state = elm_toggle_state_get(obj);
	/*FIXME: Add error handling */
	if (state) {
		phoneui_utils_resources_set_resource_policy("Display", "enabled", NULL, NULL);	
	}
	else {
		phoneui_utils_resources_set_resource_policy("Display", "auto", NULL, NULL);
	}
}
static void
_suspend_slide_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	int state = elm_toggle_state_get(obj);
	/*FIXME: Add error handling */
	if (state) {
		phoneui_utils_resources_set_resource_policy("CPU", "enabled", NULL, NULL);
	}
	else {
		phoneui_utils_resources_set_resource_policy("CPU", "auto", NULL, NULL);
	}
}
