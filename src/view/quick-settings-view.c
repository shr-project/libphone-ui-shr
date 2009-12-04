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

#include <config.h>

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>

#include <Elementary.h>

#include <glib.h>
#include <glib/gprintf.h>

// #include <gconf/gconf-client.h>

#include <time.h>

#include <config.h>

#include "util/common-utils.h"
#include "util/ui-utils.h"
#include "views.h"
#include "quick-settings-view.h"

struct QuickSettingsViewData {
	struct View parent;
	short int debug, airplane_mode, network_gsm, network_wifi, network_gprs,
		network_usb, network_bluez, network_gps, auto_dim, auto_suspend;
	Evas_Object *bg, *settings_box, *settings_area, *settings;
};

static struct QuickSettingsViewData view;

//GConfClient *conf_client = NULL;
//GConfEngine *conf_engine = NULL;

static void _quick_settings_destroy_cb(struct View *_view);

static void
on_toggle_airplane_mode(void * data, Evas_Object *this_check, void *event_info)
{
	Evas_Object *win=NULL, *check=NULL, *toggle=NULL;
	Evas *win_evas=NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);
	toggle = evas_object_name_find(win_evas, "airplane_mode");

	if(elm_toggle_state_get(toggle)) {
		check = evas_object_name_find(win_evas, "network_gsm");
		if(check) elm_check_state_set(check, 0);
		view.network_gsm = 0;

		check = evas_object_name_find(win_evas, "network_wifi");
		if(check) elm_check_state_set(check, 0);
		view.network_wifi = 0;

		check = evas_object_name_find(win_evas, "network_gprs");
		if(check) elm_check_state_set(check, 0);
		view.network_gprs = 0;

		check = evas_object_name_find(win_evas, "network_bluez");
		if(check) elm_check_state_set(check, 0);
		view.network_bluez = 0;

		check = evas_object_name_find(win_evas, "network_gps");
		if(check) elm_check_state_set(check, 0);
		view.network_gps = 0;

		view.airplane_mode = 1;
	} else
		view.airplane_mode = 0;

	printf("Airplane mode: %d\n", view.airplane_mode);
}

static void
on_toggle_network_gsm(void *data, Evas_Object *this_check, void *event_info)
{
	Evas_Object *win=NULL, *check=NULL, *entry=NULL, *toggle=NULL;
	Evas *win_evas = NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);
	entry = evas_object_name_find(win_evas, "network_gsm_status");
	toggle = evas_object_name_find(win_evas, "network_gsm");

	if(elm_toggle_state_get(toggle)) {
		if(view.airplane_mode) {
			check = evas_object_name_find(win_evas, "airplane_mode");
			if(check) {
				elm_toggle_state_set(check, 0);
				on_toggle_airplane_mode(NULL, NULL, NULL);
			}
			view.airplane_mode = 0;
		}

		view.network_gsm = 1;
		elm_entry_entry_set(entry, D_("Connecting GSM..."));
	} else {
		view.network_gsm = 0;
		elm_entry_entry_set(entry, D_("GSM is disconnected."));
	}
}

static void
on_toggle_network_wifi(void *data, Evas_Object *this_check, void *event_info)
{
	Evas_Object *win=NULL, *check=NULL, *entry=NULL, *toggle=NULL;
	Evas *win_evas = NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);
	entry = evas_object_name_find(win_evas, "network_wifi_status");
	toggle = evas_object_name_find(win_evas, "network_wifi");

	if(elm_toggle_state_get(toggle)) {
		if(view.airplane_mode) {
			check = evas_object_name_find(win_evas, "airplane_mode");
			if(check) {
				elm_toggle_state_set(check, 0);
				on_toggle_airplane_mode(NULL, NULL, NULL);
			}
			view.airplane_mode = 0;
		}

		view.network_wifi = 1;
		elm_entry_entry_set(entry, D_("Connecting..."));
	} else {
		view.network_wifi = 0;
		elm_entry_entry_set(entry, D_("Disconnected."));
	}
}

