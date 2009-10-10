#ifndef PHONEGUI_INIT_H
#define PHONEGUI_INIT_H

void phonegui_backend_init(int argc, char **argv, int (*idle_cb) (void *));
void phonegui_backend_loop();

#endif
