# Readme
## Synopsis
testInterp is a first attempt at an interpreter in C

The language interpreted is based on assembly language and is written in plain text with semi-colon terminators on the end of lines.
A fuller description of the language is detailed below.

## Interpeter
The interpreter itself uses a register-based design, with 8kB of memory allocated to the program for use, with a WORD length of 2 bytes.
The registers are detailed in a lower section.

#### Calling the interpreter (cmd line only)
```interp(.exe) [file] [debug=0]```

[file] specifies the file to load and run. Can have any file extension.

[debug] defaults to 0 and may be ommitted. `1` enables debugging mode for the console output.

#### Flow
The actions of the interpreter as follows:
1. Loads the file designated in the first argument after it's cmd call
2. Sets the debug flag internally based on the second argument of the cmd call
3. Execution begins at the top of the file
4. Opcodes are read, and interpreted into `enum` format internally
5. Actions are then performed from the translated opcode, and operands accessed and acted upon
6. The execution continues until EOF or `exit;` is used

#### Specifications
* WORD length of 2 bytes (16 bits). Memory, stack and registers are in WORD lengths
* 8kB of program memory (4096 addressable locations)
* 16-deep stack register
* 16 registers, 2 bytes each (WIP)
  * r[0] --> fixed at value 0, read only
  * r[1] --> currently addressed memory location, read write
  * r[2] --> arithmetic overflow/remainder register, set when division occurs or to the lost bit in a bitshift, read only
  * r[3] --> error register, read only
  * r[4] to r[8] --> reserved for future use, read only
  * r[9] to r[15] --> open for program use, read and write
* Built-in console output

## The language
The language is based on assembly code, in that it contains 'opcodes' and 'operands'. Opcodes are strings (or char* depending on your prefered flavour).
There is a variable number of operands possible after an opcode, meaning no fixed length of instruction. Instructions are terminated with a semi-colon (';'), and only one instruction is permitted per line.
The language is capable of dealing with both '\n' and '\r\n' character returns, but requires the presence of the '\n' newline char at the end of each line.

#### Instruction set
##### Notes:
* Register numbers e.g. <r1>, <r2> are written as numbers, e.g. '1', '10', '12' etc.
* Comments may also be placed after ; characters
* Files must end in a new line

##### Instructions:

| Command | Details |
| --- | --- |
| #[comment]; | Comment line. Not executed |
| :[name]; | Creates a label at this line with [name]. Cannot contain spaces |
| exit; | Exits the currently running program. Takes no operands |
| hello [person]; | Says hello in the console to [person]. [person] may not contain spaces |
| con [text]; | Prints [text] to the console. [text] may contain spaces |
| conr [reg]; | Prints the value of register [reg] to the console |
| conc [reg]; | Prints the value of register [reg] as a char to the console |
| conv [reg]; | Prints the numerical value of [reg] to the console |
| add [r1] [r2]; | Adds the value of [r2] to [r1]. Stores in [r1] |
| addn [r1] [num]; | Adds the value of [num] to [r1]. Stores in [r1] |
| sub [r1] [r2]; | Subtracts the value of [r2] from [r1]. Stores in [r1] |
| subn [r1] [num]; | Subtracts the value of [num] from [r1]. Stores in [r1] |
| div [r1] [r2]; | Divides register [r1] by [r2], stores in [r1] and puts the remainder in r[2] |
| divn [r1] [num]; | Divides register [r1] by [num], stores in [r1] and puts the remainder in r[2] |
| mul [r1] [r2]; | Multiplies register [r1] by [r2] and stores in [r1] |
| muln [r1] [num]; | Multiplies register [r1] by [num] and stores in [r1] |
| mod [r1] [r2]; | Does [r1] % [r2] and stores in [r1] |
| modn [r1] [num]; | Does [r1] % [num] and stores in [r1] |
| bpl [r1] [r2]; | Bit-shifts up (<<) the value of [r1] by valueof([r2]) bits , e.g. `bpl 9 10` bitshifts up r[9] by the value of r[10] |
| bnp [r1] [num]; | Bit-shifts up (<<) the value of [r1] by [num] bits |
| bmi [r1] [r2]; | Bit-shifts down (>>) the value of [r1] by valueof([r2]) bits , e.g. `bpl 9 10` bitshifts down r[9] by the value of r[10] |
| bnm [r1] [num]; | Bit-shifts down (>>) the value of [r1] by [num] bits |
| mv [r1] [r2]; | Moves the value of [r2] to [r1] |
| set [r1] [num]; | Sets the value of [r1] to [num]. [num] is an integer |
| svm [r1]; | Saves the value of [r1] to the memory address pointed to by r[1] |
| rdm [r1]; | Reads the value of memory pointed to by r[1] and stores in [r1] |
| jmp [ln]; | Jumps to line [ln] of the program |
| jmpl [name]; | Jumps to label [name] (defined with :[name]) |
| func [name]; | Jumps to label [name] and stores the data for returning to this call (see `ret`) on the stack |
| ret; | Returns from `func` jump |
| jmpr [+/-]; | Jumps relative to this line [+/-] lines |
| if [e/n/l/m] [r1] [r2]; | Executes the code following the if statement if [r1] [== / != / \< / \>] [r2]. Terminated with `:endif` label |
| ife [ln] [r1] [r2]; | Jumps to line [ln] of the program if [r1] == [r2] |
| ifn [ln] [r1] [r2]; | Jumps to line [ln] of the program if [r1] != [r2] |
| ifl [ln] [r1] [r2]; | Jumps to line [ln] of the program if [r1] < [r2] |
| ifm [ln] [r1] [r2]; | Jumps to line [ln] of the program if [r1] > [r2] |
| push [r1] | Pushes the value of [r1] to the stack |
| pop [r1] | Pops the value on top of the stack and stores in [r1] |
| peak [r1] | Peaks at the value on top of the stack (doesn't pop) and stores in [r1] |

#### Todo list:
- [ ] Add a graphical output other than the console
- [ ] Give this a project a proper name
- [ ] Fix `conc` so it actually displays what it is supposed to
- [ ] Give the language a proper name
- [ ] Implement multiple commands per line
- [ ] Create a CHANGELOG
- [ ] Implement errors and the error register
- [ ] Implement overflow/remainder register capturing the lost bit in a bitshift
- [ ] Implement a better way of searching commands and operands that is extensible (will allow multiple commands per line)