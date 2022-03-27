# 82C55 Port tester

This Arduino program is used to test port A, B, and C on an 82C55 in mode 0.  It read/writes to the port several bit patterns (alternating 1's, alternating 0's, all 0's, all 1's, walking 0's, walking 1's) in both input & output directions to validate the chip is working.  In total 20 bit patterns are tested in both input & output modes for a total of 40 tests for each port.

This requires an ATmega 2560 5V board, and only 8 pins are used to read/write the port so you have to rewire your breadboard to test each port and change the portNum constant in the program.

