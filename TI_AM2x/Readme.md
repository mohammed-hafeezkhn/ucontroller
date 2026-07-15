# AM2x Sitara

---

### Data Sheet:
* Designing the physical Printed Circuit Board (PCB).
* Routing power, checking voltage limits, or planning power-up sequences.
* Calculating thermal dissipation for heatsinks.
* Checking the physical pin mappings and hardware feature limits (e.g., clock speeds of the Cortex-R5F cores).

### Technical Reference Manual (TRM):
* Writing Embedded Software code, firmware, or configuring an RTOS.
* Setting up the bootloader or initialization sequence.
* Configuring internal peripherals (like Ethernet MACs, timers, or the PRU-ICSS).
* Looking up the exact hex address of a hardware register to flip a specific bit.
---
## Quick Insights: AM2x Architecture

The AM2x family is purpose-built for **advanced motor control** and **digital power control** applications. To achieve ultra-low latency and deterministic performance, the System-on-Chip (SoC) architecture integrates several specialized subsystems:

*   **Cortex-R5F MCUs:** High-performance processing cores tailored for real-time execution.
*   **Real-Time Control Subsystem (CONTROLSS):** Dedicated hardware designed specifically for precision control loops.
*   **Hardware Security Module (HSM):** An isolated on-chip enclave for secure boot, cryptography, and system protection.
*   **Programmable Real-Time Unit Subsystem (PRU-ICSS):** Sitara’s signature programmable I/O subsystem for custom, ultra-low-latency interfaces and industrial protocols.
---

## Booting Process

The AM2x utilizes a secure, multi-stage boot sequence to ensure system integrity before handing control over to the main application. The process follows three primary phases:

### 1. Hardware Startup
The foundation of the boot process. The hardware executes the required power, clock, and reset ramp sequence to stabilize the SoC.

### 2. ROM Bootloader (RBL) Phase
During this phase, the primary goal is to securely load and execute the Secondary Bootloader (SBL). This involves a dual-core approach:
*   **R5F0 Core (Main):** Executes the ROM code to identify the boot interface, download the SBL software, and prepare it for execution.
*   **M4 Core (Secure Island):** Executes the Hardware Security Module (HSM) ROM code. It performs strict image integrity and authentication checks, acting as the gatekeeper that either allows or forbids the SBL from executing.

### 3. Secondary Bootloader (SBL) & Runtime Phase
Once authenticated, the SBL takes over to prepare the environment for the final application:
*   **SBL Execution:** The primary software responsible for configuring the SoC. It loads the application software into memory and passes control to it.
*   **HSM RunTime (TIFS-MCU):** Firmware running on the secure M4 Cortex core. It provides ongoing security services to the application as needed.
*   **R5F Runtime (Application):** The actual system payload, running either an RTOS (like FreeRTOS) or a bare-metal application on the main processor(s).

> **🛡️ Security Highlight:** The HSM on the AM263x provides continuous cryptographic acceleration, manages secure boot enforcement, and handles granular firewalls, enabling developers to design robust, highly secure industrial systems.

## ROM Code Overview

The ROM bootloader is a multi-core software embedded in the on-chip read-only memory. Its primary job is to assist in securely transferring and executing your Secondary Bootloader (SBL) and application code. 

The system relies on two distinct ROM codes operating in tandem:
* **Public ROM:** Runs on the main **R5F core**.
* **HSM ROM:** Runs on the secure **M4 core**.

### Core Startup & Inter-Processor Communication (IPC)
The ROM startup sequence is tightly coordinated between the two cores:
1. After the power-on sequence (PORz/RSTz), the **HSM ROM** starts first. It assumes the R5 core is out of reset but halted.
2. The HSM clears the `R5SS0_COREA_HALT` register to un-halt and wake up the R5 core.
3. The R5 and HSM establish communication using a dedicated **Mailbox RAM**, utilizing interrupt-based Write, Read, and ACK signals to coordinate the boot process.

### Boot Modes
The exact boot behavior is dictated by physical **BOOTMODE pins**, which are sampled automatically after the reset release. The ROM reads these pins to configure the necessary PLLs and peripherals. Boot modes fall into two main categories:

* **Host Boot Modes (e.g., UART, USB DFU):** The device waits to receive the application code from an external host computer and stores it directly into internal L2 memory.
* **Memory Boot Modes (e.g., QSPI/OSPI):** The device actively pulls the code from external non-volatile flash memory and transfers it into internal L2 memory for execution.

### The Two Phases of ROM Operation
Regardless of the boot mode selected by the pins, the ROM operation always executes in two distinct phases:
1. **Hardware Initialization Phase:** The ROM configures device resources (PLLs, pins, and specific peripherals) strictly as needed for the selected boot mode.
2. **Boot Process Phase:** The R5F and HSM collectively download the SBL image to the internal L2 RAM. Before execution is allowed, the **HSM performs code verification**. If the integrity check passes, execution begins; if it fails, the HSM forbids execution.


## Public ROM Code Architecture & Execution Flow

The Public ROM code running on the main **R5 core** is built upon a structured software stack consisting of modules, drivers, and an Inter-Processor Communication (IPC) layer. 

