Multitasking in Small Embedded Systems

- FreeRTOS is a collection of C libraries comprised of a real-time kernel and a set of modular libraries that
implement complementary functionality.

- The FreeRTOS kernel is ideally suited to deeply embedded real-time applications that run on microcontrollers
or small microprocessors. This type of application typically includes a mix of both hard and soft real-time
requirements


- Soft real-time requirements state a time deadline—but breaching the deadline would not render the system
useless. For example, responding to keystrokes too slowly might make a system seem annoyingly unresponsive
without actually making it unusable.
- Hard real-time requirements state a time deadline—and breaching the deadline would result in absolute
failure of the system. For example, a driver's airbag has the potential to do more harm than good if it
responded to crash sensor inputs too slowly.

- The FreeRTOS kernel is a real-time kernel (or real-time scheduler) that enables applications built on FreeRTOS
to meet their hard real-time requirements. It enables applications to be organized as a collection of
independent threads of execution. For example, on a processor that has only one core, only a single thread of
execution can execute at any one time. The kernel decides which thread to execute by examining the priority
assigned to each thread by the application designer. In the simplest case, the application designer could assign
higher priorities to threads that implement hard real-time requirements and lower priorities to threads that
implement soft real-time requirements. Allocating priorities in that way would ensure hard real-time threads
always execute ahead of soft real-time threads, but priority assignment decisions are not always that simplistic

- task prioritization can help ensure an application meets its processing deadlines, but a
kernel can bring other less obvious benefits. Some of these are listed very briefly below.
  - Abstracting away timing information
The RTOS is responsible for execution timing and provides a time-related API to the application. That
allows the structure of the application code to be more straightforward and the overall code size to be
smaller.
  - Maintainability/Extensibility
Abstracting away timing details results in fewer interdependencies between modules and allows the
software to evolve in a controlled and predictable way. Also, the kernel is responsible for the timing, so
application performance is less susceptible to changes in the underlying hardware.
  - Modularity
Tasks are independent modules, each of which should have a well-defined purpose.
  - Team development
Tasks should also have well-defined interfaces, allowing easier team development.
  - Easier testing
Tasks that are well-defined independent modules with clean interfaces are easier to test in isolation.
  - Code reuse
Code designed with greater modularity and fewer interdependencies is easier to reuse.
  - Improved efficiency
Application code that uses an RTOS can be completely event-driven. No processing time needs to be
wasted by polling for events that have not occurred.
Countering the efficiency gained from being event driven is the need to process the RTOS tick interrupt
and switch execution from one task to another. However, applications that don't use an RTOS normally
include some form of tick interrupt anyway.
  - Idle time
The automatically created Idle task executes when there are no application tasks that require processing.
The Idle task can measure spare processing capacity, perform background checks, or place the processor
into a low-power mode.
  - Power Management
The efficiency gains that result from using an RTOS allow the processor to spend more time in a low
power mode.
Power consumption can be decreased significantly by placing the processor into a low power state each
time the Idle task runs. FreeRTOS also has a special tick-less mode. Using the tick-less mode allows the
processor to enter a lower power mode than would otherwise be possible and remain in the low power
mode for longer.
  - Flexible interrupt handling
Interrupt handlers can be kept very short by deferring processing to either a task created by the
application writer or the automatically created RTOS daemon task (also known as the timer task).
  - Mixed processing requirements
Simple design patterns can achieve a mix of periodic, continuous, and event-driven processing within an
application. In addition, hard and soft real-time requirements can be met by selecting appropriate task
and interrupt priorities

### The FreeRTOS kernel has the following standard features:
- Pre-emptive or co-operative operation
- Optional time-slicing
- Very flexible task priority assignment
- Flexible, fast and light-weight task notification mechanisms
- Queues
- Binary semaphores
- Counting semaphores
- Mutexes
- Recursive mutexes
- Software timers
- Event groups
- Stream buffers
- Message buffers
- Co-routines (deprecated)
- Tick hook functions
- Idle hook functions
- Stack overflow checking
- Trace macros
- Task run-time statistics gathering
- Optional commercial licensing and support
- Full interrupt nesting model (for some architectures)
- A tick-less capability for extreme low power applications (for some architectures)
- Memory Protection Unit support for isolating tasks and increasing application safety (for some
- architectures)
- Software managed interrupt stack when appropriate (this can help save RAM)
- The ability to create RTOS objects using either statically or dynamically allocated memory
