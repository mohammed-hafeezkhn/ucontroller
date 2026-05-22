# Micro controllers:
---

ARM designs and licenses **CPU architectures and processor IP**.  
Companies like STM, TI, NXP etc. take these ARM cores and integrate them into their chips (MCUs, SoCs, processors) and get them manufactured.

ARM has different classes of CPUs:

## 1) ARM - A Class CPUs (Application Processors)

Application-grade processors capable of running rich operating systems such as:

- Linux
- Android
- Windows

They can run full applications like:

- Chrome browser
- Photoshop
- Multimedia applications
- AI / high-performance workloads

Examples:

- Cortex-A53
- Cortex-A72
- Cortex-A78

Used in:

- Phones
- Tablets
- Laptops
- Infotainment systems

---

## 2) ARM - R Class CPUs (Real-Time Processors)

Used in systems requiring **determinism**.

Meaning:

The CPU must respond within a **definite time window**.

Examples:

- Engine control systems
- ABS systems
- Industrial control
- Storage controllers
- Safety applications

Focus:

- Predictable latency
- Real-time response
- Reliability

Examples:

- Cortex-R4
- Cortex-R5
- Cortex-R52

---

## 3) ARM - M Class CPUs (Microcontrollers)

Used for microcontroller-based applications.

Anything that performs a **single focused task**, for example:

- Microwave oven
- Car window control
- Toaster
- Washing machine
- Power management subsystem
- Bluetooth controller
- Sensors
- IoT devices

Examples:

- Cortex-M0
- Cortex-M3
- Cortex-M4
- Cortex-M7

These are usually low power and cost optimized.

---

# Architecture vs Microarchitecture

## Architecture

Architecture = specification describing:

- What CPU should do
- Instruction behavior
- Registers
- Memory handling
- Exception handling

It defines **how software sees the CPU**.

Example:

ARMv6  
ARMv7  
ARMv8

---

## Microarchitecture

Microarchitecture = actual implementation of architecture.

It defines:

- Pipeline design
- Cache structure
- Execution units
- Internal optimizations

Two CPUs may implement the same architecture but have different microarchitectures.

Example:

Cortex-M3 and Cortex-M4 both implement ARM architecture but internally differ.

---

# Cortex Evolution

Examples:

- Cortex-M0
- Cortex-M3
- Cortex-M4

ARM architectures are generally **backward compatible**.

Usually code written for older versions can run on newer versions, although newer architectures may introduce additional instructions and features.

Example:

ARMv6 → ARMv7 → ARMv8

---

# RISC Architecture

ARM follows **RISC (Reduced Instruction Set Computing)** principles.

It mainly uses a **load-store architecture**.

Meaning:

Any computation requires:

1. Fetch data from memory into CPU registers
2. Perform operation inside CPU
3. Store result back into memory

Example flow:

Memory → Register → Compute → Memory

---

# CPU Registers

CPU contains:

## General Purpose Registers (GPR)

Used for:

- Temporary variables
- Arithmetic operations
- Address calculations

---

## Status Registers

Contain state information.

Examples:

- Overflow flag
- Carry flag
- Zero flag

Example:

```text
Addition overflow → Overflow bit set
```

---

## Configuration Registers

Used to configure CPU operation.

Examples:

- Privilege mode selection
- Interrupt settings
- Execution modes

---

Registers are temporary storage units.

Internally they are built using storage elements (flip-flops).

Example:

1 flip-flop → stores 1 bit

32 flip-flops combined:

```text
32-bit register
```

---

# ARM uses Load-Store Architecture (RISC)

ARM CPUs mostly operate on registers.

Example:

```text
LOAD  Memory → Register

ADD   Register + Register

STORE Register → Memory
```

CPU generally does not directly compute on memory contents.

---

# Understanding CPU Thinking

To understand how CPU behaves:

## 1) Programmer’s Model

Describes:

- CPU operating modes
- Register set
- Program visible behavior
 #### [Programmers_guide](https://community.arm.com/cfs-file/__key/telligent-evolution-components-attachments/01-2142-00-00-00-00-52-96/White-Paper-_2D00_-Cortex_2D00_M-for-Beginners-_2D00_-2016-_2800_final-v3_2900_.pdf) 


## General Purpose Registers (GPR)

Cortex-M CPUs expose **16 programmer visible registers**.

```text
R0  - R12  → General purpose registers
R13        → Stack Pointer (SP)
R14        → Link Register (LR)
R15        → Program Counter (PC)
```

R0–R12 are used for:

- Arithmetic operations
- Temporary variables
- Address calculations
- Function arguments
- Intermediate results

---

## Special Purpose Registers (SPR)

Special registers control CPU behavior and capture status information.

Used for:

