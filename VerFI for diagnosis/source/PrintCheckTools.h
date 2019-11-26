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

int PrintResults(int ClockCycle, int Max_No_ClockCycles, char** EndSim_OutputNames,
	int* EndSim_Outputs_IndexL, int* EndSim_Outputs_IndexH,
	char* EndSim_Outputs_Base, int EndSim_NumberOfOutputBlocks,
	SignalStruct** Signals, int NumberOfSignals,
	int* SignalValues, int &NumberOfFaultFreeOutputs, int* &FaultFreeOutputs)
{
	char *Str1 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str2 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str3 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str4 = (char *)malloc(Max_Name_Length * sizeof(char));
	int  i, j;
	int  OutputIndex;
	int  SignalIndex;
	int  IndexH, IndexL;
	int  v;

	if (ClockCycle >= Max_No_ClockCycles)
	{
		printf("simulation reached the maximum number of clock cycles\n");
		free(Str1);
		free(Str2);
		free(Str3);
		free(Str4);
		return 1;
	}

	NumberOfFaultFreeOutputs = 0;
	for (OutputIndex = 0;OutputIndex < EndSim_NumberOfOutputBlocks;OutputIndex++)
		NumberOfFaultFreeOutputs += (EndSim_Outputs_IndexH[OutputIndex] - EndSim_Outputs_IndexL[OutputIndex] + 1);

	FaultFreeOutputs = (int *)malloc(NumberOfFaultFreeOutputs * sizeof(int));
	NumberOfFaultFreeOutputs = 0;

	printf("Results after %d clock cycles\n", ClockCycle);

	for (OutputIndex = 0;OutputIndex < EndSim_NumberOfOutputBlocks;OutputIndex++)
	{
		IndexL = EndSim_Outputs_IndexL[OutputIndex];
		IndexH = EndSim_Outputs_IndexH[OutputIndex];

		Str1[0] = 0;
		if (IndexL != -1)
			sprintf(Str1, "[%d:%d] ", IndexH, IndexL);

		sprintf(Str1, "%s%s = %d'", Str1, EndSim_OutputNames[OutputIndex], IndexH - IndexL + 1);

		if (EndSim_Outputs_Base[OutputIndex] == 16)
			sprintf(Str1, "%sh", Str1);
		else if (EndSim_Outputs_Base[OutputIndex] == 2)
			sprintf(Str1, "%sb", Str1);

		Str3[0] = 0;
		v = 0;

		for (j = IndexL;j <= IndexH;j++)
		{
			if (IndexL != -1)
				sprintf(Str2, "%s[%d]", EndSim_OutputNames[OutputIndex], j);
			else
				sprintf(Str2, "%s", EndSim_OutputNames[OutputIndex]);

			for (SignalIndex = 0;SignalIndex < NumberOfSignals;SignalIndex++)
			{
				if (!strcmp(Signals[SignalIndex]->Name, Str2))
					break;
			}

			if (SignalIndex >= NumberOfSignals)
			{
				printf("simulation: signal ""%s"" as output signal not found", Str2);
				free(Str1);
				free(Str2);
				free(Str3);
				free(Str4);
				return 1;
			}

			if (EndSim_Outputs_Base[OutputIndex] == 16)
			{
				FaultFreeOutputs[NumberOfFaultFreeOutputs++] = SignalIndex;

				i = (j - IndexL) % 4;
				v |= SignalValues[SignalIndex] << i;

				if ((i == 3) | (j == IndexH))
				{
					sprintf(Str4, "%x%s", v, Str3);
					strcpy(Str3, Str4);
					v = 0;
				}
			}
			else if (EndSim_Outputs_Base[OutputIndex] == 2)
			{
				FaultFreeOutputs[NumberOfFaultFreeOutputs++] = SignalIndex;
				sprintf(Str4, "%d%s", SignalValues[SignalIndex], Str3);
				strcpy(Str3, Str4);
			}
		}

		sprintf(Str1, "%s%s", Str1, Str3);

		printf(Str1);
		printf("\n");
	}

	free(Str1);
	free(Str2);
	free(Str3);
	free(Str4);
	return 0;
}

//***************************************************************************************

