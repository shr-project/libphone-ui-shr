#include <glib.h>
#include "phoneui-shr.h"
#include "ui-utils.h"
#include "views.h"
#include "phoneui-sim-auth.h"
#include <sim-auth-input-view.h>


void
phoneui_backend_sim_auth_show(const int status)
{
	g_debug("phoneui_backend_sim_auth_show()");

	if (!sim_auth_input_view_is_init()) {
		if (sim_auth_input_view_init()) {
			return;
		}
	}
	sim_auth_input_view_show(status);
}

void
phoneui_backend_sim_auth_hide(const int status)
{
	(void) status;
	g_debug("phoneui_backend_sim_auth_hide()");
	sim_auth_input_view_hide();
}