static void
on_toggle_network_gprs(void *data, Evas_Object *this_check, void *event_info)
{
	Evas_Object *win=NULL, *check=NULL, *entry=NULL, *toggle=NULL;
	Evas *win_evas = NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);
	entry = evas_object_name_find(win_evas, "network_gprs_status");
	toggle = evas_object_name_find(win_evas, "network_gprs");

	if(elm_toggle_state_get(toggle)) {
		if(view.airplane_mode) {
			check = evas_object_name_find(win_evas, "airplane_mode");
			if(check) {
				elm_toggle_state_set(check, 0);
				on_toggle_airplane_mode(NULL, NULL, NULL);
			}
			view.airplane_mode = 0;
		}

		view.network_gprs = 1;
		elm_entry_entry_set(entry, D_("Connecting..."));
	} else {
		view.network_gprs = 0;
		elm_entry_entry_set(entry, D_("Disconnected."));
	}
}

static void
on_toggle_network_bluez(void *data, Evas_Object *this_check, void *event_info)
{
	Evas_Object *win=NULL, *check=NULL, *entry=NULL, *toggle=NULL;
	Evas *win_evas = NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);
	entry = evas_object_name_find(win_evas, "network_bluez_status");
	toggle = evas_object_name_find(win_evas, "network_bluez");

	if(elm_toggle_state_get(toggle)) {
		if(view.airplane_mode) {
			check = evas_object_name_find(win_evas, "airplane_mode");
			if(check) {
				elm_toggle_state_set(check, 0);
				on_toggle_airplane_mode(NULL, NULL, NULL);
			}
			view.airplane_mode = 0;
		}

		view.network_bluez = 1;
		elm_entry_entry_set(entry, D_("Connecting..."));
	} else {
		view.network_bluez = 0;
		elm_entry_entry_set(entry, D_("Disconnected."));
	}
}

static void
on_toggle_network_gps(void *data, Evas_Object *this_check, void *event_info)
{
	Evas_Object *win=NULL, *check=NULL, *entry=NULL, *toggle=NULL;
	Evas *win_evas = NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);
	entry = evas_object_name_find(win_evas, "network_gps_status");
	toggle = evas_object_name_find(win_evas, "network_gps");

	if(elm_toggle_state_get(toggle)) {
		if(view.airplane_mode) {
			check = evas_object_name_find(win_evas, "airplane_mode");
			if(check) {
				elm_toggle_state_set(check, 0);
				on_toggle_airplane_mode(NULL, NULL, NULL);
			}
			view.airplane_mode = 0;
		}

		view.network_gps = 1;
		elm_entry_entry_set(entry, D_("Connecting..."));
	} else {
		view.network_gps = 0;
		elm_entry_entry_set(entry, D_("Disconnected."));
	}
}

static void
on_toggle_network_usb(void *data, Evas_Object *this_check, void *event_info)
{
	Evas_Object *win=NULL, *check=NULL, *entry=NULL, *toggle=NULL;
	Evas *win_evas = NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);
	entry = evas_object_name_find(win_evas, "network_usb_status");
	toggle = evas_object_name_find(win_evas, "network_usb");

	if(elm_toggle_state_get(toggle)) {
		view.network_usb = 1;
		elm_entry_entry_set(entry, D_("Connecting..."));
	} else {
		view.network_usb = 0;
		elm_entry_entry_set(entry, D_("Disconnected."));
	}
}

static void
on_phone_profile_set(void *data, Evas_Object *profiles, void *event_info)
{
	const char *new_label = (const char*)data;
	char *message=NULL;

	message=g_strdup_printf("Profiles (%s)", new_label);
	elm_hoversel_label_set(profiles, message);
	g_free(message);
}

static void
on_suspend(void *data, Evas_Object *obj, void *event_info)
{
	printf("Suspend\n");
}

static void
on_lock(void *data, Evas_Object *obj, void *event_info)
{
	printf("Lock\n");
}

static void
on_poweroff(void *data, Evas_Object *obj, void *event_info)
{
	printf("Power-off\n");
}

