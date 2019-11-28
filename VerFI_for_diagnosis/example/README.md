
This example is an implementation of CRAFT with 2-bit error detection.

It simulates the design with plaintedxt "0123456789ABCDEF" and key "18F4EEBDFCED7841D9E0E38CFE6A9405" for 10000 fault injections.

It injects exactly 4 toggle faults in an encryption run. 2 toggle faults at clock cycle 10 and 2 others at clock cycle 11.

All cells of the design can be targeted by fault injection except those in the module "OutputReg".

The design should detect all possible 2-bit faults injected at only one clock cycle of the entire encryption. 
It means, univariate adversary model with t=2 (Impeccable Circuits: https://eprint.iacr.org/2018/203

However, it does not guarantee to detect more faults (e.g., 2-bit faults per clock cycle in consecutive clock cycles).

The results show that among 10000 fault injections 1 case led to "non-detected".

It can be checked that the design detects all 2-bit faults (injected at only one clock cycle per encryption) by the following settings in sim.txt file:


number_of_target_clock_cycles  % number of clock cycles to inject the faults followed by the targeted clock cycles
1                              % one target clock cycle
10                             % clock cycle 10 is targeted

max_no_faults_per_run     % maximum number of injected faults per run (encryption)
2

min_no_faults_per_run     % minumum number of injected faults per run (encryption)
2

max_no_faults_per_cycle   % maximum number of injected faults per clock cycle
2

min_no_faults_per_cycle   % minumum number of injected faults per clock cycle
2


