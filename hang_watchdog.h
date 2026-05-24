#ifndef HANG_WATCHDOG_H
#define HANG_WATCHDOG_H

void InitHangWatchdog();
void DeinitHangWatchdog();
void TouchMainThreadHeartbeat();

#endif
