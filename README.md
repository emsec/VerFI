# VerFI ver 2 Beta

This repository contains the source code for the program written for the project "Cryptographic Fault Diagnosis using VerFI"

The first version of the program is published here: https://github.com/vmarribas/VerMFi

This repository is the second version of the program which supports not all features but has a higher performance. 
The progarm is written in Ruhr University Bochum, Embedded Security (Germany)

You find two folders in the repository: 
 * "VerFI for attack": it can be used to emulate fault-injection attacks. It generates input/outputs for which the injected fault was 
 "detected"/"ineffectice"/"non-detected". The generated result can be used to mount an attack like DFA/SIFA.

* "VerFI for diagnosis": it can be used to exmaine a design in detection/correction of the faults. 
 The tool provides a summary of the diagnosis and can generate detailed list of the diagnosis, e.g., 
 for which kind of fault, in which clock cycle, and which cells the result is "faulty"/"fault is detected"/"fault is ineffective".
 

The paper is published by IEEE International Symposium on Hardware Oriented Security and Trust, HOST 2020.
Please use the following citation, if you use our implementations in your research:

Victor Arribas and Felix Wegener and Amir Moradi and Svetla Nikova: Cryptographic Fault Diagnosis using VerFI. HOST 2020.

The pre-print version of the paper is available here:
https://eprint.iacr.org/2019/1312