When the device powers on and the HSM un-halts the R5 core, the ROM executes a strict sequence to securely boot the device.

### 1️⃣ ROM Entry & Initialization
Execution begins at the hardware entry point with a low-level setup:
* **Core Prep:** Clears core registers and sets up the exception and main stacks.
* **Memory Diagnostics:** Performs PBIST (Programmable Built-In Self-Test) on the TCMB (Tightly Coupled Memory).
* **Branch to Main:** Completes TI auto-initialization and branches to the `main()` function.

### 2️⃣ Main Module Configuration
Before entering the actual boot loop, the main module configures the SoC:
* **System Setup:** Initializes the System MPU, Logger, VIM (Vector Interrupt Module), and RTI (Timers).
* **Clocks:** Configures the Core PLL (setting the R5 Core to **400MHz** and the HSM Core to **200MHz**).
* **Memory Init:** Runs PBIST on TCMA and L2 RAM, then initializes them.
* **IPC Link:** Brings up the Mailbox RAM to establish the IPC link with the HSM, and sends an `IPC_MsgType_HELLO` message to indicate the R5 is ready.

### 3️⃣ Boot Loop & Interface Selection
The ROM reads the physical bootstrap pins to identify the chosen boot interface and applies hardcoded default parameters:
* **QSPI (Memory Boot):** Configured for 40MHz. Supports fast single and Quad read modes. Defaults to loading from flash address `0x0` (or `0xF_0000` for a redundant SBL image).
* **UART (Host Boot):** Configured for 115200 bps, 8 data bits, no parity, 1 stop bit (8N1), and no flow control. Uses X-modem protocol for transfers.



##  The Security Handshake (HSM State Machine)

Because the AM2x is a secure device, the R5 core cannot just load and run the Secondary Bootloader (SBL) blindly. It must securely pass the data to the **HSM (M4 core)** for verification via the shared Mailbox RAM.

1. **Wait for Hello:** The HSM ROM waits until the R5 core finishes its PLL initialization and sends the initial "Hello" message over IPC.
2. **Certificate Validation:** The R5 core downloads the security certificate from the boot interface and passes it to the HSM. The HSM validates the certificate extensions against the specific device type.
3. **Receive & Verify Image:** The R5 core downloads the SBL image and feeds it to the HSM in chunks (≥ 2KB). The HSM performs two critical operations:
   * **Integrity (SHA512):** Computes the hash of the downloaded image and compares it to the hash in the validated certificate.
   * **Decryption (Optional):** If the certificate requires it, the HSM decrypts the image chunks.
4. **Handoff (R5 Wait / Sleep):** 
   * **✅ On Success:** The HSM ROM eclipses the R5 ROM and issues a reset to the R5 core. The verified SBL software then begins execution from memory address `0x0`.
   * **❌ On Failure:** If the certificate or image fails validation, the HSM rejects the boot, jumps back to the "Wait for Certificate" state, and retries the process.

---

##  ROM Software Stack Reference

To support this process, the Public ROM utilizes a lightweight internal architecture:
* **Modules:** Abstraction layers that handle the Boot Interface logic, Certificate parsing, X-modem protocol, System/Interrupt handling, IPC messaging, Pinmuxing, and Logging.
* **Drivers:** The low-level hardware control code for **QSPI** (Flash reading), **UART** (Host FIFO communication), **EDMA** (Direct memory transfers from flash to L2 RAM), **RTI** (Timeouts/timestamps), and **VIM** (Interrupts).

---

##  Boot Mode Pins Configuration

The AM2x ROM execution path is heavily hardware-dependent. The primary source for configuring the boot sequence comes from physical pins on the chip, which are sampled immediately after a Power-On Reset (POR).

*   **`BOOTMODE[3:0]` Pins:** These 4 pins define the primary boot mode, telling the ROM exactly which peripheral or memory interface to attempt to boot from first (e.g., QSPI, UART, USB).
*   **Boot Parameter Tables:** Once the pins are sampled, the ROM uses their values to populate internal tables that configure clocks, pins, and drivers for the selected interface.

> **⚠️ Hardware Design Note:** It is the user/board designer's responsibility to set these boot mode pins correctly using pull-up/pull-down resistors, jumpers, or dip switches based on the desired boot scenario.

---

## QSPI Boot Process (Memory Boot)

When booting from QSPI (Quad Serial Peripheral Interface) flash, the memory can serve as the boot media (holding the SBL image) or as a general storage drive for the application later on. 

**Core QSPI Boot Characteristics:**
*   **Copy-to-RAM (Not XIP):** QSPI boot on the AM2x is **not** eXecute-In-Place (XIP). The ROM code physically copies the boot image from the external flash memory into the internal on-chip RAM before executing it.
*   **Redundant Boot Support:** 
    * The ROM first looks for the primary SBL image at flash address `0x0000_0000`. 
    * If this fails (e.g., corrupted image, failed hash check), it automatically falls back to a redundant image located at `0x000F_0000`.
*   **No Hardware Handholding:** The ROM *does not* toggle reset lines or power pins for the QSPI device. It assumes the flash chip is fully powered and out of reset before the boot attempt begins.