static void
on_toggle_autodim(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *win=NULL, *toggle=NULL;
	Evas *win_evas = NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);

	toggle = evas_object_name_find(win_evas, "auto_dim");

	if(elm_toggle_state_get(toggle))
		view.auto_dim=1;
	else
		view.auto_dim=0;

	printf("Autodim: %d!\n", view.auto_dim);
}

static void
on_toggle_autosuspend(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *win=NULL, *toggle=NULL;
	Evas *win_evas = NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);

	toggle = evas_object_name_find(win_evas, "auto_suspend");

	if(elm_toggle_state_get(toggle))
		view.auto_suspend=1;
	else
		view.auto_suspend=0;

	printf("Autosuspend: %d!\n", view.auto_suspend);
}

void
profiles_fill(Evas_Object *profiles)
{
	Elm_Hoversel_Item * hsi=NULL;
	int idx=0;
	const char * profile_list[] = { "Default", "Ring", "Vibrate", "Silence", NULL };

	elm_hoversel_hover_begin(profiles);
	while(profile_list[idx] != NULL) {
		hsi = elm_hoversel_item_add(profiles, profile_list[idx], NULL, ELM_ICON_NONE, on_phone_profile_set, profile_list[idx]);
		idx++;
	}
	elm_hoversel_hover_end(profiles);
}

