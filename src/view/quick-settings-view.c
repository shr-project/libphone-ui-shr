/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *              Rui Miguel Silva Seabra <rms@1407.org>
 *              David Kozub <zub@linux.fjfi.cvut.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Elementary.h>

#include <glib.h>
#include <glib-object.h>
#include <fsoframework.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui-utils-sound.h>
#include <phoneui/phoneui-info.h>

#include "util/common-utils.h"
#include "util/ui-utils.h"
#include "phoneui-shr.h"
#include "views.h"
#include "quick-settings-view.h"

/* How long do we wait before retrying retrieving sound profile list
 * after a failure. */
static const int PROFILE_RETRY_DELAY = 2000; /*ms*/

struct QuickSettingsViewData {
	struct View parent;
	char *profile_str;
	Evas_Object *toolbar, *pager, *layout1, *layout2;
	Evas_Object *airplane_slide, *profiles_combo, *dimming_slide, *suspend_slide;
	Evas_Object *gprs_slide, *sharing_slide;
	Evas_Object *button_lock, *button_suspend, *button_shutdown;
};

static struct QuickSettingsViewData view;

static void _init_profiles_power_page();
static void _init_network_page();
static void _delete_cb(struct View *view, Evas_Object * win, void *event_info);
static gboolean _profiles_list_retry(gpointer data);
static void _profiles_list_cb(GError *error, char **list, int count, gpointer userdata);
static void _profile_get_current_cb(GError *error, char *profile, gpointer userdata);
static void _profile_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _get_offline_mode_cb(GError *error, gboolean offline, gpointer data);
static void _button_lock_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _button_shutdown_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _button_suspend_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _airplane_slide_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _dimming_slide_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _suspend_slide_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _gprs_slide_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _sharing_slide_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _profile_changed_signal_cb(void *userdata, const char *profile);
static void _resource_changed_signal_cb(void *userdata, const char *resource, gboolean state, GHashTable *attributes);
static void _pdp_context_status_signal_cb(void* data, FreeSmartphoneGSMContextStatus status, GHashTable* attributes);
static void _cpu_get_policy_cb(GError *error, FreeSmartphoneUsageResourcePolicy policy, gpointer userdata);
static void _display_get_policy_cb(GError *error, FreeSmartphoneUsageResourcePolicy policy, gpointer userdata);
static void _toolbar_clicked(void *data, Evas_Object *obj, void *event_info);

