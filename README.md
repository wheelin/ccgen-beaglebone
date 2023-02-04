# CCGEN Beaglebone

CCGEN stands for Crank/Cam generation. The aim is to generate crankshaft and camshaft speed signals
on the beaglebone using the PRU.

This work has been realized with the help of the repo [exploringBB](https://github.com/derekmolloy/exploringBB) and the [PRU cookbook](https://markayoder.github.io/PRUCookbook/).

## Building and running

Certain commands below might require admin rights.

### Pru
```
config-pin P9_27 pruout
config-pin P9_28 pruout
cd Pru
make
make install_PRU0
```

### Host
```
cd Host/
mkdir build
cd build
cmake ../
make
./ccgen
```
The program waits for the following inputs:
- d: decrease speed generation
- u: increase speed generation
- e: enable signal generation
- n: disable signal generation
- q: quit

## Architecture

The host program generates the pattern and shares it with the PRU0 in its DRAM at address 0x200,
just after the stack and heap memory spaces.

The PRU0 iterates on the pattern, composed of 7200 elements, each one a bit mask for the state of
the sensor signals.

The speed is controlled by a decreasing counter whose value is given by the host application. No direct relation is currently intended between the value of the counter and the values of the RPM on the output.

The pattern generates the camshaft signal on the P9_27 and the crankshaft on P9_28.