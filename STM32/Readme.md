# Processor Architecture and organization

- computer architecture: user view of computer the instruction set,register,memory mgmt ,exception handling all are part of architecture
- computer organization: user invisible implimentation of architecture ,the pipeline structure,transperent cache,table look aside buffer,table walking hardware


- levels of abstraction
```text
   App SW               (program)                     
     ↓
Operating system        (device driver)
     ↓
 Architecture           (Instructions,registers)
     ↓
micro Architecture      (Data path,controllers)
     ↓
   Logic                (adders,memories)
     ↓
Digital circuit         (AND,OR Gates)
     ↓
Analog circuit          (Amplifiers,filters)
     ↓
  devices               (Transistors,diodes)
     ↓
  Physics               (Electrons)
```

  > - transistors --> LOGIC dESIGN(truth table logic symbol) --> Gate level design(adder,multiplexer,flipflop) --> Data Path design (RTL)/Control Path(control logic)/ALU design,power reduction design -->instruction set design (addressing modes,control flow instructions)


### ➡️ **[Cortex-M4](./STcontroller.md)**
### ➡️ **[Bootloader](./Bootloader/Readme.md)**
### ➡️ **[Embedded Driver development](./Driver_development/Readme.md)**
### ➡️ **[Protocols](./Communication/Readme.md)**
### ➡️ **[RTOS](../RealTimeOS/Readme.md)**
### ➡️ **[SystemDesign](./StateMachines/Readme.md)**