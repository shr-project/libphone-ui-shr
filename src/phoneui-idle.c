#include <phoneui/phoneui.h>

#include "phoneui-idle.h"
#include "view/idle-view.h"

void
phoneui_backend_idle_screen_show()
{
	if (!idle_screen_view_is_init()) {
		if (idle_screen_view_init()) {
			return;
		}
	}
	idle_screen_view_show();
}

void
phoneui_backend_idle_screen_hide()
{
	idle_screen_view_hide();
}

void
phoneui_backend_idle_screen_toggle()
{
	/* FIXME: should also handle is active */
	if (!idle_screen_view_is_init()) {
		if (idle_screen_view_init()) {
			return;
		}
	}
	idle_screen_view_toggle();
}

void
phoneui_backend_idle_screen_update_missed_calls(const int amount)
{
	idle_screen_view_update_missed_calls(amount);
}

void
phoneui_backend_idle_screen_update_unfinished_tasks(const int amount)
{
	idle_screen_view_update_unfinished_tasks(amount);
}

void
phoneui_backend_idle_screen_update_unread_messages(const int amount)
{
	idle_screen_view_update_unread_messages(amount);
}

void
phoneui_backend_idle_screen_update_power(const int capacity)
{
	idle_screen_view_update_power(capacity);
}

void
phoneui_backend_idle_screen_update_call(enum PhoneuiCallState state,
		const char *name, const char *number)
{
	idle_screen_view_update_call(state, name, number);
}

void
phoneui_backend_idle_screen_update_signal_strength(const int signal)
{
	idle_screen_view_update_signal_strength(signal);
}

void
phoneui_backend_idle_screen_update_provider(const char *provider)
{
	idle_screen_view_update_provider(provider);
}

void
phoneui_backend_idle_screen_update_resource(const char *resource,
		const int state)
{
	idle_screen_view_update_resource(resource, state);
}

void
phoneui_backend_idle_screen_update_alarm(const int alarm)
{
	idle_screen_view_update_alarm(alarm);
}

void
phoneui_backend_idle_screen_update_profile(const char *profile)
{
	idle_screen_view_update_profile(profile);
}