static void
on_phone_settings(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *win=NULL, *box=NULL, *netbox=NULL, *profiles=NULL, *buttons=NULL, *button=NULL, *toggle=NULL, *frame=NULL, *label=NULL;

	win = ui_utils_view_window_get(&view.parent);

	if(view.settings)
		evas_object_del(view.settings);

	view.settings = elm_box_add(win);
	{
		evas_object_size_hint_weight_set(view.settings, 1, 0);
		evas_object_size_hint_align_set(view.settings, -1, 0);

		frame = elm_frame_add(win);
		{
			evas_object_size_hint_weight_set(frame, 1, 0);
			evas_object_size_hint_align_set(frame, -1, 0);

			elm_frame_label_set(frame, "Phone profiles...");

			box = elm_box_add(win);
			{
				evas_object_size_hint_weight_set(box, 1, 0);
				evas_object_size_hint_align_set(box, -1, 0);
				elm_box_horizontal_set(box, 1);

				netbox = elm_box_add(win);
				{
					evas_object_size_hint_weight_set(netbox, 1, 0);
					evas_object_size_hint_align_set(netbox, 0.5, 0.5);

					label = elm_label_add(win);
					elm_label_label_set(label, "Airplane mode:");
					elm_box_pack_end(netbox, label);
					evas_object_show(label);

					toggle = elm_toggle_add(win);
					{
						evas_object_size_hint_weight_set(toggle, 1, 0);
						evas_object_size_hint_align_set(toggle, 0.5, 0);
						elm_toggle_states_labels_set(toggle, D_("On"), D_("Off"));

						if(view.airplane_mode)
							elm_toggle_state_set(toggle, 1);
						else
							elm_toggle_state_set(toggle, 0);

						evas_object_smart_callback_add(toggle, "changed", on_toggle_airplane_mode, NULL);
						evas_object_name_set(toggle, "airplane_mode");

						elm_box_pack_end(netbox, toggle);
					}
					evas_object_show(toggle);

					elm_box_pack_end(box, netbox);
				}
				evas_object_show(netbox);

				
				profiles = elm_hoversel_add(win);
				evas_object_size_hint_weight_set(profiles, 1, 0);
				evas_object_size_hint_align_set(profiles, 0.5, 0.5);
				elm_hoversel_hover_parent_set(profiles, view.settings);

				elm_hoversel_label_set(profiles, "Profiles (Default)");
				profiles_fill(profiles);

				elm_box_pack_end(box, profiles);
				evas_object_show(profiles);

				elm_frame_content_set(frame, box);
			}
			evas_object_show(box);

			elm_box_pack_end(view.settings, frame);
		}
		evas_object_show(frame);

		box = elm_box_add(win);
		{
			evas_object_size_hint_weight_set(box, 1, 1);
			evas_object_size_hint_align_set(box, -1, 0);
			elm_box_horizontal_set(box, 1);

			frame = elm_frame_add(win);
			{
				evas_object_size_hint_weight_set(frame, 1, 0);
				evas_object_size_hint_align_set(frame, -1, 0);
				elm_frame_label_set(frame, D_("Power settings..."));

				netbox = elm_box_add(win);
				{
					evas_object_size_hint_weight_set(box, 1, 0);
					evas_object_size_hint_align_set(box, -1, 0);

					toggle = elm_toggle_add(win);
					{
						evas_object_size_hint_weight_set(toggle, 1, 0);
						evas_object_size_hint_align_set(toggle, 0.5, 0);
						elm_toggle_label_set(toggle, D_("Auto-dimming:"));
						elm_toggle_states_labels_set(toggle, D_("On"), D_("Off"));

						if(view.auto_dim)
							elm_toggle_state_set(toggle, 1);
						else
							elm_toggle_state_set(toggle, 0);

						evas_object_smart_callback_add(toggle, "changed", on_toggle_autodim, NULL);
						evas_object_name_set(toggle, "auto_dim");

						evas_object_show(toggle);
						elm_box_pack_end(netbox, toggle);
					}
					evas_object_show(toggle);

					toggle = elm_toggle_add(win);
					{
						evas_object_size_hint_weight_set(toggle, 1, 0);
						evas_object_size_hint_align_set(toggle, 0.5, 0);
						elm_toggle_label_set(toggle, D_("Auto-suspend:"));
						elm_toggle_states_labels_set(toggle, D_("On"), D_("Off"));

						if(view.auto_suspend)
							elm_toggle_state_set(toggle, 1);
						else
							elm_toggle_state_set(toggle, 0);

						evas_object_smart_callback_add(toggle, "changed", on_toggle_autosuspend, NULL);
						evas_object_name_set(toggle, "auto_suspend");

						evas_object_show(toggle);
						elm_box_pack_end(netbox, toggle);
					}
					evas_object_show(toggle);

					buttons = elm_table_add(win);
					{
						evas_object_size_hint_weight_set(buttons, 1, 1);
						evas_object_size_hint_align_set(buttons, -1, 0);

						button = elm_button_add(win);
						evas_object_size_hint_weight_set(button, 1, 1);
						evas_object_size_hint_align_set(button, 0.5, 0);
						elm_button_label_set(button, D_("Suspend"));
						evas_object_smart_callback_add(button, "clicked", on_suspend, NULL);
						elm_table_pack(buttons, button, 0, 0, 1, 1);
						evas_object_show(button);

						button = elm_button_add(win);
						evas_object_size_hint_weight_set(button, 1, 1);
						evas_object_size_hint_align_set(button, 0.5, 0);
						elm_button_label_set(button, D_("Lock"));
						evas_object_smart_callback_add(button, "clicked", on_lock, NULL);
						elm_table_pack(buttons, button, 1, 0, 1, 1);
						evas_object_show(button);

						button = elm_button_add(win);
						evas_object_size_hint_weight_set(button, 1, 1);
						evas_object_size_hint_align_set(button, 0.5, 0);
						elm_button_label_set(button, D_("Power-off"));
						evas_object_smart_callback_add(button, "clicked", on_poweroff, NULL);
						elm_table_pack(buttons, button, 2, 0, 1, 1);
						evas_object_show(button);

						elm_box_pack_end(netbox, buttons);
					}
					evas_object_show(buttons);

					elm_frame_content_set(frame, netbox);
				}
				evas_object_show(netbox);
			}
			evas_object_show(frame);

			elm_box_pack_end(view.settings, frame);
		}
		evas_object_show(box);
	}
	elm_scroller_content_set(view.settings_area, view.settings);
	evas_object_show(view.settings);
}

static void
on_settings_gprs_apply(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *win=NULL, *entry=NULL;
	Evas *win_evas = NULL;
	const char *apn=NULL, *user=NULL, *password=NULL;

	win = ui_utils_view_window_get(&view.parent);
	win_evas = evas_object_evas_get(win);

	entry = evas_object_name_find(win_evas, "entry_apn");
	apn = elm_entry_entry_get(entry);

	entry = evas_object_name_find(win_evas, "entry_apn_user");
	user = elm_entry_entry_get(entry);

	entry = evas_object_name_find(win_evas, "entry_apn_password");
	password = elm_entry_entry_get(entry);

	printf("GPRS settings for %s: %s/%s\n", apn, user, password);
}

