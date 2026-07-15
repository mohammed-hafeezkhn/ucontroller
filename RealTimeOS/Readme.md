# RTOS

1.  What is an RTOS, and why is it used in embedded systems?
- Answer: An RTOS provides deterministic scheduling, tasks, synchronization, timers, and IPC. It is used when timing, concurrency, and separation of
responsibilities are difficult to manage in a single super-loop. In production ECU software I keep ISR paths short, push work to tasks, budget
stack/latency, and verify behavior with traces under worst-case bus and CPU load. A good interview answer should include one concrete example, one
common bug, and one verification method.

2. What is the difference between an RTOS and a general-purpose OS?
- Answer: An RTOS provides deterministic scheduling, tasks, synchronization, timers, and IPC. It is used when timing, concurrency, and separation of
responsibilities are difficult to manage in a single super-loop. In production ECU software I keep ISR paths short, push work to tasks, budget
stack/latency, and verify behavior with traces under worst-case bus and CPU load. A good interview answer should include one concrete example, one
common bug, and one verification method.
3. What is a task in an RTOS?
- Answer: An RTOS provides deterministic scheduling, tasks, synchronization, timers, and IPC. It is used when timing, concurrency, and separation of
responsibilities are difficult to manage in a single super-loop. In production ECU software I keep ISR paths short, push work to tasks, budget
stack/latency, and verify behavior with traces under worst-case bus and CPU load. A good interview answer should include one concrete example, one
common bug, and one verification method.

- >> [FreeRTOS](./FreeRTOS/FreeRTOS.md)
- >> [ZephyrRTOS](./Zephyr)
- >> [qnx](./other/NeutrinoRTOS.md)
- >> [vxw](./other/VxWorks.md)