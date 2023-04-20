#include "wdr/timer.h"
#include "v5_api.h"

/**
 * @brief Initializes a wdr_timer_t object
 * 
 * Starts timer in paused state by default
 * 
 * @param timer Timer object to initialize
 */
void wdrTimerInit(wdr_timer_t *timer)
{
  timer->prev_time_ms = vexSystemTimeGet();
  timer->elapsed_time_ms = 0;
  timer->pause = true;
}

/**
 * @brief Returns the time elapsed since starting
 * 
 * This function updates the elapsed_time_ms if the timer is not paused.
 * 
 * @param timer Timer object to read
 * @return uint32_t Elapsed time [ms]
 */
uint32_t wdrTimerGetTime(wdr_timer_t *timer)
{
  if (!timer->pause) {
    uint32_t current_time = vexSystemTimeGet();
    timer->elapsed_time_ms += current_time - timer->prev_time_ms;
    timer->prev_time_ms = current_time;
  }
  
  return timer->elapsed_time_ms;
}

/**
 * @brief Starts (i.e. unpauses) a timer
 * 
 * @param timer Timer object to start
 */
void wdrTimerStart(wdr_timer_t *timer)
{
  if (timer->pause) {
    // Update prev_time now that timer is unpaused
    timer->prev_time_ms = vexSystemTimeGet();
    timer->pause = false;
  }
}

/**
 * @brief Pauses a timer
 * 
 * @param timer Timer object to pause
 */
void wdrTimerPause(wdr_timer_t *timer)
{
  timer->pause = true;
}

/**
 * @brief Reset timer (elapsed time to zero, pause)
 * 
 * @param timer Timer object to reset
 */
void wdrTimerReset(wdr_timer_t *timer)
{
  wdrTimerInit(timer);
}



void wdrHighResTimerInit(wdr_highres_timer_t *timer) {
  timer->prev_time_us = vexSystemHighResTimeGet();
  timer->elapsed_time_us = 0;
  timer->pause = true;
}

uint64_t wdrHighResTimerGetTime(wdr_highres_timer_t *timer) {
  if (!timer->pause) {
    uint64_t current_time = vexSystemHighResTimeGet();
    timer->elapsed_time_us += current_time - timer->prev_time_us;
    timer->prev_time_us = current_time;
  }
  
  return timer->elapsed_time_us;
}

void wdrHighResTimerStart(wdr_highres_timer_t *timer) {
  if (timer->pause) {
    // Update prev_time now that timer is unpaused
    timer->prev_time_us = vexSystemHighResTimeGet();
    timer->pause = false;
  }
}

void wdrHighResTimerPause(wdr_highres_timer_t *timer) {
  timer->pause = true;
}

void wdrHighResTimerReset(wdr_highres_timer_t *timer) {
  wdrHighResTimerInit(timer);
}