# SYSTEMS 12 - BridgeSync Instructions
Tester program to determine bridge open/close variables and limits.

## About
This sub-program is written to be used to be manually modified to suit the required tests of the bridge. It is designed to run UNTIL TERMINATION, reading the number of rotations it takes to complete a cycle of the bridge. This is done by running the motor forward initially for 5 seconds, following this it will print out the number of rotations it completed in this time. Next, the motor will reset back to its neutral 'closed' state by running back those counted rotations (instead of it being timed). Upon returning to its neutral state it will then run again forward for 10 seconds, continuing the pattern in increments of 5. There is no overide in this program, meaning when it needs to be stopped, the ESP or power source needs to be deactivated (unplugged).

## Manual Input
The program has the main BridgeSync, but also contains a clockwise and anticlockwise program that can be used for debugging. The variable can be found at 'System Run Type', this needs to be manually changed before compiling. The functions are as follows:
- 0 = Run sync check (Main)
- 1 = Run continuously clockwise
- 2 = Run continuously anticlockwise

## Program Goal
By printing out the motor's rotations to complete a cycle of opening the bridge, we can use this as a variable that can directly be applied into the main BridgeController program. This sets the limit for the motor so it will never breach the specific bridge's max/min lifting distance. By running the motor on a rotation counter it means that it can be run on any voltage and doesn't require a specific amount of time to complete the bridge opening/closing procedure.

## WARNING
**This program could damaged the bridge when reaching limits! Ensure that when the motor is hitting its limit of the bridge, SHUT OFF ITS POWER.**