###  QSPI Single (1S) vs. Quad (4S) Modes

The ROM supports both standard Single SPI and Quad SPI modes, depending on your pin configuration:

#### QSPI Quad (4S) Boot
*   **Command Issued:** `0x6B` (Quad Output Fast Read).
*   **Flash Dependency:** The ROM expects the **QE (Quad Enable) bit** to already be `SET` in the flash chip's non-volatile configuration register. The flash must wake up from a power cycle natively ready to accept Quad mode commands.

#### QSPI Single (1S) Boot
*   **Command Issued:** `0x0B` (Fast Read).
*   **Usage:** A safer, albeit slower, fallback or primary mode for standard SPI flash chips that do not support or are not configured for Quad-lane output.

##  UART Boot Process (Host Boot)

UART boot is a "Host Boot" mode, typically used during manufacturing, recovery, or development when external flash memory is blank or corrupted. 

### 1. Initialization & The "Ping"
Once the ROM code configures the selected UART port (based on the Boot Parameter Table), it immediately begins polling the host computer. 
* The device will send continuous **ASCII 'C'** characters to the host terminal for a few seconds.
* This ping indicates the ROM is alive and waiting for a file transfer.

### 2. XMODEM Loading Process
Before the 'C' pings time out, the host computer must begin transferring the boot image (SBL) using the **XMODEM** protocol. 
> **⚠️ XMODEM Constraints:** The ROM *only* supports XMODEM with **CRC mode** (Checksum mode is explicitly not supported). It accepts both standard 128-byte and 1024-byte (XMODEM-1K) block sizes.

### 3. Execution Hand-Over
Once the complete image is successfully transferred over UART and passes the HSM integrity checks, the ROM Code reads the boot header. It then branches execution to the specific memory address defined in the header's "Boot Info" field.

---

## DevBoot Mode

In addition to standard boot modes, the AM2x supports a specialized **DevBoot** mode. 
* This mode is exclusively designed for the development and debugging of the Secondary Bootloader (SBL).
* It is also the required mode for running the **JTAG-based KeyWriter**, which is used to provision security keys and certificates into the chip's eFuses during manufacturing.

---

## Redundant Boot Support (QSPI)

To ensure system reliability in the field, the AM2x ROM features built-in failover mechanics when booting from QSPI flash memory. 

If the primary boot fails, the ROM will automatically attempt to boot from a secondary, redundant image.
* **Primary Address:** `0x0000_0000`
* **Redundant Address:** `0x000F_0000`

### What triggers a redundant boot fallback?
The ROM will fail over to the redundant address if it encounters either of the following issues at the primary address:
1. **Certificate Corruption:** The HSM detects a mismatch in the security certificate (e.g., wrong image size, failed SHA512 hash, invalid extension IDs, or a broken digital signature).
2. **Image Corruption:** The physical data is unreadable or corrupted. This is often caused by single-bit or byte corruption due to flash memory aging (wear leveling), radiation, or external electromagnetic interference.


## 🛡️ Secure Boot Flow: ROM to Application Handoff

The secure boot process relies on a tightly orchestrated handoff between the main processor (MSS R5F) and the secure enclave (HSM CM4). The primary goal is to establish a chain of trust and ensure that the ROM code is completely isolated ("eclipsed") from the user application once the boot is complete.

### The 4-Step Handoff Sequence

**1. SBL Download (ROM Phase)**
The sequence begins with the MSS R5F ROM and the HSM ROM interacting to securely download and authenticate the Secondary Bootloader (SBL).

**2. SBL Execution & R5F Eclipsing**
Once verified, the SBL begins execution. At this exact moment, the MSS R5F ROM is **eclipsed** (hidden and locked out). From this point forward, R5F ROM services are completely unavailable for the remainder of the power cycle.

**3. HSM Runtime Loading**
With the SBL now in control of the main core, it invokes a specific API directed at the HSM ROM (which is still active on the secure CM4 core). This API instructs the HSM to download and verify the dedicated HSM Runtime Firmware.

**4. HSM Eclipsing & Application Load**
Once the HSM Runtime Firmware is successfully downloaded and running, the **HSM ROM is also eclipsed** and its services are locked out. With both ROMs secured and out of the picture, the SBL assumes full responsibility for downloading, configuring, and launching the final user application images.

##  Image Certification & Verification (X.509)

To guarantee that only authorized software runs on the AM2x, every boot image must consist of two parts: an **X.509 Certificate** followed immediately by the **Binary Image Blob**. 

The certificate uses specific Object Identifiers (OIDs), such as the Boot Information OID (`1.3.6.1.4.1.294.1.1`), to tell the ROM critical details like the target core, image size, and destination memory address.

### 1. Binary Image Creation (Developer/Build Phase)
Before flashing the device, your build tools (using OpenSSL) secure the image:
1. **Extension Population:** The certificate is populated with the image load address and a "Magic Number" from the unencrypted image.
2. **Encryption:** The binary is encrypted using **AES-256-CBC**.
3. **Hashing:** A SHA-512 hash of the *encrypted* image is computed and written into the certificate.
4. **Signing:** The entire certificate is hashed (SHA-512) and encrypted with an RSA Private Key (typically 4096-bit). The resulting signature and Public Key are embedded into the certificate.