static void
on_connectivity_settings(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object * win = ui_utils_view_window_get(&view.parent);
	Evas_Object *netbox=NULL, *button=NULL, *check=NULL, *frame=NULL, *subframe=NULL, *networks=NULL, *icon=NULL, *entry=NULL, *table=NULL;

	if(view.settings)
		evas_object_del(view.settings);

	view.settings = elm_box_add(win);
	{
		evas_object_size_hint_weight_set(view.settings, 1, 0);
		evas_object_size_hint_align_set(view.settings, -1, 0);

		frame = elm_frame_add(win);
		{
			evas_object_size_hint_weight_set(frame, 1, 0);
			evas_object_size_hint_align_set(frame, -1, 0);
			elm_frame_label_set(frame, D_("Enabled networks..."));

			networks = elm_table_add(win);
			{
				evas_object_size_hint_weight_set(networks, 0, 0);
				evas_object_size_hint_align_set(networks, -1, 0);

				netbox = elm_box_add(win);
				{
					evas_object_size_hint_weight_set(netbox, 1, 0);
					evas_object_size_hint_align_set(netbox, -1, 0);

					check = elm_check_add(win);
					{
						evas_object_size_hint_weight_set(check, 1, 0);
						evas_object_size_hint_align_set(check, 0, 0);
						elm_check_label_set(check, D_("GSM"));

						icon = elm_icon_add(win);
						elm_icon_standard_set(icon, "edit");
						evas_object_show(icon);
						elm_check_icon_set(check, icon);

						if(view.network_gsm)
							elm_check_state_set(check, 1);
						else
							elm_check_state_set(check, 0);

						evas_object_smart_callback_add(check, "changed", on_toggle_network_gsm, NULL);
						evas_object_name_set(check, "network_gsm");

						evas_object_show(check);
					}
					elm_box_pack_end(netbox, check);

					entry = elm_entry_add(win);
					evas_object_size_hint_weight_set(entry, 1, 1);
					evas_object_size_hint_align_set(entry, -1, 0);
					evas_object_name_set(entry, "network_gsm_status");
					elm_entry_editable_set(entry, 0);
					elm_entry_line_wrap_set(entry, 1);
					elm_entry_single_line_set(entry, 0);

					elm_box_pack_end(netbox, entry);
					evas_object_show(entry);

					elm_table_pack(networks, netbox, 0, 0, 1, 1);
				}
				evas_object_show(netbox);

				netbox = elm_box_add(win);
				{
					evas_object_size_hint_weight_set(netbox, 1, 0);
					evas_object_size_hint_align_set(netbox, -1, 0);

					check = elm_check_add(win);
					{
						evas_object_size_hint_weight_set(check, 1, 0);
						evas_object_size_hint_align_set(check, 0, 0);
						elm_check_label_set(check, D_("Wireless"));

						icon = elm_icon_add(win);
						elm_icon_standard_set(icon, "edit");
						evas_object_show(icon);
						elm_check_icon_set(check, icon);

						if(view.network_wifi)
							elm_check_state_set(check, 1);
						else
							elm_check_state_set(check, 0);

						evas_object_smart_callback_add(check, "changed", on_toggle_network_wifi, NULL);
						evas_object_name_set(check, "network_wifi");

						elm_box_pack_end(netbox, check);
					}
					evas_object_show(check);

					entry = elm_entry_add(win);
					evas_object_size_hint_weight_set(entry, 1, 1);
					evas_object_size_hint_align_set(entry, -1, 0);
					evas_object_name_set(entry, "network_wifi_status");
					elm_entry_editable_set(entry, 0);
					elm_entry_line_wrap_set(entry, 1);
					elm_entry_single_line_set(entry, 0);

					elm_box_pack_end(netbox, entry);
					evas_object_show(entry);

					elm_table_pack(networks, netbox, 1, 0, 1, 1);
				}
				evas_object_show(netbox);

				netbox = elm_box_add(win);
				{
					evas_object_size_hint_weight_set(netbox, 1, 0);
					evas_object_size_hint_align_set(netbox, -1, 0);

					check = elm_check_add(win);
					{
						evas_object_size_hint_weight_set(check, 1, 0);
						evas_object_size_hint_align_set(check, 0, 0);
						elm_check_label_set(check, D_("GPRS"));

						icon = elm_icon_add(win);
						elm_icon_standard_set(icon, "edit");
						evas_object_show(icon);
						elm_check_icon_set(check, icon);

						if(view.network_gprs)
							elm_check_state_set(check, 1);
						else
							elm_check_state_set(check, 0);

						evas_object_smart_callback_add(check, "changed", on_toggle_network_gprs, NULL);
						evas_object_name_set(check, "network_gprs");

						elm_box_pack_end(netbox, check);
					}
					evas_object_show(check);

					entry = elm_entry_add(win);
					evas_object_size_hint_weight_set(entry, 1, 1);
					evas_object_size_hint_align_set(entry, -1, 0);
					evas_object_name_set(entry, "network_gprs_status");
					elm_entry_editable_set(entry, 0);
					elm_entry_line_wrap_set(entry, 1);
					elm_entry_single_line_set(entry, 0);

					elm_box_pack_end(netbox, entry);
					evas_object_show(entry);

					elm_table_pack(networks, netbox, 0, 1, 1, 1);
				}
				evas_object_show(netbox);

				netbox = elm_box_add(win);
				{
					evas_object_size_hint_weight_set(netbox, 1, 0);
					evas_object_size_hint_align_set(netbox, -1, 0);

					check = elm_check_add(win);
					{
						evas_object_size_hint_weight_set(check, 1, 0);
						evas_object_size_hint_align_set(check, 0, 0);
						elm_check_label_set(check, D_("USB"));

						icon = elm_icon_add(win);
						elm_icon_standard_set(icon, "edit");
						evas_object_show(icon);
						elm_check_icon_set(check, icon);

						if(view.network_usb)
							elm_check_state_set(check, 1);
						else
							elm_check_state_set(check, 0);

						evas_object_smart_callback_add(check, "changed", on_toggle_network_usb, NULL);
						evas_object_name_set(check, "network_usb");

						elm_box_pack_end(netbox, check);
					}
					evas_object_show(check);

					entry = elm_entry_add(win);
					evas_object_size_hint_weight_set(entry, 1, 1);
					evas_object_size_hint_align_set(entry, -1, 0);
					evas_object_name_set(entry, "network_usb_status");
					elm_entry_editable_set(entry, 0);
					elm_entry_line_wrap_set(entry, 1);
					elm_entry_single_line_set(entry, 0);

					elm_box_pack_end(netbox, entry);
					evas_object_show(entry);

					elm_table_pack(networks, netbox, 1, 1, 1, 1);
				}
				evas_object_show(netbox);

				netbox = elm_box_add(win);
				{
					evas_object_size_hint_weight_set(netbox, 1, 0);
					evas_object_size_hint_align_set(netbox, -1, 0);

					check = elm_check_add(win);
					{
						evas_object_size_hint_weight_set(check, 1, 0);
						evas_object_size_hint_align_set(check, 0, 0);
						elm_check_label_set(check, D_("Bluetooth"));

						icon = elm_icon_add(win);
						elm_icon_standard_set(icon, "edit");
						evas_object_show(icon);
						elm_check_icon_set(check, icon);

						if(view.network_usb)
							elm_check_state_set(check, 1);
						else
							elm_check_state_set(check, 0);

						evas_object_smart_callback_add(check, "changed", on_toggle_network_bluez, NULL);
						evas_object_name_set(check, "network_bluez");

						elm_box_pack_end(netbox, check);
					}
					evas_object_show(check);

					entry = elm_entry_add(win);
					evas_object_size_hint_weight_set(entry, 1, 1);
					evas_object_size_hint_align_set(entry, -1, 0);
					evas_object_name_set(entry, "network_bluez_status");
					elm_entry_editable_set(entry, 0);
					elm_entry_line_wrap_set(entry, 1);
					elm_entry_single_line_set(entry, 0);

					elm_box_pack_end(netbox, entry);
					evas_object_show(entry);

					elm_table_pack(networks, netbox, 0, 2, 1, 1);
				}
				evas_object_show(netbox);

				netbox = elm_box_add(win);
				{
					evas_object_size_hint_weight_set(netbox, 1, 0);
					evas_object_size_hint_align_set(netbox, -1, 0);

					check = elm_check_add(win);
					{
						evas_object_size_hint_weight_set(check, 1, 0);
						evas_object_size_hint_align_set(check, 0, 0);
						elm_check_label_set(check, D_("GPS"));

						icon = elm_icon_add(win);
						elm_icon_standard_set(icon, "edit");
						evas_object_show(icon);
						elm_check_icon_set(check, icon);

						if(view.network_gps)
							elm_check_state_set(check, 1);
						else
							elm_check_state_set(check, 0);

						evas_object_smart_callback_add(check, "changed", on_toggle_network_gps, NULL);
						evas_object_name_set(check, "network_gps");

						elm_box_pack_end(netbox, check);
					}
					evas_object_show(check);

					entry = elm_entry_add(win);
					evas_object_size_hint_weight_set(entry, 1, 1);
					evas_object_size_hint_align_set(entry, -1, 0);
					evas_object_name_set(entry, "network_gps_status");
					elm_entry_editable_set(entry, 0);
					elm_entry_line_wrap_set(entry, 1);
					elm_entry_single_line_set(entry, 0);

					elm_box_pack_end(netbox, entry);
					evas_object_show(entry);

					elm_table_pack(networks, netbox, 1, 2, 1, 1);
				}
				evas_object_show(netbox);
			}
			evas_object_show(networks);

			elm_frame_content_set(frame, networks);

			elm_box_pack_end(view.settings, frame);
		}
		evas_object_show(frame);

		frame = elm_frame_add(win);
		{
			evas_object_size_hint_weight_set(frame, 1, 0);
			evas_object_size_hint_align_set(frame, -1, 0);
			elm_frame_label_set(frame, D_("GPRS settings..."));

			table = elm_table_add(win);
			{
				evas_object_size_hint_weight_set(table, 1, 0);
				evas_object_size_hint_align_set(table, -1, 0);

				subframe = elm_frame_add(win);
				{
					evas_object_size_hint_weight_set(subframe, 1, 0);
					evas_object_size_hint_align_set(subframe, -1, 0);
					elm_frame_label_set(subframe, D_("APN"));

					entry = elm_entry_add(win);
					evas_object_name_set(entry, "entry_apn");
					elm_entry_single_line_set(entry, 1);
					evas_object_show(entry);

					elm_frame_content_set(subframe, entry);
					elm_table_pack(table, subframe, 0, 0, 1, 1);
				}
				evas_object_show(subframe);

				subframe = elm_frame_add(win);
				{
					evas_object_size_hint_weight_set(subframe, 1, 0);
					evas_object_size_hint_align_set(subframe, -1, 0);
					elm_frame_label_set(subframe, D_("User"));

					entry = elm_entry_add(win);
					evas_object_name_set(entry, "entry_apn_user");
					elm_entry_single_line_set(entry, 1);
					evas_object_show(entry);

					elm_frame_content_set(subframe, entry);
					elm_table_pack(table, subframe, 1, 0, 1, 1);
				}
				evas_object_show(subframe);

				subframe = elm_frame_add(win);
				{
					evas_object_size_hint_weight_set(subframe, 1, 0);
					evas_object_size_hint_align_set(subframe, -1, 0);
					elm_frame_label_set(subframe, D_("Password"));

					entry = elm_entry_add(win);
					evas_object_name_set(entry, "entry_apn_password");
					elm_entry_single_line_set(entry, 1);
					evas_object_show(entry);

					elm_frame_content_set(subframe, entry);
					elm_table_pack(table, subframe, 2, 0, 1, 1);
				}
				evas_object_show(subframe);

				button = elm_button_add(win);
				elm_button_label_set(button, D_("Apply"));
				evas_object_smart_callback_add(button, "clicked", on_settings_gprs_apply, NULL);
				elm_table_pack(table, button, 1, 1, 1, 1);
				evas_object_show(button);

				elm_frame_content_set(frame, table);
			}
			evas_object_show(table);

			elm_box_pack_end(view.settings, frame);
		}
		evas_object_show(frame);

		frame = elm_frame_add(win);
		evas_object_size_hint_weight_set(frame, 1, 0);
		evas_object_size_hint_align_set(frame, -1, 0);
		elm_frame_label_set(frame, D_("Wifi settings..."));
		elm_box_pack_end(view.settings, frame);
		evas_object_show(frame);

	}
	evas_object_show(view.settings);

	elm_scroller_content_set(view.settings_area, view.settings);
}

