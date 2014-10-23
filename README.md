MQTT2CANopen
============

MQTT messaging gateway for CANopen



CANopen stack
=============

The CANopen slave stack can be acquired from company emtas GmbH providing an
excellent and compliant implementation of the CANopen specification. It is
available as a download package for evaluation purposes without a license, so
beware that it is illegal to use their binary code for anything more than
evaluation. If you want to use this project for other purposes than evaluation,
please buy the CANopen slave stack or source code.

For compiling you will need to put the header files and binary libraries of the
[example for standard PC with
Linux](http://www.emtas.de/en/download/canopen#Example_for_standard_PC_with_Linux).
Copy the header files from directory colib/inc/ into colib/include/ and the
libraries from directory libs/ into colib/lib/.
