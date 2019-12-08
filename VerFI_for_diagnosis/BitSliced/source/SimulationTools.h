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

//***************************************************************************************

void FillTables64(uint64_t OneIn64[64], uint64_t ZeroIn64[64])
{
	char i;

	OneIn64[0] = 1;
	ZeroIn64[0] = ~OneIn64[0];
	for (i = 1;i < 64;i++)
	{
		OneIn64[i] = OneIn64[i - 1] << 1;
		ZeroIn64[i] = ~OneIn64[i];
	}
}

//***************************************************************************************

uint64_t EvaluateOperation(OperationStruct Operation, uint64_t *Values, char Intermediate)
{
	uint64_t	Res;
	char		i, j;

	for (i = 0;i < Operation.NumberOfClauses;i++)
	{
		Res = Values[Operation.OperandsInClause[i][0]];

		if (Operation.OperationOfClause[i] == Operation_AND)
			for (j = 1;j < Operation.NumberOfOperandsInClause[i];j++)
				Res &= Values[Operation.OperandsInClause[i][j]];
		else if (Operation.OperationOfClause[i] == Operation_OR)
			for (j = 1;j < Operation.NumberOfOperandsInClause[i];j++)
				Res |= Values[Operation.OperandsInClause[i][j]];
		else if (Operation.OperationOfClause[i] == Operation_XOR)
			for (j = 1;j < Operation.NumberOfOperandsInClause[i];j++)
				Res ^= Values[Operation.OperandsInClause[i][j]];
		else
			Res = ~Res;

		Values[Intermediate + i] = Res;
	}

	return Res;
}

//***************************************************************************************

