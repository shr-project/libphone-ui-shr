#ifndef PHONEUI_CALL_H
#define PHONEUI_CALL_H

void phoneui_backend_incoming_call_show(const int id, const int status,
					 const char *number);
void phoneui_backend_incoming_call_hide(const int id);
void phoneui_backend_outgoing_call_show(const int id, const int status,
					 const char *number);
void phoneui_backend_outgoing_call_hide(const int id);

#endif
