# Yama Compiler

This is a Runtime for the Language Yama.

## Commands
```
size (Size)    The Memorysize for the Programm
help           Show help
(filepath)     A .yexe file to exectue
```

## Getting Started

make build

### Install
I use a link to Yama Compiler/Runtime and use it
```
sudo ln -s /mnt/c/pro/yamaRuntime/bin/yamaRuntime.exe /usr/bin/yama
```
In Windows i register the filetype .yexe to open defualt with yamaRuntime.exe

### Prerequisites

* gcc
* make

## Running the tests

*placeholder*

## Built With

* make build

## Assembler
### Opcodes Definition
 * {X}
    - 0x0 Add
    - 0x1 Adc
    - 0x2 Sub
### Formats
 * Format 1
    - This Opcodes are Format 1: 0x5{X}, 0xF{Z}, 0x30, 0x31
    - Opcode (31 - 24) (8 Bits) (op),
    - Condition (23 - 20) (4 Bits) (co),
    - Register Destination (19 - 16) (4 Bits) (rd),
    - Register Left Operation (15 - 12) (4 Bits) (rl),
    - Register Right Operation (11 - 8) (4 Bits) (rr)
```
    add(co) (rd),(rl),(rr)
```
 * Format 2
    - This Opcodes are Format 2: 0x1{X}
    - Opcode (31 - 24) (8 Bits) (op),
    - Condition (23 - 20) (4 Bits) (co),
    - Register Destination (19 - 16) (4 Bits) (rd),
    - Register Left Operation (15 - 12) (4 Bits) (rl),
    - Const Right Operation (11 - 0) (12 Bits) (im)
```
    add(co) (rd),(rl),#(im)
```
* Format 3
    - This Opcodes are Format 3: 0x2{X}, 0x4{Y}, 0x32
    - Opcode (31 - 24) (8 Bits) (op),
    - Condition (23 - 20) (4 Bits) (co),
    - Register (19 - 16) (4 Bits) (rd),
    - Const Right Operation (15 - 0) (16 Bits) (im)
```
    mov(co) (rd),#(im)
    push(co) {register list}
```
## Authors

* **Robin D'Andrea** - *Robinterra* - [Robinterra](https://github.com/Robinterra)