### 2. Binary Image Verification (Boot Phase)
During boot on High-Security Enforced (HS-SE) devices, the HSM ROM acts as the gatekeeper:
1. **Public Key Check:** Computes the SHA-512 hash of the public key in the certificate and compares it against the secure key hash stored in the silicon's eFuses.
2. **Signature Check:** Decrypts the certificate signature to verify the certificate hasn't been tampered with.
3. **Image Hash Check:** Hashes the downloaded encrypted binary and ensures it perfectly matches the hash stored in the certificate.
4. **Decryption:** Decrypts the binary using AES-256-CBC.
5. **Magic Number Check:** Verifies the internal magic number matches the certificate's expected value.

---

## HSM Runtime Handoff & Eclipsing

Once the Secondary Bootloader (SBL) is successfully running on the R5F core, it must load the dedicated HSM Runtime Firmware onto the M4 core.

1. **Load Request:** The SBL sends a `LoadHSMRt` IPC message to the HSM ROM, providing the memory address where the HSM Runtime image is temporarily stored.
2. **Validation & Copy:** The HSM ROM validates the runtime's certificate. Upon success, it copies the binary into its internal IRAM (address `0x20000`).
3. **Integrity & Decryption:** The HSM ROM performs standard integrity checks and optional decryption on the copied image.
4. **The Eclipse:** To secure the system, the HSM ROM initiates the **eclipse process**. The ROM masks itself, remaps the IRAM start address to `0x0`, and issues a core reset. The HSM Runtime then begins execution at `0x0`, and the HSM ROM is permanently locked out.

> ** Post-Boot Assets:** Before eclipsing, the HSM ROM leaves behind an "Asset Data Structure" at the beginning of the SECURE RAM. This contains the ROM version, device type, UID, and derived keys for the HSM Runtime to utilize.

---

##  Failure & Recovery (Watchdog)

If either the R5 core or the HSM encounters a critical failure during the boot process (e.g., a failed certificate hash, missing image, or corrupted memory), the system will not hang indefinitely. 

A hardware Watchdog Timer (WDT) is actively monitoring the boot process. If the boot does not complete successfully within **180 seconds**, the WDT will issue a SoC warm reset, forcing the device to restart the boot sequence from the beginning.

---

| Feature / Stage | Standard Microcontroller Boot | Secure Microcontroller Boot |
| :--- | :--- | :--- |
| **Primary Goal** | Instantly execute the firmware application directly from Flash reset. | Verify the firmware integrity/authenticity before letting it control hardware. |
| **Root of Trust** | None. The hardware trusts whatever bytes exist at the default Reset Vector. | Immutable Hardware Boot ROM or write-protected Secure Flash sectors containing vendor public keys. |
| **Malware Safety** | High risk. A malicious firmware update can permanently hijack the device. | High safety. If signature verification fails, the system rolls back or enters a safe DFU recovery mode. |
| **Stage 1: Power-On Reset (Hardware)** | Core reads Stack Pointer from `0x0` and jumps immediately to the Main Application's Reset Handler. | Core reads Stack Pointer and jumps to the **Secure Bootloader** code section instead of the main app. |
| **Stage 2: Bootloader Execution** | **Skipped.** (Unless using a simple, non-secure custom bootloader for simple UART flashing). | The Secure Bootloader calculates a cryptographic hash (SHA-256) of the entire main application space. |
| **Stage 3: Cryptographic Validation** | **Skipped.** | The bootloader uses its built-in Public Key to verify the Application's digital signature (e.g., RSA or ECDSA). |
| **Stage 4: Vector Table Relocation** | System initializes hardware registers directly. | Bootloader changes the Vector Table Offset Register (VTOR) to point to the main validated application's vector table. |
| **Stage 5: App Handoff** | Application code calls `main()`. | Bootloader physically modifies the Program Counter to jump to the freshly authenticated Application's Reset Handler. |
| **Stage 6: RTOS / Bare-Metal Initialization** | Low-level clocks/memory initialization (`SystemInit`) followed directly by setting up global variables (`.data` / `.bss`). | Low-level clocks/memory initialization (`SystemInit`) followed directly by setting up global variables (`.data` / `.bss`). |
| **Stage 7: Thread Launch** | **Bare Metal:** Calls `main()` and enters a `while(1)` loop.<br>**FreeRTOS:** Calls `vTaskCreate()` then `vTaskStartScheduler()`. | **Bare Metal:** Calls `main()` and enters a `while(1)` loop.<br>**FreeRTOS:** Calls `vTaskCreate()` then `vTaskStartScheduler()`. |



---

### AM263x Sitara™ MCU Complete Boot Lifecycle 

