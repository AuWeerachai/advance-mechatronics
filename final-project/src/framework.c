#include "framework.h"
#include "services.h"
#include "stm32f446xx_timer_mort.h"

static struct Event eventQueue[EVENT_QUEUE_SIZE];
static uint32_t readIndex;
static uint32_t writeIndex;
static uint32_t eventCount;
static uint32_t queueOverflow;

void clearEventQueue(void)
{
    readIndex = 0;
    writeIndex = 0;
    eventCount = 0;
}

void initFramework(void)
{
    clearEventQueue();
    queueOverflow = 0;
}

uint32_t postEvent(enum EventType type, uint32_t value)
{
    // All producers run in the main loop; no interrupt modifies this queue.
    if(eventCount == EVENT_QUEUE_SIZE)
    {
        queueOverflow = 1;
        return 0;
    }

    eventQueue[writeIndex].type = type;
    eventQueue[writeIndex].value = value;
    eventQueue[writeIndex].time = readTimer2();
    writeIndex = writeIndex + 1;
    if(writeIndex == EVENT_QUEUE_SIZE)
    {
        writeIndex = 0;
    }
    eventCount = eventCount + 1;
    return 1;
}

void serviceEventQueue(void)
{
    struct Event event;

    if(queueOverflow != 0)
    {
        // Do not continue a game after losing its input sequence.
        clearEventQueue();
        queueOverflow = 0;
        event.type = QUEUE_OVERFLOW;
        event.value = 0;
        event.time = readTimer2();
        runService(event);
        return;
    }
    if(eventCount == 0)
    {
        return;
    }

    event = eventQueue[readIndex];
    readIndex = readIndex + 1;
    if(readIndex == EVENT_QUEUE_SIZE)
    {
        readIndex = 0;
    }
    eventCount = eventCount - 1;

    // Handle one event and return so inputs are checked again.
    runService(event);
}
