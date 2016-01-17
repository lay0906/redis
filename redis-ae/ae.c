#define HAVE_EPOLL

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ae.h"

#include "ae_epoll.c"

aeEventLoop *aeCreateEventLoop(int setsize) {
    aeEventLoop *eventLoop;
    int i;

    if ((eventLoop = malloc(sizeof(*eventLoop))) == NULL) goto err;

    eventLoop->events = malloc(sizeof(aeFileEvent)*setsize);
    eventLoop->fired = malloc(sizeof(aeFiredEvent)*setsize);
    if (eventLoop->events == NULL || eventLoop->fired == NULL) goto err;
    eventLoop->setsize = setsize;

    eventLoop->stop = 0;
    eventLoop->maxfd = -1;
    if (aeApiCreate(eventLoop) == -1) goto err;
    for (i = 0; i < setsize; i++)
        eventLoop->events[i].mask = AE_NONE;
    return eventLoop;
err:
    if (eventLoop) {
        free(eventLoop->events);
        free(eventLoop->fired);
        free(eventLoop);
    }
    return NULL;
}

void aeDeleteEventLoop(aeEventLoop *eventLoop) {
    aeApiFree(eventLoop);
    free(eventLoop->events);
    free(eventLoop->fired);
    free(eventLoop);
}

void aeStop(aeEventLoop *eventLoop) {
    eventLoop->stop = 1;
}

int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
                      aeFileProc *proc, void *clientData)
{
    if (fd >= eventLoop->setsize) return AE_ERR;
    aeFileEvent *fe = &eventLoop->events[fd];

    if (aeApiAddEvent(eventLoop, fd, mask) == -1)
        return AE_ERR;

    fe->mask |= mask;
    if (mask & AE_READABLE) fe->rfileProc = proc;
    if (mask & AE_WRITABLE) fe->wfileProc = proc;

    fe->clientData = clientData;

    if (fd > eventLoop->maxfd)
        eventLoop->maxfd = fd;

    return AE_OK;
}

void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask)
{
    if (fd >= eventLoop->setsize) return;
    aeFileEvent *fe = &eventLoop->events[fd];

    if (fe->mask == AE_NONE) return;

    fe->mask = fe->mask & (~mask);
    if (fd == eventLoop->maxfd && fe->mask == AE_NONE) {
        int j;

        for (j = eventLoop->maxfd-1; j >= 0; j--)
            if (eventLoop->events[j].mask != AE_NONE) break;
        eventLoop->maxfd = j;
    }

    aeApiDelEvent(eventLoop, fd, mask);
}

int aeProcessEvents(aeEventLoop *eventLoop, int flags)
{
    int processed = 0, numevents;

    if (!(flags & AE_FILE_EVENTS)) return 0;

    if (eventLoop->maxfd != -1) {
            int j;
            numevents = aeApiPoll(eventLoop);
            for (j = 0; j < numevents; j++) {
                aeFileEvent *fe = &eventLoop->events[eventLoop->fired[j].fd];

                int mask = eventLoop->fired[j].mask;
                int fd = eventLoop->fired[j].fd;
                int rfired = 0;

                /* note the fe->mask & mask & ... code: maybe an already processed
                * event removed an element that fired and we still didn't
                * processed, so we check if the event is still valid. */
                if (fe->mask & mask & AE_READABLE) {
                    rfired = 1;
                    fe->rfileProc(eventLoop,fd,fe->clientData,mask);
                }
                if (fe->mask & mask & AE_WRITABLE) {
                    if (!rfired || fe->wfileProc != fe->rfileProc)
                        fe->wfileProc(eventLoop,fd,fe->clientData,mask);
                }

                processed++;
            }
    }
    return processed; /* return the number of processed file/time events */
}

void aeMain(aeEventLoop *eventLoop) {

    eventLoop->stop = 0;

    while (!eventLoop->stop) {
        aeProcessEvents(eventLoop, AE_FILE_EVENTS);
    }
}