| Phase / Step | Standard Development Boot (GP / HS-FS) | Secure Production Boot (HS-SE) |
| :--- | :--- | :--- |
| **Primary Goal** | Direct, unhindered multi-core application launch for fast development iteration. | Mitigate physical and remote threats by ensuring only untampered, signed code executes. |
| **Root of Trust** | Software-defined via external `BOOTMODE` pin states sampled at reset. | Hardware-defined via immutable silicon **eFuses** containing public key hashes. |
| **Security State** | Firewalls are open or permissive; cryptographic enforcement is inactive. | **Hardware Security Module (HSM)** core actively firewalls memory and peripherals. |
| **1. Voltage Stabilization** | PMIC holds the active-low **`PORz`** pin at Ground while 1.2V core and 3.3V I/O power rails ramp up to stabilize internal logic gates. | PMIC holds the active-low **`PORz`** pin at Ground while 1.2V core and 3.3V I/O power rails ramp up to stabilize internal logic gates. |
| **2. Hardware Latching** | `PORz` goes High (1). The SoC hardware snapshots the physical voltage levels on **`BOOTMODE`** GPIO pins to lock in the boot interface selection and initial QSPI clock speeds. | `PORz` goes High (1). The SoC hardware snapshots **`BOOTMODE`** pins AND reads the state of the internal hardware **eFuses** to lock down the High-Security configuration. |
| **3. Clock tree & First Core** | Internal High-Frequency Oscillator (HFOSC) starts. Hardware keeps all 4 user ARM Cortex-R5F cores frozen in reset and releases **only the internal HSM core**. | Internal High-Frequency Oscillator (HFOSC) starts. Hardware keeps all 4 user ARM Cortex-R5F cores frozen in reset and releases **only the internal HSM core**. |
| **4. ROM Bootloader Execution** | The immutable TI ROM code maps the internal 2MB Shared SRAM (OCSRAM) and copies the unencrypted **Secondary Bootloader (SBL)** from the chosen boot peripheral (e.g., QSPI Flash) into SRAM. | The immutable TI ROM code boots the **HSM Runtime** environment first to lock secure hardware firewalls. It then copies the encrypted, signed **SBL** from external Flash into a private SRAM buffer. |
| **5. SBL Validation** | **Skipped.** The ROM code immediately hands off execution control and jumps directly to the SBL binary on **Cortex-R5F Cluster 0, Core 0**. | **Strict.** The HSM core computes the SHA-256 hash of the SBL image and verifies its digital signature against the public key hash in the eFuses. If valid, it decrypts the SBL and allows R5F Core 0 to run it. |
| **6. Image Parsing** | SBL configures board-level clocks, peripheral pin multiplexing (PinMux), and parses the multi-core **`appimage`** file structure from Flash to locate distinct binaries for each R5F core. | SBL configures basic clocks under HSM firewall supervision and parses the signed multi-core **`appimage`** from Flash. It extracts the security certificates and code blocks. |
| **7. App Authentication** | **Skipped.** The SBL blindly moves application binary segments directly into target local memories without integrity or signature inspections. | **Strict.** SBL sends the application signature headers to the HSM via an **IPC (Inter-Processor Communication) mailbox**. The HSM verifies the vendor signature. If the binary has been modified, the HSM freezes the boot. |
| **8. Vector Mapping** | Because Cortex-R5F cores lack a hardware Vector Table Offset Register (VTOR), the SBL manually copies the application’s assembly branch block (**`.vectors` section**) directly into each individual R5F core’s private, local **ATCM memory space at address `0x0000_0000`**. | SBL manually copies the verified, clean **`.vectors` assembly section** directly into each validated R5F core’s local **ATCM memory space at address `0x0000_0000`**. |
| **9. Core Release** | SBL writes to the System Control registers to clear the individual hardware reset lines for all target R5F cores simultaneously. | SBL requests the HSM to open specific peripheral firewalls for the application, then clears the reset bits to release the R5F cores into their secure workspaces. |
| **10. Runtime Execution** | Released R5F cores hit their local address `0x0`, execute the vector branch to the C-runtime initialization script (`_c_int00`), clear `.bss`, copy `.data` variables, and call `main()` to launch **FreeRTOS** tasks. | Released R5F cores hit their local address `0x0`, execute the vector branch to `_c_int00`, set up their respective C-runtime variables in isolated RAM zones, and call `main()` to launch **FreeRTOS** tasks. |

---

# Microcontroller: Inter-Processor Communication (IPC) & Synchronization Reference Guide

This document provides a comprehensive technical synthesis of the Inter-Processor Communication (IPC) mechanisms, hardware synchronization primitives, memory architectural requirements, and software initialization workflows for the AM263x Sitara™ Microcontroller, based on the Technical Reference Manual (TRM).

---

## 1. Architectural Overview & Primitives

The AM263x platform relies on a heterogeneous multi-core architecture comprising ARM Cortex-R5F cores, an Integrated Cloud Subsystem for Mobile/Industrial (ICSSM), and a Hardware Security Module (HSM) powered by an ARM Cortex-M4F core. To implement higher-level system functions, these distinct processors interact via dedicated hardware and software IPC layers.

