
This example is an implementation of CRAFT with 2-bit error detection.

It simulates the design with plaintedxt "0123456789ABCDEF" and key "18F4EEBDFCED7841D9E0E38CFE6A9405" for all possible 2-bit faults at a single clock cycle.

It injects exactly 2 toggle faults in an encryption run, both at clock cycle 10.

All cells of the design can be targeted by fault injection except those in the module "OutputReg" and "FSMSignals_doneInst".
Fault injection on these cells leads to meaningless results.

The design should detect all such faults.
It means, univariate adversary model with t=2 (Impeccable Circuits: https://eprint.iacr.org/2018/203

In total 1753 can be targeted for fault injection.
Such an exhaustive diagnostic needs  3073009  2-bit fault injections, which took 5 seconds using a 30-core machine (thanks to the BitSlided implementation).

The results show that there is no "non-detected" fault.
