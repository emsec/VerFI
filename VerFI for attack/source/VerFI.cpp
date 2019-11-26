//////////////////////////////////////////////////////////////////////////////////
// COMPANY:		Ruhr University Bochum, Embedded Security
// AUTHOR:		Amir Moradi (for the paper: https://eprint.iacr.org/2019/1312 )
//////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019, Amir Moradi 
// All rights reserved.
//
// BSD-3-Clause License
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the copyright holder, their organization nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTERS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "cstdio"
#include "conio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "omp.h"
#include "exprtk.hpp"

#include "Definitions.h"
#include "ReadLibraryFile.h"
#include "ReadCellReportFile.h"
#include "ReadDesignFile.h"
#include "ReadSimulationFile.h"
#include "PrintCheckTools.h"
#include "SimulationTools.h"

int main(int argc, char *argv[]) 
{
	char *LibraryFileName = (char *)malloc(Max_Name_Length * sizeof(char));
	char *LibraryName = (char *)malloc(Max_Name_Length * sizeof(char));
	char *InputVerilogFileName = (char *)malloc(Max_Name_Length * sizeof(char));
	char *InputCellReportFileName = (char *)malloc(Max_Name_Length * sizeof(char));
	char *InputSimulationFileName = (char *)malloc(Max_Name_Length * sizeof(char));
	char *MainModuleName = (char *)malloc(Max_Name_Length * sizeof(char));
	char *SummaryFileName = (char *)malloc(Max_Name_Length * sizeof(char));
	char *ResultFileName = (char *)malloc(Max_Name_Length * sizeof(char));
	int  i;

	LibraryFileName[0] = 0;
	LibraryName[0] = 0;
	InputVerilogFileName[0] = 0;
	InputCellReportFileName[0] = 0;
	InputSimulationFileName[0] = 0;
	MainModuleName[0] = 0;
	SummaryFileName[0] = 0;
	ResultFileName[0] = 0;

	if (argc > 1)
	{
		if ((!strcmp(argv[1], "-h")) | (!strcmp(argv[1], "-?")) | (!strcmp(argv[1], "/?")) | (!strcmp(argv[1], "-help")))
		{
			printf("usage:\n");
			printf("VerFI.exe [-lf/-libraryfile ?] [-ln/-libraryname    ?] [-df/-designfile     ?]\n");
			printf("          [-mn/-modulename  ?] [-cf/-cellreportfile ?] [-sf/-simulationfile ?]\n");
			printf("          [-uf/-summaryfile ?] [-rf/-resultfile  ?]\n");
			return 0;
		}
	}

	i = 1;
	while (i < argc-1)
	{
		if ((!strcmp(argv[i], "-lf")) | (!strcmp(argv[i], "-libraryfile")))
			strcpy(LibraryFileName, argv[i+1]);

		if ((!strcmp(argv[i], "-ln")) | (!strcmp(argv[i], "-libraryname")))
			strcpy(LibraryName, argv[i + 1]);

		if ((!strcmp(argv[i], "-df")) | (!strcmp(argv[i], "-designfile")))
			strcpy(InputVerilogFileName, argv[i + 1]);

		if ((!strcmp(argv[i], "-mn")) | (!strcmp(argv[i], "-modulename")))
			strcpy(MainModuleName, argv[i + 1]);

		if ((!strcmp(argv[i], "-cf")) | (!strcmp(argv[i], "-cellreportfile")))
			strcpy(InputCellReportFileName, argv[i + 1]);

		if ((!strcmp(argv[i], "-sf")) | (!strcmp(argv[i], "-simulationfile")))
			strcpy(InputSimulationFileName, argv[i + 1]);

		if ((!strcmp(argv[i], "-uf")) | (!strcmp(argv[i], "-summaryfile")))
			strcpy(SummaryFileName, argv[i + 1]);

		if ((!strcmp(argv[i], "-rf")) | (!strcmp(argv[i], "-resultfile")))
			strcpy(ResultFileName, argv[i + 1]);

		i += 2;
	}

	if (!LibraryFileName[0])
	{
		strcpy(LibraryFileName, "Library.txt");
		printf("default libraryfile \t""%s"" \tis taken\n", LibraryFileName);
	}

	if (!LibraryName[0])
	{
		strcpy(LibraryName, "NANG45");
		printf("default libraryname \t""%s""   \tis taken\n", LibraryName);
	}

	if (!InputVerilogFileName[0])
	{
		strcpy(InputVerilogFileName, "design.v");
		printf("default designfile \t""%s"" \tis taken\n", InputVerilogFileName);
	}

	if (!MainModuleName[0])
	{
		strcpy(MainModuleName, "Cipher");
		printf("default modulename \t""%s""   \tis taken\n", MainModuleName);
	}

	if (!InputCellReportFileName[0])
	{
		strcpy(InputCellReportFileName, "cells.rpt");
		printf("default cellreportfile \t""%s"" \tis taken\n", InputCellReportFileName);
	}

	if (!InputSimulationFileName[0])
	{
		strcpy(InputSimulationFileName, "sim.txt");
		printf("default simulationfile \t""%s"" \tis taken\n", InputSimulationFileName);
	}

	if (!SummaryFileName[0])
	{
		strcpy(SummaryFileName, "Summary.txt");
		printf("default summaryfile \t""%s"" \tis taken\n", SummaryFileName);
	}

	if (!ResultFileName[0])
	{
		strcpy(ResultFileName, "Result.txt");
		printf("default resultfile \t""%s"" \tis taken\n", ResultFileName);
	}

	printf("\n");

	//---------------------------------------------------------------------------------------------//
	//------------------- reading the library file ------------------------------------------------//
	int res;

	CellTypeStruct **CellTypes = NULL;
	int NumberOfCellTypes;

	res = ReadLibrryFile(CellTypes, NumberOfCellTypes, LibraryFileName, LibraryName);

	//---------------------------------------------------------------------------------------------//
	//------------------- reading the cell repoty file --------------------------------------------//

	HierarchyStruct **Hierarchy = NULL;
	int  NumberOfHierarchy;

	if (!res)
		res = ReadCellReportFile(Hierarchy, NumberOfHierarchy, InputCellReportFileName, CellTypes, NumberOfCellTypes);

	//---------------------------------------------------------------------------------------------//
	//------------------- reading the design file -------------------------------------------------//

	SignalStruct **Signals = NULL;
	int  NumberOfSignals;
	int  *Inputs = NULL;
	int  *Outputs = NULL;
	int  NumberOfInputs;
	int  NumberOfOutputs;

	CellStruct **Cells = NULL;
	int  NumberOfCells;
	int  *Gates = NULL;
	int  *Regs = NULL;
	int  NumberOfGates;
	int  NumberOfRegs;
	int  NumberOfRegValues;

	if (!res)
		res = ReadDesignFile(Signals, NumberOfSignals, Inputs, NumberOfInputs, Outputs, NumberOfOutputs,
			Cells, NumberOfCells, Gates, NumberOfGates, Regs, NumberOfRegs, NumberOfRegValues,
			InputVerilogFileName, MainModuleName, CellTypes, NumberOfCellTypes,
			Hierarchy, NumberOfHierarchy);

	//---------------------------------------------------------------------------------------------//
	//------------------- make the depth of the cells ---------------------------------------------//

	short MaxDepth;
	int   **CellsInDepth = NULL;
	int   *NumberOfCellsInDepth = NULL;

	if (!res)
		res = MakeCircuitDepth(Signals, NumberOfSignals, CellTypes, Cells,
			Gates, NumberOfGates, MaxDepth, CellsInDepth, NumberOfCellsInDepth);

	//---------------------------------------------------------------------------------------------//
	//------------------- read simulation file ----------------------------------------------------//

	int   ClockSignal;
	int   Max_No_ClockCycles;
	int   InitialSim_NumberOfClockCycles;
	int   **InitialSim_Inputs = NULL;
	char  **InitialSim_Values = NULL;
	int   InitialSim_NumberOfInputs;
	int   *EndSimCondition_Signals = NULL;
	char  *EndSimCondition_Values = NULL;
	int   EndSimCondition_NumberOfSignals;
	int   EndSim_NumberOfWaitCycles;
	char  **EndSim_OutputNames = NULL;
	int   *EndSim_Outputs_IndexL = NULL;
	int   *EndSim_Outputs_IndexH = NULL;
	char  *EndSim_Outputs_Base = NULL;
	int   EndSim_NumberOfOutputBlocks;
	int   Max_no_of_Threads;
	char  FaultInjectionType;
	int   NumberOfSimulationsInFile;
	int   NumberOfTargetClockCycles;
	int   *TargetClockCycles=NULL;
	int   MaxNumberOfFaultsPerRun;
	int   MinNumberOfFaultsPerRun;
	int   MaxNumberOfFaultsPerCycle;
	int   MinNumberOfFaultsPerCycle;
	int   NumberOfRandomInputs;
	int   *RandomInputs = NULL;

	if (!res)
		res = ReadSimulationFile(Signals, NumberOfSignals, InputSimulationFileName,
			Cells, NumberOfCells, Hierarchy, Max_no_of_Threads,
			ClockSignal, Max_No_ClockCycles, InitialSim_NumberOfClockCycles,
			InitialSim_Inputs, InitialSim_Values, InitialSim_NumberOfInputs,
			EndSimCondition_Signals, EndSimCondition_Values, EndSimCondition_NumberOfSignals,
			EndSim_NumberOfWaitCycles, EndSim_OutputNames,
			EndSim_Outputs_IndexL, EndSim_Outputs_IndexH,
			EndSim_Outputs_Base, EndSim_NumberOfOutputBlocks,
			FaultInjectionType, NumberOfSimulationsInFile,
			NumberOfTargetClockCycles, TargetClockCycles,
			MaxNumberOfFaultsPerRun, MinNumberOfFaultsPerRun,
			MaxNumberOfFaultsPerCycle, MinNumberOfFaultsPerCycle,
			NumberOfRandomInputs, RandomInputs);

	//---------------------------------------------------------------------------------------------//
	//------------------- make selected outputs ---------------------------------------------------//

	int  **EndSim_OutputsInBlock = NULL;
	int  *EndSim_NumberOfOutputsInBlock = NULL;

	if (!res)
		res = MakeSelectedOutputs(EndSim_OutputNames, EndSim_Outputs_IndexL, EndSim_Outputs_IndexH,
			EndSim_NumberOfOutputBlocks, Signals, NumberOfSignals,
			EndSim_OutputsInBlock, EndSim_NumberOfOutputsInBlock);
	
	//---------------------------------------------------------------------------------------------//
	//------------------- run simulation with fault injection -------------------------------------//

	SimulationResultStruct* SimulationResults = NULL;
	int  NumberOfSimulations;

	if (!res)
		res = RunFaultInjection(Max_no_of_Threads, Signals, NumberOfSignals,
			ClockSignal, NumberOfRegValues, Max_No_ClockCycles,
			Cells, NumberOfCells,
			FaultInjectionType, NumberOfSimulationsInFile, NumberOfTargetClockCycles, TargetClockCycles,
			MaxNumberOfFaultsPerRun, MinNumberOfFaultsPerRun, MaxNumberOfFaultsPerCycle, MinNumberOfFaultsPerCycle,
			NumberOfRandomInputs, RandomInputs,
			SummaryFileName,
			InitialSim_NumberOfClockCycles, InitialSim_NumberOfInputs,
			InitialSim_Inputs, InitialSim_Values,
			Regs, NumberOfRegs,
			MaxDepth, CellsInDepth, NumberOfCellsInDepth, CellTypes,
			EndSimCondition_Signals, EndSimCondition_Values,
			EndSimCondition_NumberOfSignals, EndSim_NumberOfWaitCycles,
			EndSim_OutputNames, EndSim_Outputs_IndexL, EndSim_Outputs_IndexH,
			EndSim_Outputs_Base, EndSim_NumberOfOutputBlocks,
			EndSim_OutputsInBlock, EndSim_NumberOfOutputsInBlock,
			SimulationResults, NumberOfSimulations);

	//---------------------------------------------------------------------------------------------//
	//------------------- print the result in file ------------------------------------------------//

	if (!res)
		PrintResultInFile(ResultFileName, NumberOfSimulations, SimulationResults);

	if (res)
	{
		printf("\nerror");
		_getch();
		return 1;
	}
	else
	{
		printf("done");
		_getch();
		return 0;
	}
}