```
+-----------------------------------------------------------------------------------+
|                            APPLICATION LAYER (C Code)                             |
|          Uses RPMessage APIs for packets, IpcNotify APIs for signaling            |
+-----------------------------------------------------------------------------------+
                                         │
                                         ▼
+-----------------------------------------------------------------------------------+
|                          SOFTWARE ABSTRACTION LAYER (DPL)                         |
|      Transforms abstract endpoints and logical client IDs into register events    |
+-----------------------------------------------------------------------------------+
                                         │
                                         ▼
+-----------------------------------------------------------------------------------+
|                           HARDWARE SUBSYSTEM CORES                                |
|   [ MSS_CTRL (R5F / ICSSM Cores) ]               [ HSM Subsystem (M4F Core) ]     |
+-----------------------------------------------------------------------------------+
                │                                                │
                ▼                                                ▼
┌────────────────────────────────┐              ┌────────────────────────────────┐
│      HW MAILBOX INTERRUPTS     │              │       HW SPINLOCK MODULE       │
│ Asynchronous notification ring │              │  256 Atomic Semaphores (0-255) │
└───────────────┬────────────────┘              └───────────────┬────────────────┘
                │                                                │
                └───────────────────────┬────────────────────────┘
                                        │
                                        ▼
+-----------------------------------------------------------------------------------+
|                                PHYSICAL SRAM SHM                                  |
|   Shared Memory (USER_SHM, LOG_SHM, IPC_SHM) Configured via MPU as Non-Cached     |
+-----------------------------------------------------------------------------------+
```

### Primary Subsystem Components
1. **Hardware Mailbox Subsystem:** An asynchronous signaling mechanism generating hardware interrupts across processors.
2. **Hardware Spinlocks:** 256 hardware semaphores providing atomic synchronization across heterogeneous cores.
3. **Shared SRAM Banks:** Memory arrays accessible concurrently by multiple processors, utilized to store message payloads, debugging logs, and application states.
4. **Memory Protection Units (MPU):** Hardware firewalls at each Mailbox and processing node to control access, partition layouts, and enforce caching behaviors.

---

## 2. Hardware Synchronization Primitives

### 2.1 Hardware Mailbox Mechanism
The Mailbox framework coordinates asynchronous message exchanges. Instead of driving data buses directly, it utilizes hardware interrupts generated by a source processor to alert a target processor. The message payload resides in one of **two shared memory banks** optimized and recommended for this purpose.

* **Memory Partitioning:** Each Mailbox includes a dedicated MPU, allowing software to dynamically allocate and partition the mailbox memory between controllers and cores rather than using a rigid, pre-allocated hardware configuration.
* **Register Architecture:**
  * **R5F and ICSSM Cores:** Registers are mapped into the Master Subsystem Control (`MSS_CTRL`) memory space.
  * **HSM M4 Core:** Registers reside completely inside the isolated `HSM` memory space.

#### The Mailbox Communication Sequence
The mailbox mechanism enforces a strict protocol: **The sender must not initiate another transaction to the same receiver until the active sequence is complete.**

```
 SENDER PROCESSOR                                               RECEIVER PROCESSOR
+----------------+                                             +------------------+
| Write payload  |                                             |                  |
| to Shared SRAM |                                             |                  |
+───────┬────────+                                             |                  |
        │                                                      |                  |
        ▼                                                      |                  |
| Write 0x1 to   |─── (Hardware Interrupt: MBOX_READ_REQ) ────>│ Read             |
| _MBOX_WRITE_   |                                             | _MBOX_READ_REQ   |
| DONE[RECEIVER] |                                             | bit [SENDER]     |
+────────────────+                                             +────────┬─────────+
                                                                        │
                                                                        ▼
                                                               | Write 0x1 to     |
                                                               | _MBOX_READ_REQ   |
                                                               | [SENDER] (Clear) |
                                                               +────────┬─────────+
                                                                        │
                                                                        ▼
                                                               | Read payload from|
                                                               | Shared SRAM space|
                                                               +────────┬─────────+
                                                                        │
                                                                        ▼
| Receive Ack /  │<─── (Hardware Interrupt: MBOX_READ_DONE) ───| Write 0x1 to     |
| Clear Register |                                             | _MBOX_READ_DONE_ |
+────────────────+                                             | ACK[SENDER]      |
                                                               +──────────────────+
```

1. **Write Data:** The **SENDER** writes the message payload into the shared SRAM block.
2. **Signal Interrupt:** The **SENDER** triggers a hardware interrupt by writing `0x1` to the `_MBOX_WRITE_DONE[RECEIVER]` register.
3. **Aggregate Alert:** The **RECEIVER** receives a single aggregated interrupt named `MBOX_READ_REQ`.
4. **Identify Source:** The **RECEIVER** evaluates the `_MBOX_READ_REQ` register and detects that the bit corresponding to `[SENDER]` is set to `0x1`.
5. **Clear Flag:** The **RECEIVER** clears the active interrupt by writing `0x1` to `_MBOX_READ_REQ[SENDER]`.
6. **Fetch Payload:** The **RECEIVER** safely reads the message payload from the shared memory block.
7. **Complete Handshake:** The **RECEIVER** issues an acknowledgment interrupt back to the sender by writing `0x1` to `RECEIVER>_MBOX_READ_DONE_ACK[SENDER]`.

