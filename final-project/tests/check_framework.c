#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "framework.h"
#include "services.h"

static struct Event received[4096];
static uint32_t receivedCount;
static uint32_t postFromService;

uint32_t readTimer2(void) { return 1234; }

void runService(struct Event event)
{
    assert(receivedCount < 4096);
    received[receivedCount++] = event;
    if(postFromService != 0)
    {
        postFromService = 0;
        assert(postEvent(PUMP_COMPLETE, 99));
    }
}

int main(void)
{
    initFramework();
    serviceEventQueue();
    assert(receivedCount == 0);
    uint32_t next = 0;
    // Alternate enqueue/dequeue sizes to exercise both queue indices wrapping.
    for(uint32_t cycle = 0; cycle < 100; cycle++)
    {
        for(uint32_t i = 0; i < 13; i++) assert(postEvent(HAND_CHANGED, next++));
        uint32_t before = receivedCount;
        for(uint32_t i = 0; i < 13; i++) serviceEventQueue();
        assert(receivedCount == before + 13);
        for(uint32_t i = before; i < receivedCount; i++)
        {
            assert(received[i].type == HAND_CHANGED && received[i].value == i);
            assert(received[i].time == 1234);
        }
    }
    uint32_t before = receivedCount;
    postFromService = 1;
    assert(postEvent(LEVER_REACHED_MINIMUM, 0));
    serviceEventQueue();
    assert(receivedCount == before + 1); // Newly posted event waits for a later call.
    serviceEventQueue();
    assert(receivedCount == before + 2 && received[before + 1].type == PUMP_COMPLETE);
    assert(received[before + 1].value == 99);
    for(uint32_t i = 0; i < EVENT_QUEUE_SIZE; i++) assert(postEvent(HAND_CHANGED, i));
    assert(!postEvent(HAND_CHANGED, 999));
    before = receivedCount;
    serviceEventQueue();
    assert(receivedCount == before + 1 && received[before].type == QUEUE_OVERFLOW);
    serviceEventQueue(); assert(receivedCount == before + 1);
    assert(postEvent(HAND_CHANGED, 55)); serviceEventQueue();
    assert(received[before + 1].value == 55);
    assert(postEvent(PUMP_COMPLETE, 1)); clearEventQueue();
    before = receivedCount; serviceEventQueue(); assert(receivedCount == before);
    puts("PASS: FIFO order, queue wraparound, one event per call, service-posted events and overflow recovery.");
}