- Setting privileges
- Selecting operating modes
- Selecting active stack
- Capturing CPU state
- Exception handling

Examples:

- xPSR
- CONTROL register
- Interrupt mask registers

---

# Stack Pointer Register (R13)

R13 acts as **Stack Pointer (SP)**.

Cortex-M supports two stack pointers:

## 1) MSP (Main Stack Pointer)

```text
R13 → MSP
```

Used:

- After CPU reset (default stack)
- Automatically during Handler mode
- Exception handling
- Interrupt servicing

---

## 2) PSP (Process Stack Pointer)

```text
R13 → PSP
```

Used mainly for:

- Application tasks
- User processes
- Thread execution in RTOS systems
- Unprivileged execution

Thread mode may use:

```text
MSP or PSP
```

Handler mode always uses:

```text
MSP
```

---

# Link Register (R14)

```text
R14 → LR
```

Link Register remembers:

```text
Where CPU should return after function call finishes
```

Example:

```c
foo()
{
   bar();
}
```

When CPU enters:

```text
bar()
```

Return address gets stored in:

```text
LR (R14)
```

After completion:

```text
CPU returns back
```

LR is also used during exception entry and return.

---

# Program Counter (R15)

```text
R15 → PC
```

Program Counter stores:

```text
Address of next instruction
```

CPU executes instructions by updating PC continuously.

Example:

```text
PC = 0x1000

Instruction fetch

PC = 0x1004

Next fetch
```

---

# xPSR (Program Status Register)

xPSR combines several status fields.

Contains:

## Status Flags

- N → Negative flag
- Z → Zero flag
- C → Carry flag
- V → Overflow flag

Example:

```text
Large addition overflow

V = 1
```

---

## Exception Information

Stores:

```text
Current exception number
```

CPU knows:

- Which interrupt is running
- Which fault occurred

---

## Execution State Information

Contains processor execution state information.

xPSR is internally formed from:

```text
APSR + IPSR + EPSR
```

---

# Interrupt Mask Registers

Used to control interrupt handling priority.

Examples:

## PRIMASK

Can disable most interrupts.

---

## BASEPRI

Allows blocking interrupts below a priority level.

---

## FAULTMASK

Masks almost all exceptions except very critical ones.

These registers decide:

```text
Which interrupt CPU should attend
```

and

```text
Which interrupt should wait
```

---

# CONTROL Register

Used for:

1. Selecting privilege level

```text
Privileged
Unprivileged
```

2. Selecting active stack

```text
MSP
PSP
```

---

# CPU Modes & Privileges

Cortex-M mainly operates in two modes.

---

## 1) Thread Mode

Used for:

- Application code
- Computation
- Main program execution

Can run as:

```text
Privileged
or
Unprivileged
```

Examples:

```c
main()
{
   app_task();
}
```

Runs in:

```text
Thread mode
```

---

## 2) Handler Mode

Whenever exception occurs:

CPU automatically switches:

```text
Thread Mode
        ↓

Handler Mode
```

Examples:

- Interrupt
- Bus fault
- Memory fault
- Usage fault

Handler mode always executes as:

```text
Privileged
```

Only MSP is used here.

---

# Privileged vs Unprivileged Execution

## Privileged Code

Can access:

- Control registers
- Status registers
- System configuration
- Interrupt control
- MSP / PSP selection

---

## Unprivileged Code

Restricted access.

Usually uses:

```text
PSP
```

Sometimes called:

```text
Process stack
```

Cannot directly modify many system registers.

---

# Fault Handling Example

Suppose stack grows beyond valid memory:

```text
Stack overflow
```

or

CPU accesses invalid memory:

```text
Bad address access
```

Internal fault logic detects issue.

CPU raises exception:

Examples:

```text
BusFault
MemManage Fault
UsageFault
```

CPU then moves:

```text
Thread Mode
      ↓

Handler Mode
```

and executes fault handler.


---

## 2) Memory Model

Describes:

How CPU interacts with memory.

Examples:

- Address space
- Access ordering
- Memory regions

---

## 3) Exception Model

Defines:

How CPU handles:

- Interrupts
- Faults
- Exceptions

---

## 4) Debug Model

Extra circuitry inside CPU communicates with debugger hardware.

Allows:

- Reading registers
- Breakpoints
- Single stepping
- CPU control

Examples:

- JTAG
- SWD

---

# CPU Buses

CPU generally has two major buses:

1. Instruction Bus

Used to fetch instructions.

```text
Memory → CPU instruction fetch
```

2. Data Bus

Used for data transfer.

```text
Memory ↔ CPU data movement
```

Bus means:

Multiple wires grouped together.

It is **not a single signal line**.

---

# Interrupts

Interrupt = external hardware event.

CPU normally executes instructions sequentially:

```text
Instruction 1
Instruction 2
Instruction 3
```

