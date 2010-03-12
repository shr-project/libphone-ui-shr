
#include "phoneui-sim-manager.h"
#include "view/sim-manager-view.h


void
phoneui_backend_sim_manager_show()
{
	if (!sim_manager_view_is_init()) {
		if (sim_manager_view_init()) {
			return;
		}
	}
	sim_manager_view_show();
}

void
phoneui_backend_sim_manager_hide()
{
	sim_manager_view_hide();
}