void CheckResults(int ClockCycle, int Max_No_ClockCycles, char** EndSim_OutputNames,
	int* EndSim_Outputs_IndexL, int* EndSim_Outputs_IndexH,
	char* EndSim_Outputs_Base, int EndSim_NumberOfOutputBlocks,
	SignalStruct** Signals, int NumberOfSignals,
	int* FaultFreeSignalValues, int NumberOfFaultFreeOutputs, int* FaultFreeOutputs,
	int* SignalValues, SimulationResultStruct &SimulationResult,
	char Print_Nondetected, char Print_Detected, char Print_Ineffective,
	int &IneffectiveCounter, int &NondetectedCounter, int &DetectedCounter, int &RunTimeOverCounter)
{
	char *Str1 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str2 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str3 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str4 = (char *)malloc(Max_Name_Length * sizeof(char));
	int  i, j;
	int  OutputIndex;
	int  SignalIndex;
	int  IndexH, IndexL;
	int  v;
	char Check;

	if (ClockCycle >= Max_No_ClockCycles)
	{
		SimulationResult.Result = Result_RunTimeOver;
		RunTimeOverCounter++;
	}
	else
	{
		Check = 0;
		for (OutputIndex = 0;OutputIndex < NumberOfFaultFreeOutputs;OutputIndex++)
			if (SignalValues[FaultFreeOutputs[OutputIndex]] != FaultFreeSignalValues[FaultFreeOutputs[OutputIndex]])
			{
				Check = 1;
				break;
			}

		if (Check == 0)      // fault was ineffective
		{
			SimulationResult.Result = Result_Ineffective;
			IneffectiveCounter++;
		}
		else
		{
			for (OutputIndex = 0; OutputIndex < NumberOfFaultFreeOutputs; OutputIndex++)
				if (SignalValues[FaultFreeOutputs[OutputIndex]] != 0)
				{
					Check = 2;
					break;
				}

			if (Check == 1) // fault was detected
			{
				SimulationResult.Result = Result_Detected;
				DetectedCounter++;
			}
			else                 // fault was not detected
			{
				SimulationResult.Result = Result_Nondetected;
				NondetectedCounter++;
			}
		}
	}
	
	if ((SimulationResult.Result == Result_Nondetected) & Print_Nondetected)
	{
		for (OutputIndex = 0;OutputIndex < EndSim_NumberOfOutputBlocks;OutputIndex++)
		{
			IndexL = EndSim_Outputs_IndexL[OutputIndex];
			IndexH = EndSim_Outputs_IndexH[OutputIndex];

			Str1[0] = 0;
			if (IndexL != -1)
				sprintf(Str1, "[%d:%d] ", IndexH, IndexL);

			sprintf(Str1, "%s%s = %d'", Str1, EndSim_OutputNames[OutputIndex], IndexH - IndexL + 1);

			if (EndSim_Outputs_Base[OutputIndex] == 16)
				sprintf(Str1, "%sh", Str1);
			else if (EndSim_Outputs_Base[OutputIndex] == 2)
				sprintf(Str1, "%sb", Str1);

			Str3[0] = 0;
			v = 0;

			for (j = IndexL;j <= IndexH;j++)
			{
				if (IndexL != -1)
					sprintf(Str2, "%s[%d]", EndSim_OutputNames[OutputIndex], j);
				else
					sprintf(Str2, "%s", EndSim_OutputNames[OutputIndex]);

				for (SignalIndex = 0;SignalIndex < NumberOfSignals;SignalIndex++)
				{
					if (!strcmp(Signals[SignalIndex]->Name, Str2))
						break;
				}

				if (EndSim_Outputs_Base[OutputIndex] == 16)
				{
					i = (j - IndexL) % 4;
					v |= SignalValues[SignalIndex] << i;

					if ((i == 3) | (j == IndexH))
					{
						sprintf(Str4, "%x%s", v, Str3);
						strcpy(Str3, Str4);
						v = 0;
					}
				}
				else if (EndSim_Outputs_Base[OutputIndex] == 2)
				{
					sprintf(Str4, "%d%s", SignalValues[SignalIndex], Str3);
					strcpy(Str3, Str4);
				}
			}

			sprintf(Str1, "%s%s\n", Str1, Str3);

			SimulationResult.Output = (char *)malloc(strlen(Str1) * sizeof(char));
			strcpy(SimulationResult.Output, Str1);
		}
	}

	free(Str1);
	free(Str2);
	free(Str3);
	free(Str4);
	return;
}