int
quick_settings_view_init()
{
	g_debug("Initializing the quick-settings screen");
	Evas_Object *win;
	int ret;
	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("Quick-settings"),
				NULL, NULL, NULL);

	if (ret) {
		g_critical("Failed to init quick-settings view");
		return ret;
	}

	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);
	ui_utils_view_layout_set(VIEW_PTR(view), phoneui_theme,
				 "phoneui/settings/quick-settings");

	view.pager = elm_pager_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "pager", view.pager);
	_init_network_page();
	_init_profiles_power_page();
	evas_object_show(view.pager);

	view.toolbar = elm_toolbar_add(win);
	ui_utils_view_swallow(VIEW_PTR(view), "toolbar", view.toolbar);
	elm_toolbar_homogenous_set(view.toolbar, EINA_TRUE);
	elm_toolbar_mode_shrink_set(view.toolbar, ELM_TOOLBAR_SHRINK_NONE);
	elm_toolbar_align_set(view.toolbar, 0.0);
	elm_toolbar_icon_size_set(view.toolbar, 16);
	evas_object_size_hint_weight_set(view.toolbar, 0.0, 0.0);
	evas_object_size_hint_align_set(view.toolbar, EVAS_HINT_FILL, 0.0);

	Elm_Toolbar_Item *tbitem = elm_toolbar_item_append(view.toolbar,
		"icon/profile", D_("Profile/Power"), _toolbar_clicked, view.layout1);

	elm_toolbar_item_append(view.toolbar, "icon/network", D_("Network"),
			     _toolbar_clicked, view.layout2);
	evas_object_show(view.toolbar);
	elm_toolbar_item_selected_set(tbitem, EINA_TRUE);

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
_init_profiles_power_page()
{
	Evas_Object *win = ui_utils_view_window_get(VIEW_PTR(view));
	view.layout1 = elm_layout_add(win);
	elm_win_resize_object_add(win, view.layout1);
	evas_object_size_hint_weight_set(view.layout1,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_file_set(view.layout1, phoneui_theme,
			    "phoneui/settings/quick-settings/profile-power");
	evas_object_show(view.layout1);

	view.airplane_slide = elm_toggle_add(win);
	elm_layout_content_set(view.layout1, "profiles-frame-airplane-slide", view.airplane_slide);
	evas_object_smart_callback_add(view.airplane_slide, "changed",
				       _airplane_slide_changed_cb, NULL);
	evas_object_show(view.airplane_slide);

	view.dimming_slide = elm_toggle_add(win);
	elm_layout_content_set(view.layout1, "power-frame-auto-frame-dimming-slide", view.dimming_slide);
	evas_object_smart_callback_add(view.dimming_slide, "changed",
				       _dimming_slide_changed_cb, NULL);
	evas_object_show(view.dimming_slide);

	view.suspend_slide = elm_toggle_add(win);
	elm_layout_content_set(view.layout1, "power-frame-auto-frame-suspend-slide", view.suspend_slide);
	evas_object_smart_callback_add(view.suspend_slide, "changed",
				       _suspend_slide_changed_cb, NULL);
	evas_object_show(view.suspend_slide);

	elm_toggle_states_labels_set(view.suspend_slide, D_("Forbid"), D_("Allow"));
	elm_toggle_states_labels_set(view.dimming_slide, D_("Forbid"), D_("Allow"));

	view.profiles_combo = elm_hoversel_add(win);
	elm_layout_content_set(view.layout1, "profiles-frame-profiles-combo", view.profiles_combo);
	elm_hoversel_hover_parent_set(view.profiles_combo, win);
	evas_object_show(view.profiles_combo);
	evas_object_smart_callback_add(view.profiles_combo, "selected", _profile_selected_cb, NULL);

	view.button_lock = elm_button_add(win);
	elm_button_label_set(view.button_lock, D_("Lock"));
	elm_layout_content_set(view.layout1, "power-frame-lock-button", view.button_lock);
	evas_object_smart_callback_add(view.button_lock, "clicked",
				       _button_lock_clicked_cb, NULL);
	evas_object_show(view.button_lock);

	view.button_suspend = elm_button_add(win);
	elm_button_label_set(view.button_suspend, D_("Suspend"));
	elm_layout_content_set(view.layout1, "power-frame-suspend-button", view.button_suspend);
	evas_object_smart_callback_add(view.button_suspend, "clicked",
				       _button_suspend_clicked_cb, NULL);
	evas_object_show(view.button_suspend);

	view.button_shutdown = elm_button_add(win);
	elm_button_label_set(view.button_shutdown, D_("Shutdown"));
	elm_layout_content_set(view.layout1, "power-frame-shutdown-button", view.button_shutdown);
	evas_object_smart_callback_add(view.button_shutdown, "clicked",
				       _button_shutdown_clicked_cb, NULL);
	evas_object_show(view.button_shutdown);

	elm_pager_content_push(view.pager, view.layout1);

	/* Disabled until we get the list + current profile. */
	elm_object_disabled_set(view.profiles_combo, EINA_TRUE);
	phoneui_utils_sound_profile_list(_profiles_list_cb, NULL);

	phoneui_utils_resources_get_resource_policy("CPU", _cpu_get_policy_cb, NULL);
	phoneui_utils_resources_get_resource_policy("Display", _display_get_policy_cb, NULL);
	phoneui_utils_get_offline_mode(_get_offline_mode_cb, NULL);

	/*Register to all signals*/
	phoneui_info_register_and_request_profile_changes(_profile_changed_signal_cb, NULL);
	phoneui_info_register_and_request_resource_status(_resource_changed_signal_cb, NULL);
}

static void
_init_network_page()
{
	Evas_Object *win = ui_utils_view_window_get(VIEW_PTR(view));
	view.layout2 = elm_layout_add(win);
	elm_win_resize_object_add(win, view.layout2);
	evas_object_size_hint_weight_set(view.layout2,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_file_set(view.layout2, phoneui_theme,
			    "phoneui/settings/quick-settings/network");
	evas_object_show(view.layout2);

	view.gprs_slide = elm_toggle_add(win);
	elm_layout_content_set(view.layout2, "network-frame-auto-frame-gprs-slide", view.gprs_slide);
	evas_object_smart_callback_add(view.gprs_slide, "changed", _gprs_slide_changed_cb, NULL);
	elm_toggle_states_labels_set(view.gprs_slide, D_("Connected"), D_("Disconnected"));
	evas_object_show(view.gprs_slide);

	view.sharing_slide = elm_toggle_add(win);
	elm_layout_content_set(view.layout2, "network-frame-auto-frame-sharing-slide", view.sharing_slide);
	evas_object_smart_callback_add(view.sharing_slide, "changed", _sharing_slide_changed_cb, NULL);
	elm_toggle_states_labels_set(view.sharing_slide, D_("Sharing"), D_("Not sharing"));
	evas_object_show(view.sharing_slide);

	// FIXME: until we implement it
	elm_object_disabled_set(view.sharing_slide, 1);

	elm_pager_content_push(view.pager, view.layout2);

	phoneui_info_register_and_request_pdp_context_status(_pdp_context_status_signal_cb, NULL);
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
_set_profile_cb(GError *error, gpointer data)
{
	(void) data;
	if (error) {
		g_warning("Failed setting the profile: (%d) %s",
			error->code, error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Failed setting the profile."), error);
	}
}

static void
_profile_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) obj;
	const char *profile;
	profile = elm_hoversel_item_label_get(event_info);
	phoneui_utils_sound_profile_set(profile, _set_profile_cb, NULL);
}

static void
_get_offline_mode_cb(GError *error, gboolean offline, gpointer data)
{
	(void) data;

	if (error) {
		g_warning("Failed with GetOfflineMode: (%d) %s",
			  error->code, error->message);
		g_error_free(error);
		return;
	}
	elm_toggle_state_set(view.airplane_slide, offline);
}

static gboolean
_profiles_list_retry(gpointer data)
{
	(void)data;
	phoneui_utils_sound_profile_list(_profiles_list_cb, NULL);
	return FALSE;
}

static void
_profiles_list_cb(GError *error, char **list, int count, gpointer userdata)
{
	/*FIXME: I should probably free this list, but how?, CHECK DBUS*/
	(void) userdata;
	int i;

	if (error || !list) {
		if (error)
			g_warning("Failed to retrieve profile list: (%d) %s; retrying later",
				error->code, error->message);
		else
			g_warning("Failed to retrieve profile list: call succeeded, "
				"but no data was received; retrying later");

		/* Schedule a retry later */
		g_timeout_add(PROFILE_RETRY_DELAY, _profiles_list_retry, NULL);

		return;
	}

	for (i = 0; i < count; i++) {
		elm_hoversel_item_add(view.profiles_combo, list[i], NULL,
			ELM_ICON_NONE, NULL, NULL);
	}

	/* Determine the current profile. */
	phoneui_utils_sound_profile_get(_profile_get_current_cb, NULL);
}

static void
_profile_get_current_cb(GError *error, char *profile, gpointer userdata)
{
	(void)userdata;

	if (error || !profile) {
		if (error)
			g_warning("Failed to retrieve the current profile: (%d) %s",
				error->code, error->message);
		else
			g_warning("Failed to retrieve the current profile: call succeeded, "
				"but no data was received");

		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Failed to retrieve the current profile."), error);
	}
	else
		elm_hoversel_label_set(view.profiles_combo, profile);

	/* Whether we got the profile or not, enable it now. */
	elm_object_disabled_set(view.profiles_combo, EINA_FALSE);
}

static void
_profile_changed_signal_cb(void *userdata, const char *profile)
{
	/*FIXME: I should probably free this profile, but how?, CHECK DBUS*/
	(void) userdata;
	elm_hoversel_label_set(view.profiles_combo, profile);
}

static void
_button_lock_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	(void) obj;
	/*FIXME: Add error handling */
	phoneui_utils_idle_set_state(FREE_SMARTPHONE_DEVICE_IDLE_STATE_LOCK, NULL, NULL);
	quick_settings_view_hide();
}

static void
_shutdown_confirm_cb(int result, void *data)
{
	(void) data;
	if (result == DIALOG_YES) {
		/*FIXME: Add error handling */
		phoneui_utils_usage_shutdown(NULL, NULL);
		quick_settings_view_hide();
	}
}

static void
_button_shutdown_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	(void) obj;
	ui_utils_dialog(VIEW_PTR(view),
			D_("Do you really want to<br>shutdown the phone?"),
			DIALOG_YES|DIALOG_NO, _shutdown_confirm_cb, NULL);
}