void RunSimulation_and_Check(SignalStruct** Signals, int ClockSignal, int Max_No_ClockCycles,
	int InitialSim_NumberOfClockCycles, int InitialSim_NumberOfInputs,
	int** InitialSim_Inputs, uint64_t** InitialSim_Values,
	CellStruct** Cells, int* Regs, int NumberOfRegs,
	short MaxDepth, int** CellsInDepth, int* NumberOfCellsInDepth, CellTypeStruct** CellTypes,
	int* EndSimCondition_Signals, uint64_t* EndSimCondition_Values,
	int EndSimCondition_NumberOfSignals, int EndSim_NumberOfWaitCycles,
	uint64_t* SignalValues, uint64_t* RegValues, uint64_t*** Faults,
	uint64_t* OneIn64, char BufferSize, int* ClockCyclesTook, 
	char Check, char** EndSim_OutputNames,
	int* EndSim_Outputs_IndexL, int* EndSim_Outputs_IndexH,
	char* EndSim_Outputs_Base, int EndSim_NumberOfOutputBlocks,
	int NumberOfSignals, uint64_t* FaultFreeSignalValues, int NumberOfFaultFreeOutputs, int* FaultFreeOutputs,
	SimulationResultStruct* SimulationResults, int* SimulationIndexBuffer, char Print_Nondetected, 
	int &IneffectiveCounter, int &NondetectedCounter, int &DetectedCounter, int &RunTimeOverCounter)
{
	int			i;
	int			InputIndex;
	int			OutputIndex;
	int			SignalIndex;
	int			RegIndex;
	int			DepthIndex;
	int			CellIndex;
	int			ClockCycle;
	uint64_t	Value;
	int			NumberOfWaitedClockCycles[64];
	uint64_t	InputVector[100];
	uint64_t	Mask;
	char		BitIndex;

	for (BitIndex = 0;BitIndex < BufferSize;BitIndex++)
		NumberOfWaitedClockCycles[BitIndex] = -1;

	for (ClockCycle = 0;ClockCycle < Max_No_ClockCycles;ClockCycle++)
	{
		SignalValues[ClockSignal] = FullOne;

		// ----------- evaluate the registers

		for (RegIndex = 0;RegIndex < NumberOfRegs;RegIndex++)
		{
			for (InputIndex = 0;InputIndex < Cells[Regs[RegIndex]]->NumberOfInputs;InputIndex++)
				InputVector[InputIndex] = SignalValues[Cells[Regs[RegIndex]]->Inputs[InputIndex]];

			for (OutputIndex = 0;OutputIndex < Cells[Regs[RegIndex]]->NumberOfOutputs;OutputIndex++)
				InputVector[Cells[Regs[RegIndex]]->NumberOfInputs + OutputIndex] = RegValues[Cells[Regs[RegIndex]]->RegValueIndexes[OutputIndex]];

			for (OutputIndex = 0;OutputIndex < Cells[Regs[RegIndex]]->NumberOfOutputs;OutputIndex++)
			{
				Value = EvaluateOperation(CellTypes[Cells[Regs[RegIndex]]->Type]->Operations[OutputIndex], InputVector, CellTypes[Cells[Regs[RegIndex]]->Type]->Intermediate);
				Value ^= Faults[FaultInjection_toggle][ClockCycle][Regs[RegIndex]];
				Value |= Faults[FaultInjection_stuck_at_1][ClockCycle][Regs[RegIndex]];
				Value &= ~Faults[FaultInjection_stuck_at_0][ClockCycle][Regs[RegIndex]];
				RegValues[Cells[Regs[RegIndex]]->RegValueIndexes[OutputIndex]] = Value;
			}
		}

		// ----------- applying the initial inputs

		if (ClockCycle < InitialSim_NumberOfClockCycles)
			for (InputIndex = 0;InputIndex < InitialSim_NumberOfInputs;InputIndex++)
				SignalValues[InitialSim_Inputs[ClockCycle][InputIndex]] = InitialSim_Values[ClockCycle][InputIndex];

		// ----------- applying the register outputs to the output signals

		for (RegIndex = 0;RegIndex < NumberOfRegs;RegIndex++)
			for (OutputIndex = 0;OutputIndex < Cells[Regs[RegIndex]]->NumberOfOutputs;OutputIndex++)
				if (Cells[Regs[RegIndex]]->Outputs[OutputIndex] != -1)
					SignalValues[Cells[Regs[RegIndex]]->Outputs[OutputIndex]] = RegValues[Cells[Regs[RegIndex]]->RegValueIndexes[OutputIndex]];

		// ----------- evaluate the circuits :D

		for (DepthIndex = 1;DepthIndex <= MaxDepth;DepthIndex++)
		{
			for (i = 0;i < NumberOfCellsInDepth[DepthIndex];i++)
			{
				CellIndex = CellsInDepth[DepthIndex][i];

				for (InputIndex = 0;InputIndex < Cells[CellIndex]->NumberOfInputs;InputIndex++)
					InputVector[InputIndex] = SignalValues[Cells[CellIndex]->Inputs[InputIndex]];

				for (OutputIndex = 0;OutputIndex < Cells[CellIndex]->NumberOfOutputs;OutputIndex++)
					if (Cells[CellIndex]->Outputs[OutputIndex] != -1)
					{
						Value = EvaluateOperation(CellTypes[Cells[CellIndex]->Type]->Operations[OutputIndex], InputVector, CellTypes[Cells[CellIndex]->Type]->Intermediate);
						Value ^= Faults[FaultInjection_toggle][ClockCycle][CellIndex];
						Value |= Faults[FaultInjection_stuck_at_1][ClockCycle][CellIndex];
						Value &= ~Faults[FaultInjection_stuck_at_0][ClockCycle][CellIndex];
						SignalValues[Cells[CellIndex]->Outputs[OutputIndex]] = Value;
					}
			}
		}

		SignalValues[ClockSignal] = 0;

		// re-evaluate (we don't need it in this design since it works only at possitive edge of the clock and does not have a latch
		//
		//
		//

		// ----------- check the conditions to terminate the simulation

		if (ClockCycle > InitialSim_NumberOfClockCycles)
		{
			Mask = 0;
			for (SignalIndex = 0;SignalIndex < EndSimCondition_NumberOfSignals;SignalIndex++)
				Mask |= SignalValues[EndSimCondition_Signals[SignalIndex]] ^ EndSimCondition_Values[SignalIndex];

			for (BitIndex = 0;BitIndex < BufferSize;BitIndex++)
			{
				if (NumberOfWaitedClockCycles[BitIndex] == -1)
				{
					if (!(Mask & OneIn64[BitIndex]))
						NumberOfWaitedClockCycles[BitIndex] = 0;
				}
				else
					NumberOfWaitedClockCycles[BitIndex]++;

				if (NumberOfWaitedClockCycles[BitIndex] == EndSim_NumberOfWaitCycles)
				{
					ClockCyclesTook[BitIndex] = ClockCycle;
					if (Check)
						CheckResults(ClockCyclesTook[BitIndex], Max_No_ClockCycles, EndSim_OutputNames,
							EndSim_Outputs_IndexL, EndSim_Outputs_IndexH,
							EndSim_Outputs_Base, EndSim_NumberOfOutputBlocks,
							Signals, NumberOfSignals,
							FaultFreeSignalValues, NumberOfFaultFreeOutputs, FaultFreeOutputs,
							SignalValues, SimulationResults[SimulationIndexBuffer[BitIndex]], BitIndex,
							OneIn64, Print_Nondetected, IneffectiveCounter, NondetectedCounter, DetectedCounter, RunTimeOverCounter);
					else
						return;
				}
				else if ((ClockCycle == (Max_No_ClockCycles - 1)) & (NumberOfWaitedClockCycles[BitIndex] < EndSim_NumberOfWaitCycles))
				{
					ClockCyclesTook[BitIndex] = ClockCycle + 1;
					if (Check)
						CheckResults(ClockCyclesTook[BitIndex], Max_No_ClockCycles, EndSim_OutputNames,
							EndSim_Outputs_IndexL, EndSim_Outputs_IndexH,
							EndSim_Outputs_Base, EndSim_NumberOfOutputBlocks,
							Signals, NumberOfSignals,
							FaultFreeSignalValues, NumberOfFaultFreeOutputs, FaultFreeOutputs,
							SignalValues, SimulationResults[SimulationIndexBuffer[BitIndex]], BitIndex,
							OneIn64, Print_Nondetected, IneffectiveCounter, NondetectedCounter, DetectedCounter, RunTimeOverCounter);
				}
			}
		}
	}
}

