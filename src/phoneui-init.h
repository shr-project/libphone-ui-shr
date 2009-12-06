#ifndef PHONEUI_INIT_H
#define PHONEUI_INIT_H

void phoneui_backend_init(int argc, char **argv, int (*idle_cb) (void *));
void phoneui_backend_loop();

#endif
