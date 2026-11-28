#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "events.h"

#define EVENT_QUEUE_SIZE ((uint32_t)32)

void initFramework(void);
// Returns 1 if stored. A full queue reports QUEUE_OVERFLOW to the service.
uint32_t postEvent(enum EventType type, uint32_t value);
void clearEventQueue(void);
void serviceEventQueue(void);

#endif