static void
_button_suspend_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	(void) obj;
	/*FIXME: Add error handling */
	phoneui_utils_usage_suspend(NULL, NULL);
	quick_settings_view_hide();
}

static void
_airplane_slide_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	int state = elm_toggle_state_get(obj);
	// FIXME: Add callback with error handling
	phoneui_utils_set_offline_mode(state, NULL, NULL);
}
static void
_resource_changed_signal_cb(void *userdata, const char *resource, gboolean state, GHashTable *attributes)
{
	const GValue *tmp;
	int policy;
	Evas_Object *toggle = NULL;
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

	if (!attributes)
		goto clean;

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
// 	g_free(resource);
        if (attributes)
		g_hash_table_unref(attributes);
}

static void
_pdp_context_status_signal_cb(void* data,
			      FreeSmartphoneGSMContextStatus status,
			      GHashTable* attributes)
{
	(void) data;
	(void) attributes;

	switch (status) {
	case FREE_SMARTPHONE_GSM_CONTEXT_STATUS_ACTIVE:
		elm_object_disabled_set(view.sharing_slide, EINA_FALSE);
	case FREE_SMARTPHONE_GSM_CONTEXT_STATUS_OUTGOING:
		elm_toggle_state_set(view.gprs_slide, EINA_TRUE);
		break;
	default:
		elm_toggle_state_set(view.gprs_slide, EINA_FALSE);
		elm_object_disabled_set(view.sharing_slide, EINA_TRUE);
		break;
	}
	elm_toggle_state_set(view.sharing_slide, EINA_FALSE);
}

