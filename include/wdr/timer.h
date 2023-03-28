/**
 * @file timer.h
 * @author your name (you@domain.com)
 * @brief Convenient timer object for interacting with vexSystemTimeGet()
 * @date 2022-12-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wdr_timer_states
{
  uint32_t prev_time_ms;
  uint32_t elapsed_time_ms;
  bool pause;
} wdr_timer_t;

// Initialize timer
void wdrTimerInit(wdr_timer_t *timer);

// Get time in ms
uint32_t wdrTimerGetTime(wdr_timer_t *timer);

// Start timer
void wdrTimerStart(wdr_timer_t *timer);

// Pause timer
void wdrTimerPause(wdr_timer_t *timer);

// Reset timer
void wdrTimerReset(wdr_timer_t *timer);


// TODO: High resolution timer?

#ifdef __cplusplus
}
#endif
#endif // TIMER_H