---

### 2.2 Spinlock Module
The Spinlock module implements **256 distinct hardware semaphores** (`y = 00h` to `FFh`) to synchronize processing loops across multiple cores running separate operating systems (or No-RTOS environments). 

#### Key Attributes
* **Atomic Access:** It processes a single read command completely before accepting subsequent requests. This allows single-access locks and eliminates the need for read-modify-write (RMW) bus transactions, which some programmable cores on the SoC cannot execute natively.
* **No Ownership Enforcement:** The hardware does **not** check or enforce that the core unlocking a register is the same one that locked it.
* **No Initiator or Congestion Checking:** The module does not verify the VBUS initiator freeing the lock, nor does it provide fairness, arbitration, or congestion control protocols.

#### Hardware State Register Matrix (`SPINLOCK_LOCK_REG_y`)

| Initial Hardware State | Software Operation | Return Value | Final Hardware State | Functional Meaning |
| :--- | :--- | :--- | :--- | :--- |
| **Not Taken (0)** | Read Register | `0` | **Taken (1)** | Lock was free; successfully acquired by caller. |
| **Taken (1)** | Read Register | `1` | **Taken (1)** | Lock is busy; request failed. |
| **Taken (1)** | Write `0` | N/A | **Not Taken (0)** | Lock is released and made available to other cores. |
| **Any State** | Write `1` | N/A | *No Change* | Invalid operation; ignored by hardware. |

#### Architectural Suitability Matrix
Spinlocks are highly effective for protecting critical shared structures across distinct subsystems, provided they meet specific operational constraints.

```
                           +----------------─────────────────+
                           |   Does it meet criteria?        |
                           |   1. Hold time < 200 CPU cycles |
                           |   2. Task is non-preemptible    |
                           |   3. Light lock contention      |
                           +────────────────┬────────────────+
                                            │
                     ┌──────────────────────┴──────────────────────┐
                     ▼ YES                                         ▼ NO
+-----------------------------------------+   +-----------------------------------------+
|        SUITABLE FOR SPINLOCK            |   |       UNSUITABLE FOR RAW SPINLOCK       |
| Execute an active retry loop until lock |   | Use spinlocks as a low-level primitive  |
| is successfully acquired.               |   | to implement a higher-level semaphore.  |
+-----------------------------------------+   +-----------------------------------------+
```

* **System Initialization / Bug Recovery:** Software resets the spinlock module by writing `1` to `SPINLOCK_SYSCONFIG[1] SOFTRESET`. Because hardware reset states can be unpredictable after a system bug recovery, **software must write `0` to all 256 `SPINLOCK_LOCK_REG_y` registers during system startup** to guarantee all locks begin in the *Not Taken* state.

---

## 3. High-Level Software IPC Frameworks

To abstract these low-level registers, the software stack provides two driver layers: **IPC Notify** and **IPC RP Message**.

```
+---------------------------------------------------------------------------------------+
|                                    IPC RP MESSAGE                                     |
|  • Packet payload-based data flows (4 to 512 Bytes)                                   |
|  • Multi-endpoint routing matching UDP socket topologies                              |
+---------------------------------------------------------------------------------------+
                                           │
                                           ▼ (Uses underneath)
+---------------------------------------------------------------------------------------+
|                                      IPC NOTIFY                                       |
|  • Lightweight, low-latency messaging framework                                       |
|  • Transmits immediate 28-bit Message IDs straight to Client IDs                      |
+---------------------------------------------------------------------------------------+
```

### 3.1 IPC Notify
* **Functional Behavior:** An optimized, low-latency framework that sends signals directly over hardware interrupts without the overhead of shared memory management buffers.
* **Payload Capacity:** Limited to a **28-bit message ID** sent alongside the interrupt vector.
* **Addressing Model:** Supports logical endpoints or ports called **Client IDs**, up to the maximum limit defined by `IPC_NOTIFY_CLIENT_ID_MAX`.

### 3.2 IPC RP Message
* **Functional Behavior:** A structured packet network layer that passes message packet buffers through a dedicated shared memory segment. It uses the low-level **IPC Notify** driver underneath to signal packet delivery events.
* **Payload Capacity:** Supports variable packet dimensions from **4 bytes up to 512 bytes**. In standard multi-core RTOS/No-RTOS deployments, the default packet layout is capped at **128 bytes** to minimize the physical footprint within on-chip RAM.
* **Addressing Model:** Emulates standard UDP routing. The `CPU ID` serves as the physical network address, and a 16-bit identifier called a **Local Endpoint** serves as the port. This layout is constrained to `RPMESSAGE_MAX_LOCAL_ENDPT` to maintain a low memory footprint and high processing efficiency.

> ⚠️ **M4F Isolation Constraint:** When communicating with the **M4F core**, applications **must use IPC Notify exclusively**. On this hardware platform (e.g., AM243X), the M4F serves as an isolated safety controller. It is restricted from accessing shared memory blocks outside its subsystem, which prevents the use of the RP Message framework.

---

## 4. Multi-Core System Design Patterns

