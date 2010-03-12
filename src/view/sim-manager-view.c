#include "sim-manager-view.h"
#include "ui-utils.h"

struct SimManagerViewData {
	struct View view;
};
static struct SimManagerViewData view;

int
sim_manager_view_init()
{
	Evas_Object *win;
	int ret;
	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("SIM Manager"),
				NULL, NULL, NULL);

	if (ret) {
		g_critical("Failed to init sim manager view");
		return ret;
	}

	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);

	ui_utils_view_layout_set(VIEW_PTR(view), DEFAULT_THEME,
				 "phoneui/settings/sim-manager");

	return 0;
}

int
sim_manager_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

void
sim_manager_view_deinit()
{
	ui_utils_view_deinit(VIEW_PTR(view));
}

void
sim_manager_view_show()
{
	ui_utils_view_show(VIEW_PTR(view));
}

void
sim_manager_view_hide()
{
	ui_utils_view_hide(VIEW_PTR(view));
}
