#include <stdbool.h>
#include <stdint.h>

/* look at
https://hackaday.com/2019/09/24/asynchronous-routines-for-c/
https://www.embeddedrelated.com/showarticle/455.php
https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
https://ideone.com/UJRVh3
 */

/* Task types */
typedef enum { TASK_RESET, TASK_RUN } task_control_t;
typedef enum { TASK_DONE, TASK_RUNNING } task_status_t;


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
#define TASK(task, ...)                                                        \
  task_status_t task(task_control_t task_control __VA_OPT__(, ) __VA_ARGS__)

/* Runs a function by passing the TASK_RUN argument. Function will start
 * execution at the beginning or previously yielded resume address.
 */
#define RUN(task, ...) task(TASK_RUN __VA_OPT__(, ) __VA_ARGS__)

/* Resets a function by passing the TASK_RESET argument. Function will start
 * execution at the beginning and yield at the TASK_BEGIN macro.
 */
#define RESET(task, ...) task(TASK_RESET __VA_OPT__(, ) __VA_ARGS__)

/* drive task to finish */
#define AWAIT(task, ...)                                                       \
  do {                                                                         \
  } while (task(TASK_RUN __VA_OPT__(, ) __VA_ARGS__) != TASK_DONE)

/* Helper macros to generate a unique GCC label from a label string and line
 * number, then set the resume_address to the label using GCC's unary operator.
 */
#define CONCAT(label, line) label##line
#define LABEL(label, line) CONCAT(label, line)
#define TASK_SET_RESUME_ADDRESS()                                              \
  do {                                                                         \
    LABEL(TASK, __LINE__) : resume_address = &&LABEL(TASK, __LINE__);          \
  } while (0)

/* Creates and initializes the local resume_address variable and evaluates the
 * task_control command. This macro must be included at the beginning of a task.
 */
#define TASK_BEGIN()                                                           \
  static void *resume_address = 0;                                             \
  do {                                                                         \
    if (task_control == TASK_RESET)                                            \
      return resume_address = 0, TASK_RUNNING;                                 \
    if (task_control == TASK_RUN && resume_address)                            \
      goto *resume_address;                                                    \
  } while (0)

/* Sets the resume_address to the end of the task and returns the TASK_DONE
 * status. This macro must be included at the end of a task.
 */
#define TASK_END()                                                             \
  do {                                                                         \
    resume_address = &&LABEL(TASK, __LINE__);                                  \
    LABEL(TASK, __LINE__) : return TASK_DONE;                                  \
  } while (0)

/* Sets the resume_address to the current line and exits the task with the
 * TASK_RUNNING status. Uses the LABEL() and CONCAT() helper macros to create a
 * unique label, then uses GCC's unary operator to get the address for that
 * label. The task will resume from this point the text time it is invoked with
 * the TASK_RUN() macro.
 *//* optional boolean arguement, defaults to true */
#define TASK_YIELD(...)                                                        \
  do {                                                                         \
    if (1 __VA_OPT__(, ) __VA_ARGS__) {                                        \
      resume_address = &&LABEL(TASK, __LINE__);                                \
      return TASK_RUNNING;                                                     \
      LABEL(TASK, __LINE__) :;                                                 \
    }                                                                          \
  } while (0)

#define TASK_YIELD_UNTIL(condition)                                            \
  if (!(condition)) {                                                          \
    TASK_YIELD();                                                              \
  }

/* note this will yield at least once */
#define TASK_YIELD_UNTIL(condition)                                            \
  do {                                                                         \
    TASK_YIELD();                                                              \
  } while (!(condition))

#define TASK_YIELD_FOR(milliseconds)                                           \
  do {                                                                         \
    static timeout_t timeout;                                                  \
    timeout_set(&timeout, milliseconds);                                       \
    TASK_YIELD_UNTIL(timeout_expired(&timeout));                               \
  } while (0)