int
quick_settings_view_init()
{
	Evas_Object *win=NULL, *toolbar=NULL, *icon=NULL;
	Elm_Toolbar_Item *tb_phone=NULL, *tb_item=NULL;
        int ret=0;

	g_debug("Initializing the quick settings screen");
        ret = ui_utils_view_init(&view.parent, ELM_WIN_BASIC, D_("Quick settings"),
                                NULL, NULL, _quick_settings_destroy_cb);

	if (ret) {
		g_critical("Faild to init quick settings view");
		return ret;
	}

	win = ui_utils_view_window_get(&view.parent);

	view.bg = elm_bg_add(win);
	evas_object_size_hint_weight_set(view.bg, 1.0, 1.0);
	elm_win_resize_object_add(win, view.bg);
	evas_object_show(view.bg);

	view.settings_box = elm_box_add(win);
	{
		evas_object_size_hint_weight_set(view.settings_box, 1, 1);
		evas_object_size_hint_align_set(view.settings_box, -1, -1);
		elm_win_resize_object_add(win, view.settings_box);

		toolbar = elm_toolbar_add(win);
		{
			evas_object_size_hint_weight_set(toolbar, 1, 0);
			evas_object_size_hint_align_set(toolbar, -1, -1);

			icon = elm_icon_add(win);
			elm_icon_standard_set(icon, "edit");
			evas_object_show(icon);
			tb_phone = elm_toolbar_item_add(toolbar, icon, D_("Phone"), on_phone_settings, NULL);

			icon = elm_icon_add(win);
			elm_icon_standard_set(icon, "edit");
			evas_object_show(icon);
			tb_item = elm_toolbar_item_add(toolbar, icon, D_("Connectivity"), on_connectivity_settings, NULL);

			evas_object_show(toolbar);
		}
		elm_box_pack_end(view.settings_box, toolbar);

		view.settings_area = elm_scroller_add(win);
		elm_scroller_bounce_set(view.settings_area, 0, 1);
		evas_object_size_hint_weight_set(view.settings_area, 1, 1);
		evas_object_size_hint_align_set(view.settings_area, -1, -1);
		evas_object_show(view.settings_area);
		elm_toolbar_item_select(tb_phone);
		elm_box_pack_end(view.settings_box, view.settings_area);
	}
	evas_object_show(view.settings_box);
}

void
quick_settings_view_deinit()
{
        ui_utils_view_deinit(&view.parent);

        evas_object_del(view.bg);
        evas_object_del(view.settings);
        evas_object_del(view.settings_area);
        evas_object_del(view.settings_box);
}

int
quick_settings_view_is_init()
{
        return ui_utils_view_is_init(&view.parent);
}

void
quick_settings_view_show()
{
        ui_utils_view_show(&view.parent);
}

void
quick_settings_view_hide()
{
        ui_utils_view_hide(&view.parent);
}


static void
_quick_settings_destroy_cb(struct View *_view)
{
        struct QuickSettingsViewData *view = (struct QuickSettingsViewData *) _view;
        quick_settings_view_hide();

}

