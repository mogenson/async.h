#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#pragma once

/* look at
https://hackaday.com/2019/09/24/asynchronous-routines-for-c/
https://www.embeddedrelated.com/showarticle/455.php
https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
https://ideone.com/UJRVh3
 */

/* Task types */
typedef enum { TASK_RESET, TASK_RUN } task_control_t;
typedef enum { TASK_DONE, TASK_RUNNING } task_status_t;
typedef struct {
  task_status_t status; // execution status
  void *address;        // label address to resume at next run
  void *result;         // return value
} task_t;

// IDEA: add a return type to a task control struct
// then we can return stuff like a generator

/* Timer types */
typedef struct {
  uint32_t start;
  uint32_t duration;
} timeout_t;

#ifdef __unix__
#include <sys/time.h>
#include <unistd.h>
static inline uint32_t millis(void) {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#else
#warning "You must define your own millis() function"
#endif

static inline void timeout_set(timeout_t *timeout, uint32_t milliseconds) {
  if (!timeout)
    return;
  timeout->start = millis();
  timeout->duration = milliseconds;
}

static inline bool timeout_expired(timeout_t *timeout) {
  if (!timeout)
    return true; // default to timeout
  return (millis() - timeout->start) >= timeout->duration;
}

/* Generates a function definition/declaration given a task name and optional
 * list of arguments. Includes the function return type task_status_t and
 * mandatory function argument task_control_t task_control.
 *
 * Examples:
 *     TASK(foo()) expands to: task_status_t foo(task_control_t task_control)
 *     TASK(bar(int i, int j)) expands to:
 *         task_status_t bar(task_control_t task_control, int i, int j)
 */
#define ASYNC(task, ...)                                                       \
  task_t *task(task_control_t task_control __VA_OPT__(, ) __VA_ARGS__)

/* Runs a function by passing the TASK_RUN argument. Function will start
 * execution at the beginning or previously yielded resume address.
 */
#define AWAIT(task, ...) task(TASK_RUN __VA_OPT__(, ) __VA_ARGS__)

/* Resets a function by passing the TASK_RESET argument. Function will start
 * execution at the beginning and yield at the TASK_BEGIN macro.
 */
#define RESET(task, ...) task(TASK_RESET __VA_OPT__(, ) __VA_ARGS__)

/* drive task to finish */
#define BLOCK(task, ...)                                                       \
  ({                                                                           \
    task_t *t;                                                                 \
    while (t = task(TASK_RUN __VA_OPT__(, ) __VA_ARGS__),                      \
           t->status != TASK_DONE)                                             \
      continue;                                                                \
    t;                                                                         \
  })

/* Helper macros to generate a unique GCC label from a label string and line
 * number, then set the address to the label using GCC's unary operator.
 */
#define CONCAT(label, line) label##line
#define LABEL(label, line) CONCAT(label, line)

/* Creates and initializes the local address variable and evaluates the
 * task_control command. This macro must be included at the beginning of a task.
 */
#define TASK_BEGIN(...)                                                        \
  static task_t task = {                                                       \
      .status = TASK_RUNNING, .address = NULL, .result = NULL};                \
  do {                                                                         \
    switch (task_control) {                                                    \
    case TASK_RUN:                                                             \
      if (task.address)                                                        \
        goto *task.address;                                                    \
      break;                                                                   \
    case TASK_RESET:                                                           \
      task.status = TASK_RUNNING;                                              \
      task.address = NULL;                                                     \
      task.result = (NULL __VA_OPT__(, ) __VA_ARGS__);                         \
      return &task;                                                            \
      break;                                                                   \
    }                                                                          \
  } while (0)

/* Sets the address to the end of the task and returns the TASK_DONE
 * status. This macro must be included at the end of a task.
 */
#define TASK_END(...)                                                          \
  do {                                                                         \
    task.status = TASK_DONE;                                                   \
    task.address = &&LABEL(TASK, __LINE__);                                    \
    task.result = (NULL __VA_OPT__(, ) __VA_ARGS__);                           \
    LABEL(TASK, __LINE__) : return &task;                                      \
  } while (0)

/* Sets the address to the current line and exits the task with the
 * TASK_RUNNING status. Uses the LABEL() and CONCAT() helper macros to create a
 * unique label, then uses GCC's unary operator to get the address for that
 * label. The task will resume from this point the text time it is invoked with
 * the TASK_RUN() macro.
 */
#define YIELD(...)                                                             \
  do {                                                                         \
    task.address = &&LABEL(TASK, __LINE__);                                    \
    task.result = (NULL __VA_OPT__(, ) __VA_ARGS__);                           \
    return &task;                                                              \
    LABEL(TASK, __LINE__) :;                                                   \
  } while (0)

/* will yield at least once */
#define YIELD_UNTIL(condition)                                                 \
  do {                                                                         \
    YIELD();                                                                   \
  } while (!(condition))

#define YIELD_FOR(milliseconds)                                                \
  do {                                                                         \
    static timeout_t timeout;                                                  \
    timeout_set(&timeout, milliseconds);                                       \
    YIELD_UNTIL(timeout_expired(&timeout));                                    \
  } while (0)
