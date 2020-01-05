# Async/await macro routines for C/C++

## Description

This project contains a single header file: `async.h` to be included in a C/C++ project. It enables cooperative multitasking via the `async`/`await` keywords popularized by modern languages. A function declared using the `async` macro is a task (coroutine) that can yield before the end of the function. This can be used to wait on a long-running job (such as blocking IO), return a chunk of an incremental calculation (like a generator), or simplify a state machine into a linear progression (like a message parser). An `async` function can be `awaited`, meaning it will resume running from the previous yield point and will exit at the next yield point. There is no scheduler and no separate stacks. Tasks cannot be preemptively interrupted and local variables must be declared `static` to retain their values.

The `async.h` header contains some inline timer functions needed by the `YIELD_FOR()` macro. These can be used for generic non-blocking timeout calculations. Non-unix platforms (such as embedded microcontrollers) will need to supply a function to count the number of milliseconds since boot.

These macros use a couple of interesting C preprocessor and C language features, including [the comma operator](https://www.gnu.org/software/gnu-c-manual/gnu-c-manual.html#The-Comma-Operator), [varadic macros](https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html), [statement expressions](https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html), and [labels as values](https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html). Some of these features are exclusive to GCC, so this library will not work with other compilers.

## API

```
/* Generates a function definition/declaration for a task name and optional
 * list of arguments. Assigns a return type of task_t* and mandatory function
 * argument task_control_t task_control.
 *
 * Examples:
 *     ASYNC(task)
 *     ASYNC(task, int arg1, int arg2)
 */
#define ASYNC(task, ...)
```

```
/* Runs a task by passing the TASK_RUN argument. Task will start execution at
 * beginning or previously yielded resume address if set. Task may yield a
 * pointer to a return result or else NULL.
 *
 * Examples:
 *     AWAIT(task, arg1, arg2)
 *     void *var = AWAIT(task, arg1, arg2)->result
 */
#define AWAIT(task, ...)
```

```
/* Resets a task by passing the TASK_RESET task_control command. Task will start
 * execution at the beginning and yield at the TASK_BEGIN macro. Task may yield
 * a pointer to a return result or else NULL.
 *
 * Examples:
 *     RESET(task, arg1, arg2)
 *     void *var = RESET(task, arg1, arg2)->result
 */
#define RESET(task, ...)
```

```
/* Drive a task to completion and return a pointer to the task_t struct.
 *
 * Examples:
 *     BLOCK(task, arg1, arg2)
 *     void *var = BLOCK(task, arg1, arg2)->result
 */
#define BLOCK(task, ...)
```

```
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
#define TASK_BEGIN(...)
```

```
/* Sets the address to the end of the task and returns the TASK_DONE
 * status. This macro must be included at the end of a task. A pointer to a
 * return result can be provided as an optional argument.
 *
 * Examples:
 *     TASK_END()
 *     TASK_END(&var) where var is a static variable
 */
#define TASK_END(...)
```

```
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
#define YIELD(...)
```

```
/* Yield task execution until condition evaluates to true. An optional return
 * result pointer can be provided as a second argument. Note: this will yield
 * once before condition is evaluated.
 *
 * Examples:
 *     YIELD_UNTIL(bytes_available() > 0) for function int bytes_available()
 *     YIELD_UNTIL(bytes_available() > 0, &var) where var is a static variable
 */
#define YIELD_UNTIL(condition, ...)
```

```
/* Yield task execution for a number of milliseconds. Yields until a timeout
 * timer has expired. An optional return result pointer can be provided as a
 * second argument.
 *
 * Examples:
 *     YIELD_FOR(1000) to delay for 1 second
 *     YIELD_FOR(1000, &var) where var is a static variable
 */
#define YIELD_FOR(milliseconds, ...)
```

## Examples

Examples can be found in the `examples` directory. Navigate to that directory and run `make` to build all examples.

### [arguments.c](https://github.com/mogenson/async.h/blob/master/examples/arguments.c)

Shows how to pass arguments to an async function. The task prints out the value of the arguments it receives each run.

### [generator.c](https://github.com/mogenson/async.h/blob/master/examples/generator.c)

Shows how to yield a value from an async function. The generator task yields an incrementing count up to `max`, similar to Python's `range()` function. The `while` loop in `main()` `awaits` the task until it is exhausted.

### [factorial.c](https://github.com/mogenson/async.h/blob/master/examples/factorial.c)

Shows the use of the `BLOCK()` macro to run a task to completion and retrieve a result. An `async` function calculates the factorial of a provided argument, in a piece-wise fashion.

### [nested.c](https://github.com/mogenson/async.h/blob/master/examples/nested.c)

Shows that tasks can be nested and an `async` function can be called from another `async` function. There are different strategies for scheduling the execution of multiple tasks and monitoring their progress.

### [delay.c](https://github.com/mogenson/async.h/blob/master/examples/delay.c)

Shows the use of the `YIELD_FOR()` macro to wait for a number of milliseconds while allowing other tasks to run.

### [spsc.c](https://github.com/mogenson/async.h/blob/master/examples/spsc.c)

A single-producer-single-consumer example where a producer task sets the value of a shared item and a consumer task waits for the next item. Shows the user of the `YIELD_UNTIL()` macro. Since this multi-tasking is single-threaded and cooperative, a mutex is not required and there is no risk of deadlock.

### [password.c](https://github.com/mogenson/async.h/blob/master/examples/password.c)

A stateful password parser example. Shows how to wait for a blocking IO operation (input from `stdin`) and parse a message piecewise without a full state machine. An OS call is made to enable non-blocking reads from `stdin`.

---
This project is inspired by the protothreads library from Contiki OS and Daniel Ozick's task macros.