### 4.1 Client-Server Topology (RP Message Mode)

The classic operational design pattern for cross-core messaging maps service endpoints to independent tasks or processing execution loops.

```
CLIENT CPU (e.g., Core R5F_0)                              SERVER CPU (e.g., Core R5F_1)
+──────────────────────────┐                              +──────────────────────────┐
| Create unique local      |                              | Create public well-known |
| Endpoint for replies     |                              | Service Endpoint         |
+────────────┬─────────────+                              +────────────┬─────────────+
             │                                                         │
             ▼                                                         ▼
| Construct command packet |                              | Enter blocking state     |
| with parameters inside   |                              | awaiting incoming packet |
+────────────┬─────────────+                              +────────────┬─────────────+
             │                                                         │
             │     1. Send RP Message Packet                           │
             └────────────────────────────────────────────────────────>│ • Extracts Command ID
                                                                       │ • Decodes parameters
                                                                       | • Tracks Source Core
                                                                       | • Tracks Reply Endpoint
                                                                       +────────────┬─────────────+
                                                                                    │
                                                                                    ▼
| Block or execute local   |                                   | Execute requested task   |
| background processes     |                                   | compute or read sensor   |
+────────────┬─────────────+                                   +────────────┬─────────────+
             ▲                                                              │
             │             2. Transmit Ack / Return Packet                  │
             └──────────────────────────────────────────────────────────────┘
```

#### Large Parameter Scaling Optimization
If an application needs to pass parameters that exceed the standard buffer limits (e.g., > 512 bytes), it should place a physical **memory pointer** inside the RP Message packet. This pointer should reference an auxiliary, larger shared memory allocation containing the actual data block.

```
+─────────────────────────────────────────+
|         RP MESSAGE PACKET STREAM        |
|  [Command ID] [Status] [SHM Address] ───┼──────────┐
+─────────────────────────────────────────+          │ (References)
                                                     ▼
                  +──────────────────────────────────────────────────────────────────+
                  |               AUXILIARY USER SHARED MEMORY SECTION               |
                  |  Contains heavy analytics datasets, multi-channel arrays, etc.   |
                  +──────────────────────────────────────────────────────────────────+
```

---

## 5. Software Configuration & Enabling Steps

To integrate IPC within an application framework, developers must configure the driver layers, memory boundaries, and synchronization barriers.

### Step 1: SysConfig Driver Integration
Open the Code Composer Studio (CCS) SysConfig user interface. Add and enable **IPC RPMessage** and/or **IPC Notify** modules for all participating cores.

### Step 2: Linker Command File Alignment
Modify the application linker scripts (`.cmd`) for every core on the SoC. The shared memory regions **must be mapped to identical physical addresses across all cores**, whereas code and stack blocks must occupy separate, non-overlapping regions.

```linker
/* Memory configuration: Shared memory blocks must match across all CPU linker files */
MEMORY
{
    USER_SHM_MEM            : ORIGIN = 0x701D0000, LENGTH = 0x00004000  /* 16 KB User Alloc */
    LOG_SHM_MEM             : ORIGIN = 0x701D4000, LENGTH = 0x00004000  /* 16 KB Log Alloc  */
    RTOS_NORTOS_IPC_SHM_MEM : ORIGIN = 0x701D8000, LENGTH = 0x00008000  /* 32 KB VRing Alloc*/
}

/* Section Placement: Use the (NOLOAD) directive to prevent cores from clearing shared data */
SECTIONS
{
    .bss.user_shared_mem (NOLOAD) : {} > USER_SHM_MEM
    .bss.log_shared_mem  (NOLOAD) : {} > LOG_SHM_MEM
    .bss.ipc_vring_mem   (NOLOAD) : {} > RTOS_NORTOS_IPC_SHM_MEM
}
```

### Step 3: Configure the MMU/MPU as Non-Cached
Because cores utilize aggressive data caches, modifications to shared memory might remain local to a core's cache and not update physical RAM immediately. This can cause data coherency issues where other cores read stale values.

* **Fix:** In SysConfig, go to **TI DRIVER PORTING LAYER (DPL)** -> **MPU**. Add a dedicated region entry encompassing the entire shared memory block (`0x701D0000` through `0x701DFFFF`). Set its attributes to **Non-Cached / Strongly Ordered / Shared**. This forces all read and write commands to bypass the cache layers and access the physical SRAM directly.

### Step 4: Implement Core Synchronization Barriers
Because cores initialize at different speeds, a client core could attempt to send an IPC packet before the destination core has configured its endpoints. To prevent packet loss or system faults, use the following API to establish a synchronization barrier during the boot sequence:

```c
#include <drivers/ipc_notify.h>
#include <kernel/dpl/SystemP.h>

void main_application_entry(void)
{
    // 1. Core-local peripheral and hardware subsystem initialization
    System_init();
    Board_init();

    // 2. Wait indefinitely until all configured SoC cores are synchronized
    IpcNotify_syncAll(SystemP_WAIT_FOREVER);

    // 3. Safe initialization of communication loops and endpoints
    Server_Endpoint_Init();
    
    while(1)
    {
        // Core execution loop
    }
}