Suppose external hardware raises signal:

Examples:

- Timer expired
- Button pressed
- CAN message arrived

CPU temporarily stops current flow and executes interrupt handler.

After completion:

CPU returns to previous execution.

---

# Exceptions

Suppose CPU tries:

```text
Read address = 0x12345678
```

But memory does not exist there.

Internal logic detects fault.

CPU generates exception.

Examples:

- Invalid memory access
- Bus fault
- Divide by zero
- Usage fault

---

# Interrupt vs Exception

Interrupt:

- Comes from outside CPU
- Hardware generated

Examples:

- GPIO event
- Timer
- UART receive

Exception:

- Happens inside CPU
- Generated due to internal condition

Examples:

- Invalid memory access
- Faults
- Illegal operation

Conceptually both are similar:

```text
Interrupt → External event

Exception → Internal event
```

CPU changes execution flow and handles them through dedicated handlers.

---

# M-Class CPU Boot Sequence

---

# Vector Table

Interrupts and exceptions in Cortex-M CPUs are **vectored**.

CPU maintains a structure called:

```text
Vector Table (VTable)
```

Each entry is usually:

```text
4 bytes
```

Each 4-byte entry stores:

```text
Address in memory
```

This address tells CPU:

```text
Where should CPU jump when an exception or interrupt occurs
```

So at every 4-byte boundary:

```text
+------------------+
| Address / Vector |
+------------------+
| Address / Vector |
+------------------+
| Address / Vector |
+------------------+
```

CPU uses these entries to decide execution flow.

---

# Vector Table Layout

## Offset 0x00000000

### MSP Initialization

First entry:

```text
VectorTable[0]
```

Stored value:

```text
Initial Main Stack Pointer value
```

This value becomes:

```text
R13 = MSP
```

It indicates:

```text
Where stack should start
```

Example:

```text
0x20020000
```

CPU loads:

```text
MSP = 0x20020000
```

---

## Offset 0x00000004

### Reset Vector

Second entry:

```text
VectorTable[1]
```

Contains:

```text
Address of Reset Handler
```

This tells CPU:

```text
Where instruction fetching should begin after reset
```

Example:

```text
0x08000100
```

CPU loads:

```text
PC = 0x08000100
```

Then execution starts.

---

# Exception Vectors

After reset vector, remaining entries define handlers for exceptions.

Examples:

```text
NMI
HardFault
MemManage Fault
BusFault
UsageFault
SVCall
PendSV
SysTick
```

Each entry stores:

```text
Handler address
```

CPU jumps there whenever that event occurs.

---

# Bus Fault Example

Suppose CPU accesses:

```text
0x90000000
```

But memory does not exist there.

Bus/interconnect responds:

```text
ERROR
```

CPU detects:

```text
BusFault
```

CPU then:

1. Finds BusFault vector entry

Example offset:

```text
0x00000014
```

(from vector table base)

2. Reads stored address

Example:

```text
0x08000200
```

3. Loads PC

```text
PC = 0x08000200
```

4. Starts fetching instructions there

Instructions present here act as:

```text
Recipe to handle BusFault
```

Examples:

- Log error
- Reset module
- Enter safe state
- Recover system

---

# Peripheral Interrupt Handling

Same mechanism applies for peripherals.

Examples:

```text
SysTick
DMA
UART
CAN
GPIO
Timer
```

Each peripheral interrupt is mapped to a specific vector entry.

Example flow:

```text
UART interrupt occurs

        ↓

CPU identifies UART vector

        ↓

Reads handler address

        ↓

Loads PC

        ↓

Starts executing UART ISR
```

---

# Boot-Up Sequence

When power-up or reset happens:

## Step 1

CPU reads:

```text
VectorTable[0]
```

Loads:

```text
R13 (MSP)
```

Example:

```text
MSP = Initial Stack Address
```

---

## Step 2

CPU reads:

```text
VectorTable[1]
```

Loads:

```text
PC = Reset Handler Address
```

---

## Step 3

CPU immediately starts instruction fetch.

Flow:

```text
Power ON / Reset

        ↓

Read Vector[0]

        ↓

Initialize MSP

        ↓

Read Vector[1]

        ↓

Load PC

        ↓

Start instruction fetch

        ↓

Execute Reset Handler

        ↓

System initialization

        ↓

main()
```

---

# Simplified View

```text
Vector Table

Offset      Purpose

0x00000000  Initial MSP value
0x00000004  Reset Handler
0x00000008  NMI Handler
0x0000000C  HardFault
0x00000010  MemManage
0x00000014  BusFault
0x00000018  UsageFault
...
...
SysTick
Peripheral IRQs
```

Every entry contains:

```text
Jump address
```

CPU simply reads address and transfers execution there.

---