//***************************************************************************************

void PrintResultInFile(char Print_Nondetected, char Print_Detected, char Print_Ineffective, char Print_RunTimeOver, 
	char* NondetectedFileName, char* DetectedFileName, char* IneffectiveFileName, char* RunTimeOverFileName,
	int NumberOfSimulations, SimulationResultStruct* SimulationResults, CellStruct** Cells, HierarchyStruct** Hierarchy)
{
	FILE *NondetectedFile = NULL;
	FILE *DetectedFile = NULL;
	FILE *IneffectiveFile = NULL;
	FILE *RunTimeOverFile = NULL;
	int  SimulationIndex;

	if (Print_Nondetected)
		NondetectedFile = fopen(NondetectedFileName, "wt");

	if (Print_Detected)
		DetectedFile = fopen(DetectedFileName, "wt");

	if (Print_Ineffective)
		IneffectiveFile = fopen(IneffectiveFileName, "wt");

	if (Print_RunTimeOver)
		RunTimeOverFile = fopen(RunTimeOverFileName, "wt");

	for (SimulationIndex = 0;SimulationIndex < NumberOfSimulations; SimulationIndex++)
	{
		if (SimulationResults[SimulationIndex].Valid == 1)
		{
			if (Print_Nondetected & (SimulationResults[SimulationIndex].Result == Result_Nondetected))
			{
				for (int i = 0;i < SimulationResults[SimulationIndex].NumberOfInjectedFaults;i++)
					fprintf(NondetectedFile, "cycle: %3d   cell: %s  hierarchy: %s\n",
						SimulationResults[SimulationIndex].TaregtClockCycles[i],
						Cells[SimulationResults[SimulationIndex].TaregtCells[i]]->Name,
						Hierarchy[Cells[SimulationResults[SimulationIndex].TaregtCells[i]]->Hierarchy]->Name);

				fprintf(NondetectedFile, "%s\n", SimulationResults[SimulationIndex].Output);
			}

			if (Print_Detected & (SimulationResults[SimulationIndex].Result == Result_Detected))
			{
				for (int i = 0;i < SimulationResults[SimulationIndex].NumberOfInjectedFaults;i++)
					fprintf(DetectedFile, "cycle: %3d   cell: %s  hierarchy: %s\n",
						SimulationResults[SimulationIndex].TaregtClockCycles[i],
						Cells[SimulationResults[SimulationIndex].TaregtCells[i]]->Name,
						Hierarchy[Cells[SimulationResults[SimulationIndex].TaregtCells[i]]->Hierarchy]->Name);

				fprintf(DetectedFile, "\n");
			}

			if (Print_Ineffective & (SimulationResults[SimulationIndex].Result == Result_Ineffective))
			{
				for (int i = 0;i < SimulationResults[SimulationIndex].NumberOfInjectedFaults;i++)
					fprintf(IneffectiveFile, "cycle: %3d   cell: %s  hierarchy: %s\n",
						SimulationResults[SimulationIndex].TaregtClockCycles[i],
						Cells[SimulationResults[SimulationIndex].TaregtCells[i]]->Name,
						Hierarchy[Cells[SimulationResults[SimulationIndex].TaregtCells[i]]->Hierarchy]->Name);

				fprintf(IneffectiveFile, "\n");
			}

			if (Print_RunTimeOver & (SimulationResults[SimulationIndex].Result == Result_RunTimeOver))
			{
				for (int i = 0;i < SimulationResults[SimulationIndex].NumberOfInjectedFaults;i++)
					fprintf(RunTimeOverFile, "cycle: %3d   cell: %s  hierarchy: %s\n",
						SimulationResults[SimulationIndex].TaregtClockCycles[i],
						Cells[SimulationResults[SimulationIndex].TaregtCells[i]]->Name,
						Hierarchy[Cells[SimulationResults[SimulationIndex].TaregtCells[i]]->Hierarchy]->Name);

				fprintf(RunTimeOverFile, "\n");
			}
		}
	}

	if (Print_Nondetected)
		fclose(NondetectedFile);

	if (Print_Detected)
		fclose(DetectedFile);

	if (Print_Ineffective)
		fclose(IneffectiveFile);

	if (Print_RunTimeOver)
		fclose(RunTimeOverFile);
}

//***************************************************************************************