//***************************************************************************************

int RunFaultInjection(int Max_no_of_Threads, SignalStruct** Signals, int NumberOfSignals,
	int ClockSignal, int NumberOfRegValues, int Max_No_ClockCycles,
	CellStruct** Cells, int NumberOfCells, char FaultInjectionType,
	char FaultInjectionMethod, int NumberOfSimulationsInFile, int NumberOfTargetClockCycles, int* TargetClockCycles,
	int MaxNumberOfFaultsPerRun, int MinNumberOfFaultsPerRun, int MaxNumberOfFaultsPerCycle, int MinNumberOfFaultsPerCycle,
	char* EvaluationResultFileName,
	int InitialSim_NumberOfClockCycles, int InitialSim_NumberOfInputs,
	int** InitialSim_Inputs, uint64_t** InitialSim_Values,
	int* Regs, int NumberOfRegs,
	short MaxDepth, int** CellsInDepth, int* NumberOfCellsInDepth, CellTypeStruct** CellTypes,
	int* EndSimCondition_Signals, uint64_t* EndSimCondition_Values,
	int EndSimCondition_NumberOfSignals, int EndSim_NumberOfWaitCycles,
	char** EndSim_OutputNames, int* EndSim_Outputs_IndexL, int* EndSim_Outputs_IndexH,
	char* EndSim_Outputs_Base, int EndSim_NumberOfOutputBlocks,
	uint64_t* FaultFreeSignalValues, int NumberOfFaultFreeOutputs, int* FaultFreeOutputs,
	char Print_Nondetected, char Print_Detected, char Print_Ineffective, char Print_RunTimeOver,
	SimulationResultStruct* &SimulationResults, int &NumberOfSimulations, 
	uint64_t* OneIn64, uint64_t* ZeroIn64)
{
	int			CellIndex;
	int			*FaultAllowedCells;
	int			NumberOfFaultAllowedCells;
	int			ClockCycle;
	int			ClockCyclesTook[64];
	uint64_t	**SignalValues = NULL;
	uint64_t	**RegValues = NULL;
	uint64_t	****Faults = NULL;
	int			ThreadIndex;
	int			SimulationIndex;
	int			SimulationCounter;
	int			RangeNumberOfFaultsPerCycle;
	int			RangeNumberOfFaultsPerRun;
	int			*DetectedCounter;
	int			*NondetectedCounter;
	int			*IneffectiveCounter;
	int			*RunTimeOverCounter;
	FILE		*EvaluationResultFile;
	int			TotalDetected;
	int			TotalNondetected;
	int			TotalIneffective;
	int			TotalRunTimeOver;
	int			LocalIndex;
	int			i, j, k;
	char		ValidSimulation;
	char		NumberOfInjectedFaults;
	char		NumberOfFaultsInCycle;
	int			SelectedNumberOfInjectedFaults;
	char		*Seeded;
	int			MaxTargetClockCycle;
	int			MinTargetClockCycle;
	char		*TargetClockCycleValid;
	int			ClockCycleIndex;
	clock_t		begin;
	char		*BitIndex;
	int			**SimulationIndexBuffer;

	NumberOfFaultAllowedCells = 0;
	for (CellIndex = 0;CellIndex < NumberOfCells;CellIndex++)
		if (Cells[CellIndex]->FaultAllowed)
			NumberOfFaultAllowedCells++;

	FaultAllowedCells = (int*)malloc(NumberOfFaultAllowedCells * sizeof(int));
	NumberOfFaultAllowedCells = 0;
	for (CellIndex = 0;CellIndex < NumberOfCells;CellIndex++)
		if (Cells[CellIndex]->FaultAllowed)
			FaultAllowedCells[NumberOfFaultAllowedCells++] = CellIndex;

	SignalValues = (uint64_t **)malloc(Max_no_of_Threads * sizeof(uint64_t *));
	RegValues = (uint64_t **)malloc(Max_no_of_Threads * sizeof(uint64_t *));
	Faults = (uint64_t ****)malloc(Max_no_of_Threads * sizeof(uint64_t ***));
	DetectedCounter = (int *)calloc(Max_no_of_Threads, sizeof(int));
	NondetectedCounter = (int *)calloc(Max_no_of_Threads, sizeof(int));
	IneffectiveCounter = (int *)calloc(Max_no_of_Threads, sizeof(int));
	RunTimeOverCounter = (int *)calloc(Max_no_of_Threads, sizeof(int));
	Seeded = (char *)calloc(Max_no_of_Threads, sizeof(char));
	BitIndex = (char*)calloc(Max_no_of_Threads, sizeof(char));
	SimulationIndexBuffer = (int**)malloc(Max_no_of_Threads * sizeof(int*));

	for (ThreadIndex = 0;ThreadIndex < Max_no_of_Threads;ThreadIndex++)
	{
		SignalValues[ThreadIndex] = (uint64_t*)calloc(NumberOfSignals, sizeof(uint64_t));
		RegValues[ThreadIndex] = (uint64_t*)calloc(NumberOfRegValues, sizeof(uint64_t));
		Faults[ThreadIndex] = (uint64_t***)malloc(NumberOfFaultInjectionTypes * sizeof(uint64_t**));
		SignalValues[ThreadIndex][1] = FullOne;    // constant 1'b1
		SimulationIndexBuffer[ThreadIndex] = (int*)malloc(64 * sizeof(int));

		for (i = 0;i < NumberOfFaultInjectionTypes;i++)
		{
			Faults[ThreadIndex][i] = (uint64_t**)malloc(Max_No_ClockCycles * sizeof(uint64_t*));

			for (ClockCycle = 0;ClockCycle < Max_No_ClockCycles;ClockCycle++)
				Faults[ThreadIndex][i][ClockCycle] = (uint64_t*)calloc(NumberOfCells, sizeof(uint64_t));
		}
	}

	TargetClockCycleValid = (char*)calloc(Max_No_ClockCycles, sizeof(char));

	MaxTargetClockCycle = TargetClockCycles[0];
	MinTargetClockCycle = TargetClockCycles[0];

	for (ClockCycleIndex = 0;ClockCycleIndex < NumberOfTargetClockCycles;ClockCycleIndex++)
	{
		TargetClockCycleValid[TargetClockCycles[ClockCycleIndex]] = 1;

		if (MaxTargetClockCycle < TargetClockCycles[ClockCycleIndex])
			MaxTargetClockCycle = TargetClockCycles[ClockCycleIndex];

		if (MinTargetClockCycle > TargetClockCycles[ClockCycleIndex])
			MinTargetClockCycle = TargetClockCycles[ClockCycleIndex];
	}

	if (FaultInjectionMethod == FaultInjection_Exhaustive)
		NumberOfSimulations = (int)pow(NumberOfFaultAllowedCells, MinNumberOfFaultsPerCycle * NumberOfTargetClockCycles);
	else  // FaultInjection_Random
		NumberOfSimulations = NumberOfSimulationsInFile;

	if (NumberOfSimulations > 600000000L)
	{
		printf("Number of simulations %d is over the threshold", NumberOfSimulations);
		_getch();
		return 1;
	}
	else
	{
		if (FaultInjectionMethod == FaultInjection_Exhaustive)
			printf("Number of cells to be faulty: %d\n", NumberOfFaultAllowedCells);

		printf("Number of simulations: %d\n", NumberOfSimulations);
	}

	SimulationResults = (SimulationResultStruct *)malloc(NumberOfSimulations * sizeof(SimulationResultStruct));

	omp_set_num_threads(Max_no_of_Threads);
	RangeNumberOfFaultsPerCycle = MaxNumberOfFaultsPerCycle - MinNumberOfFaultsPerCycle + 1;
	RangeNumberOfFaultsPerRun = MaxNumberOfFaultsPerRun - MinNumberOfFaultsPerRun + 1;
	SimulationCounter = 0;
	EvaluationResultFile = fopen(EvaluationResultFileName, "wt");
	begin = clock();

	#pragma omp parallel for schedule(guided) private(ThreadIndex, ClockCycleIndex, ClockCycle, ClockCyclesTook, i, j, k, LocalIndex, SelectedNumberOfInjectedFaults, NumberOfInjectedFaults, NumberOfFaultsInCycle, ValidSimulation, TotalDetected, TotalNondetected, TotalIneffective, TotalRunTimeOver)
	for (SimulationIndex = 0; SimulationIndex < NumberOfSimulations; SimulationIndex++)
	{
		ThreadIndex = omp_get_thread_num();
		if (!Seeded[ThreadIndex])
		{
			srand(int(time(NULL)) ^ ThreadIndex);
			Seeded[ThreadIndex] = 1;
		}

		SimulationResults[SimulationIndex].TaregtCells = (int *)malloc(MaxNumberOfFaultsPerRun * sizeof(int));
		SimulationResults[SimulationIndex].TaregtClockCycles = (int *)malloc(MaxNumberOfFaultsPerRun * sizeof(int));

		if (FaultInjectionMethod == FaultInjection_Exhaustive)
		{
			NumberOfInjectedFaults = NumberOfTargetClockCycles * MinNumberOfFaultsPerCycle;
			ValidSimulation = 1;
			LocalIndex = SimulationIndex;

			for (ClockCycleIndex = 0; (ClockCycleIndex < NumberOfTargetClockCycles) & ValidSimulation; ClockCycleIndex++)
			{
				k = ClockCycleIndex * MinNumberOfFaultsPerCycle;

				for (i = 0; (i < MinNumberOfFaultsPerCycle) & ValidSimulation; i++)
				{
					SimulationResults[SimulationIndex].TaregtCells[k + i] = FaultAllowedCells[LocalIndex % NumberOfFaultAllowedCells];
					LocalIndex = (LocalIndex - (LocalIndex % NumberOfFaultAllowedCells)) / NumberOfFaultAllowedCells;
					SimulationResults[SimulationIndex].TaregtClockCycles[k + i] = TargetClockCycles[ClockCycleIndex];

					for (j = 0; (j < i) & ValidSimulation; j++)
						if (SimulationResults[SimulationIndex].TaregtCells[k + i] >= SimulationResults[SimulationIndex].TaregtCells[k + j])
							ValidSimulation = 0;
				}
			}
		}
		else //FaultInjection_Random
		{
			NumberOfInjectedFaults = 0;
			ValidSimulation = 1;
			SelectedNumberOfInjectedFaults = MinNumberOfFaultsPerRun + (rand() % RangeNumberOfFaultsPerRun);

			while (NumberOfInjectedFaults < SelectedNumberOfInjectedFaults)
			{
				do {
					ClockCycleIndex = rand() % NumberOfTargetClockCycles;
					ClockCycle = TargetClockCycles[ClockCycleIndex];

					for (j = 0; j < NumberOfInjectedFaults; j++)
						if (SimulationResults[SimulationIndex].TaregtClockCycles[j] == ClockCycle)
							break;
				} while (j < NumberOfInjectedFaults);

				NumberOfFaultsInCycle = MinNumberOfFaultsPerCycle + (rand() % RangeNumberOfFaultsPerCycle);

				for (i = 0; (i < NumberOfFaultsInCycle) & (NumberOfInjectedFaults < MaxNumberOfFaultsPerRun); i++)
				{
					SimulationResults[SimulationIndex].TaregtCells[NumberOfInjectedFaults] = rand() % NumberOfCells;

					if (Cells[SimulationResults[SimulationIndex].TaregtCells[NumberOfInjectedFaults]]->FaultAllowed)
					{
						SimulationResults[SimulationIndex].TaregtClockCycles[NumberOfInjectedFaults] = ClockCycle;

						for (j = 0; j < i; j++)
							if (SimulationResults[SimulationIndex].TaregtCells[NumberOfInjectedFaults] ==
								SimulationResults[SimulationIndex].TaregtCells[NumberOfInjectedFaults - j - 1])
								break;

						if (j < i)
							i--;
						else
							NumberOfInjectedFaults++;
					}
					else
						i--;
				}
			}
		}

		SimulationResults[SimulationIndex].Valid = ValidSimulation;

		if (ValidSimulation)
		{
			SimulationResults[SimulationIndex].NumberOfInjectedFaults = NumberOfInjectedFaults;
			for (i = 0; i < NumberOfInjectedFaults; i++)
				Faults[ThreadIndex][FaultInjectionType][SimulationResults[SimulationIndex].TaregtClockCycles[i]][SimulationResults[SimulationIndex].TaregtCells[i]] |= OneIn64[BitIndex[ThreadIndex]];

			SimulationIndexBuffer[ThreadIndex][BitIndex[ThreadIndex]] = SimulationIndex;
			BitIndex[ThreadIndex]++;

			if (BitIndex[ThreadIndex] == 64)
			{

				RunSimulation_and_Check(Signals, ClockSignal, Max_No_ClockCycles,
					InitialSim_NumberOfClockCycles, InitialSim_NumberOfInputs,
					InitialSim_Inputs, InitialSim_Values,
					Cells, Regs, NumberOfRegs,
					MaxDepth, CellsInDepth, NumberOfCellsInDepth, CellTypes,
					EndSimCondition_Signals, EndSimCondition_Values,
					EndSimCondition_NumberOfSignals, EndSim_NumberOfWaitCycles,
					SignalValues[ThreadIndex], RegValues[ThreadIndex], Faults[ThreadIndex],
					OneIn64, 64, ClockCyclesTook,
					1, EndSim_OutputNames, EndSim_Outputs_IndexL, EndSim_Outputs_IndexH,
					EndSim_Outputs_Base, EndSim_NumberOfOutputBlocks,
					NumberOfSignals, FaultFreeSignalValues,
					NumberOfFaultFreeOutputs, FaultFreeOutputs, 
					SimulationResults, SimulationIndexBuffer[ThreadIndex], Print_Nondetected, 
					IneffectiveCounter[ThreadIndex], NondetectedCounter[ThreadIndex], DetectedCounter[ThreadIndex], RunTimeOverCounter[ThreadIndex]);

				for (j = 0; j < 64; j++)
					for (i = 0; i < SimulationResults[SimulationIndexBuffer[ThreadIndex][j]].NumberOfInjectedFaults; i++)
						Faults[ThreadIndex][FaultInjectionType][SimulationResults[SimulationIndexBuffer[ThreadIndex][j]].TaregtClockCycles[i]][SimulationResults[SimulationIndexBuffer[ThreadIndex][j]].TaregtCells[i]] &= ZeroIn64[j];

				BitIndex[ThreadIndex] = 0;
			}

			#pragma omp atomic
			SimulationCounter++;

			if ((SimulationCounter & 0x3fff) == 0x3fff)
			{
				TotalDetected = 0;
				TotalNondetected = 0;
				TotalIneffective = 0;
				TotalRunTimeOver = 0;

				for (i = 0; i < Max_no_of_Threads; i++)
				{
					TotalDetected += DetectedCounter[i];
					TotalNondetected += NondetectedCounter[i];
					TotalIneffective += IneffectiveCounter[i];
					TotalRunTimeOver += RunTimeOverCounter[i];
				}

				int    elapsed_secs = int(double(clock() - begin) / CLOCKS_PER_SEC);
				char   Str1[200];

				sprintf(Str1, "%04d:%02d Total: %d  Ineffective: %d  Detected: %d  Non-detected: %d  RunTimeOver: %d\n", elapsed_secs / 60, elapsed_secs % 60,
					SimulationCounter, TotalIneffective, TotalDetected, TotalNondetected, TotalRunTimeOver);

				printf(Str1);
				fprintf(EvaluationResultFile, Str1);
			}
		}
		else
		{
			free(SimulationResults[SimulationIndex].TaregtCells);
			free(SimulationResults[SimulationIndex].TaregtClockCycles);
		}
	}

	for (ThreadIndex = 0; ThreadIndex < Max_no_of_Threads; ThreadIndex++)
	{
		if (BitIndex[ThreadIndex])
		{
			RunSimulation_and_Check(Signals, ClockSignal, Max_No_ClockCycles,
				InitialSim_NumberOfClockCycles, InitialSim_NumberOfInputs,
				InitialSim_Inputs, InitialSim_Values,
				Cells, Regs, NumberOfRegs,
				MaxDepth, CellsInDepth, NumberOfCellsInDepth, CellTypes,
				EndSimCondition_Signals, EndSimCondition_Values,
				EndSimCondition_NumberOfSignals, EndSim_NumberOfWaitCycles,
				SignalValues[ThreadIndex], RegValues[ThreadIndex], Faults[ThreadIndex],
				OneIn64, BitIndex[ThreadIndex], ClockCyclesTook,
				1, EndSim_OutputNames, EndSim_Outputs_IndexL, EndSim_Outputs_IndexH,
				EndSim_Outputs_Base, EndSim_NumberOfOutputBlocks,
				NumberOfSignals, FaultFreeSignalValues,
				NumberOfFaultFreeOutputs, FaultFreeOutputs,
				SimulationResults, SimulationIndexBuffer[ThreadIndex], Print_Nondetected,
				IneffectiveCounter[ThreadIndex], NondetectedCounter[ThreadIndex], DetectedCounter[ThreadIndex], RunTimeOverCounter[ThreadIndex]);
		}
	}

	TotalDetected = 0;
	TotalNondetected = 0;
	TotalIneffective = 0;
	TotalRunTimeOver = 0;

	for (ThreadIndex = 0; ThreadIndex < Max_no_of_Threads; ThreadIndex++)
	{
		TotalDetected += DetectedCounter[ThreadIndex];
		TotalNondetected += NondetectedCounter[ThreadIndex];
		TotalIneffective += IneffectiveCounter[ThreadIndex];
		TotalRunTimeOver += RunTimeOverCounter[ThreadIndex];
	}

	int    elapsed_secs = int(double(clock() - begin) / CLOCKS_PER_SEC);
	char   Str1[200];

	sprintf(Str1, "%04d:%02d Total: %d  Ineffective: %d  Detected: %d  Non-detected: %d  RunTimeOver: %d\n", elapsed_secs / 60, elapsed_secs % 60,
		SimulationCounter, TotalIneffective, TotalDetected, TotalNondetected, TotalRunTimeOver);

	printf(Str1);
	fprintf(EvaluationResultFile, Str1);

	fclose(EvaluationResultFile);
	return 0;
}

//***************************************************************************************
