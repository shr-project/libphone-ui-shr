#include <glib.h>
#include "phoneui-shr.h"
#include "ui-utils.h"
#include "views.h"
#include "phoneui-sim-auth.h"
#include <sim-auth-input-view.h>


void
phoneui_backend_sim_auth_show(const int status)
{
	// FIXME: remove status from the specs
	(void) status;

	if (!sim_auth_input_view_is_init()) {
		if (sim_auth_input_view_init()) {
			return;
		}
	}
	sim_auth_input_view_show();
}

void
phoneui_backend_sim_auth_hide(const int status)
{
	// FIXME: remove status from the specs
	(void) status;
	sim_auth_input_view_hide();
}
