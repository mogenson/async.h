#include <stddef.h> // where NULL is defined

#ifndef __GNUC__
#error async.h requires GCC compiler
#endif

#pragma once
#pragma GCC diagnostic ignored "-Wunused-value"

/* Task types */
typedef enum { TASK_RESET, TASK_RUN } task_control_t;
typedef enum { TASK_DONE, TASK_RUNNING } task_status_t;
typedef struct {
  task_status_t status; // task execution status
  void *address;        // label address to resume at next run
  void *result;         // pointer to return result
} task_t;

/* Generates a function definition/declaration for a task name and optional
 * list of arguments. Assings a return type of task_t* and mandatory function
 * argument task_control_t task_control.
 *
 * Examples:
 *     ASYNC(task)
 *     ASYNC(task, int arg1, int arg2)
 */
#define ASYNC(task, ...)                                                       \
  task_t *task(task_control_t task_control __VA_OPT__(, ) __VA_ARGS__)

/* Runs a task by passing the TASK_RUN argument. Task will start execution at
 * beginning or previously yielded resume address if set. Task may yield a
 * pointer to a return result or else NULL.
 *
 * Examples:
 *     AWAIT(task, arg1, arg2)
 *     void *var = AWAIT(task, arg1, arg2)->result
 */
#define AWAIT(task, ...) task(TASK_RUN __VA_OPT__(, ) __VA_ARGS__)

/* Resets a task by passing the TASK_RESET task_control command. Task will start
 * execution at the beginning and yield at the TASK_BEGIN macro. Task may yield
 * a pointer to a return result or else NULL.
 *
 * Examples:
 *     RESET(task, arg1, arg2)
 *     void *var = RESET(task, arg1, arg2)->result
 */
#define RESET(task, ...) task(TASK_RESET __VA_OPT__(, ) __VA_ARGS__)

/* Drive a task to completion and return a pointer to the task_t struct.
 *
 * Examples:
 *     BLOCK(task, arg1, arg2)
 *     void *var = BLOCK(task, arg1, arg2)->result
 */
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
 * A pointer to a return result can be provided as an optional argument. This
 * result is only returned if the task is run with the TASK_RESET task_control
 * command.
 *
 * Examples:
 *     TASK_BEGIN()
 *     TASK_BEGIN(&var) where var is a static variable
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
 * status. This macro must be included at the end of a task. A pointer to a
 * return result can be provided as an optional argument.
 *
 * Examples:
 *     TASK_END()
 *     TASK_END(&var) where var is a static variable
 */
#define TASK_END(...)                                                          \
  do {                                                                         \
    task.status = TASK_DONE;                                                   \
    task.address = &&LABEL(TASK, __LINE__);                                    \
    task.result = (NULL __VA_OPT__(, ) __VA_ARGS__);                           \
    LABEL(TASK, __LINE__) : return &task;                                      \
  } while (0)

/* Exits the current task with the TASK_RUNNING status after setting the address
 * to the current line. Uses the LABEL() and CONCAT() helper macros to create a
 * unique label, then employs GCC's unary operator to get the address for that
 * label. The task will resume from this point the text time it is invoked with
 * the TASK_RUN() macro. A pointer to a return result can be provided as an
 * optional argument.
 *
 * Examples:
 *     YIELD()
 *     YIELD(&var) where var is a static variable
 */
#define YIELD(...)                                                             \
  do {                                                                         \
    task.address = &&LABEL(TASK, __LINE__);                                    \
    task.result = (NULL __VA_OPT__(, ) __VA_ARGS__);                           \
    return &task;                                                              \
    LABEL(TASK, __LINE__) :;                                                   \
  } while (0)

/* Yield task execution until condition evaluates to true. An optional return
 * result pointer can be provided as a second argument. Note: this will yield
 * once before condition is evaluated.
 *
 * Examples:
 *     YIELD_UNTIL(bytes_available() > 0) for function int bytes_available()
 *     YIELD_UNTIL(bytes_available() > 0, &var) where var is a static variable
 */
#define YIELD_UNTIL(condition, ...)                                            \
  do {                                                                         \
    YIELD(__VA_ARGS__);                                                        \
  } while (!(condition))

/* Yield task execution for a duration. Yields until the number of ticks
 * elapsed is greater than duration. Requires TICK_FUNC() and TICK_TYPE to be
 * defined. An optional return result pointer can be provided as a second
 * argument.
 *
 * Examples:
 *     YIELD_FOR(100) to delay for 100 ticks
 *     YIELD_FOR(100, &var) where var is a static variable
 */
#define YIELD_FOR(duration, ...)                                               \
  do {                                                                         \
    static TICK_TYPE LABEL(start, __LINE__) = 0;                               \
    LABEL(start, __LINE__) = TICK_FUNC();                                      \
    YIELD_UNTIL((TICK_FUNC() - LABEL(start, __LINE__) >= duration)             \
                    __VA_OPT__(, ) __VA_ARGS__);                               \
  } while (0)

/* The YIELD_FOR() macro above requires a reference clock in the form of an
 * incremental counter for time comparison. Define TICK_FUNC() as the calling
 * signature for this function and TICK_TYPE as the return type. TICK_TYPE can
 * be any type that supports comparison operators. Rollovers for unsigned
 * integers are handled. Below is a sample implementation for UNIX systems.
 */
#if !defined(TICK_FUNC) && !defined(TICK_TYPE) && defined(__unix__)
#include <time.h>
#define TICK_FUNC() time(NULL) // get seconds since January 1, 1970
#define TICK_TYPE time_t       // time_t is an integer
#else
#warning "You must define a TICK_FUNC() function and TICK_TYPE return type"
#endif