static void
_cpu_get_policy_cb(GError* error, FreeSmartphoneUsageResourcePolicy policy,
		   gpointer userdata)
{
	/*FIXME: I should probably free this profile, but how?, CHECK DBUS*/
	(void) userdata;

	if (error) {
		g_warning("Failed to get CPU policy");
		elm_object_disabled_set(view.suspend_slide, 1);
		return;
	}
	else {
		elm_object_disabled_set(view.suspend_slide, 0);
	}

	if (policy == FREE_SMARTPHONE_USAGE_RESOURCE_POLICY_ENABLED) {
		elm_toggle_state_set(view.suspend_slide, 1);
	}
	else if (policy == FREE_SMARTPHONE_USAGE_RESOURCE_POLICY_AUTO) {
		elm_toggle_state_set(view.suspend_slide, 0);
	}
}

static void
_display_get_policy_cb(GError* error, FreeSmartphoneUsageResourcePolicy policy,
		       gpointer userdata)
{
	/*FIXME: I should probably free this profile, but how?, CHECK DBUS*/
	(void) userdata;

	if (error) {
		g_warning("Failed to get Display policy");
		elm_object_disabled_set(view.dimming_slide, 1);
		return;
	}
	else {
		elm_object_disabled_set(view.dimming_slide, 0);
	}

	if (policy == FREE_SMARTPHONE_USAGE_RESOURCE_POLICY_ENABLED) {
		elm_toggle_state_set(view.dimming_slide, 1);
	}
	else if (policy == FREE_SMARTPHONE_USAGE_RESOURCE_POLICY_AUTO) {
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
		phoneui_utils_resources_set_resource_policy("Display",
				FREE_SMARTPHONE_USAGE_RESOURCE_POLICY_ENABLED,
				NULL, NULL);
	}
	else {
		phoneui_utils_resources_set_resource_policy("Display",
				FREE_SMARTPHONE_USAGE_RESOURCE_POLICY_AUTO,
				NULL, NULL);
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
		phoneui_utils_resources_set_resource_policy("CPU",
				FREE_SMARTPHONE_USAGE_RESOURCE_POLICY_ENABLED,
				NULL, NULL);
	}
	else {
		phoneui_utils_resources_set_resource_policy("CPU",
				FREE_SMARTPHONE_USAGE_RESOURCE_POLICY_AUTO,
				NULL, NULL);
	}
}

static void
_pdp_activate_cb(GError *error, gpointer data)
{
	(void) data;
	elm_object_disabled_set(view.gprs_slide, EINA_FALSE);
	if (error) {
		g_warning("Activating PDP failed: (%d) %s",
			  error->code, error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("Activating PDP failed."), error);
		elm_toggle_state_set(view.gprs_slide, EINA_FALSE);
	}
}

static void
_pdp_deactivate_cb(GError *error, gpointer data)
{
	(void) data;
	elm_object_disabled_set(view.gprs_slide, EINA_FALSE);
	if (error) {
		g_warning("De-Activating PDP failed: (%d) %s",
			  error->code, error->message);
		ui_utils_error_message_from_gerror_show(VIEW_PTR(view),
			D_("De-Activating PDP failed."), error);
		/* FIXME: think about if this is right for the deactivate
		          does not work case !!! */
		elm_toggle_state_set(view.gprs_slide, EINA_TRUE);
	}
}

static void
_gprs_slide_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	int state = elm_toggle_state_get(obj);
	/* disable the toggler until we get hit by the signal that
	the PDP context actually really changed */
	elm_object_disabled_set(obj, EINA_TRUE);
	if (state) {
		phoneui_utils_pdp_activate_context(_pdp_activate_cb, NULL);
	}
	else {
		phoneui_utils_pdp_deactivate_context(_pdp_deactivate_cb, NULL);
	}
}

static void
_sharing_slide_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	(void) obj;
	int state = elm_toggle_state_get(obj);
	if (state) {
		phoneui_utils_network_start_connection_sharing("usb0", NULL, NULL);
	}
	else {
		phoneui_utils_network_stop_connection_sharing("usb0", NULL, NULL);
	}
}

static void
_toolbar_clicked(void *data, Evas_Object *obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	Evas_Object *ly = data;
	elm_pager_content_promote(view.pager, ly);
}
