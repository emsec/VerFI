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

void CheckResults(int ClockCycleFaultFree, int ClockCycleFaulty, int Max_No_ClockCycles,
	char** EndSim_OutputNames, int* EndSim_Outputs_IndexL, int* EndSim_Outputs_IndexH,
	char* EndSim_Outputs_Base, int EndSim_NumberOfOutputBlocks,
	int** EndSim_OutputsInBlock, int* EndSim_NumberOfOutputsInBlock,
	SignalStruct** Signals, int NumberOfSignals, int** FaultFreeOutputValues,
	int* SignalValues, SimulationResultStruct &SimulationResult,
	int NumberOfRandomInputs, int* RandomInputs,
	int &IneffectiveCounter, int &NondetectedCounter, int &DetectedCounter, int &RunTimeOverCounter)
{
	char *Str1 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str2 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str3 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str4 = (char *)malloc(Max_Name_Length * sizeof(char));
	int  i, j;
	int  BlockIndex;
	int  OutputIndex;
	int  SignalIndex;
	int  IndexH, IndexL;
	int  v;
	char Check;

	if ((ClockCycleFaultFree >= Max_No_ClockCycles) | (ClockCycleFaulty >= Max_No_ClockCycles))
	{
		SimulationResult.Result = Result_RunTimeOver;
		RunTimeOverCounter++;
	}
	else
	{
		Check = 0;
		for (BlockIndex = 0;BlockIndex < EndSim_NumberOfOutputBlocks;BlockIndex++)
			for (OutputIndex = 0;OutputIndex < EndSim_NumberOfOutputsInBlock[BlockIndex];OutputIndex++)
				if (SignalValues[EndSim_OutputsInBlock[BlockIndex][OutputIndex]] != FaultFreeOutputValues[BlockIndex][OutputIndex])
				{
					Check = 1;
					BlockIndex = EndSim_NumberOfOutputBlocks;
					break;
				}

		if (Check == 0)      // fault was ineffective
		{
			SimulationResult.Result = Result_Ineffective;
			IneffectiveCounter++;
		}
		else
		{
			for (BlockIndex = 0; BlockIndex < EndSim_NumberOfOutputBlocks; BlockIndex++)
				for (OutputIndex = 0; OutputIndex < EndSim_NumberOfOutputsInBlock[BlockIndex]; OutputIndex++)
					if (SignalValues[EndSim_OutputsInBlock[BlockIndex][OutputIndex]] != 0)
					{
						Check = 2;
						BlockIndex = EndSim_NumberOfOutputBlocks;
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

	//--- inputs

	Str2[0] = 0;
	v = 0;
	for (j = 0;j < NumberOfRandomInputs;j++)
	{
		i = j % 4;
		v |= SignalValues[RandomInputs[j]] << i;

		if ((i == 3) | (j == (NumberOfRandomInputs - 1)))
		{
			sprintf(Str3, "%x%s", v, Str2);
			strcpy(Str2, Str3);
			v = 0;
		}
	}

	strcpy(Str1, "input: ");
	strcat(Str1, Str2);

	//---  fault-free outputs

	Str2[0] = 0;
	for (BlockIndex = 0;BlockIndex < EndSim_NumberOfOutputBlocks;BlockIndex++)
	{
		strcat(Str2, " ");
		strcat(Str2, EndSim_OutputNames[BlockIndex]);
		strcat(Str2, ": ");

		sprintf(Str2, "%s%d'%c", Str2, EndSim_NumberOfOutputsInBlock[BlockIndex],
			(EndSim_Outputs_Base[BlockIndex] == 16) ? 'h' : 'b');

		Str3[0] = 0;
		v = 0;
		for (OutputIndex = 0;OutputIndex < EndSim_NumberOfOutputsInBlock[BlockIndex];OutputIndex++)
		{
			if (EndSim_Outputs_Base[BlockIndex] == 16)
			{
				i = OutputIndex % 4;
				v |= FaultFreeOutputValues[BlockIndex][OutputIndex] << i;

				if ((i == 3) | (OutputIndex == (EndSim_NumberOfOutputsInBlock[BlockIndex] - 1)))
				{
					sprintf(Str4, "%x%s", v, Str3);
					strcpy(Str3, Str4);
					v = 0;
				}
			}
			else if (EndSim_Outputs_Base[BlockIndex] == 2)
			{
				sprintf(Str4, "%d%s", FaultFreeOutputValues[BlockIndex][OutputIndex], Str3);
				strcpy(Str3, Str4);
			}
		}

		strcat(Str2, Str3);
	}

	strcat(Str1, " (fault-free)");
	strcat(Str1, Str2);

	//---  faulty outputs

	Str2[0] = 0;
	for (BlockIndex = 0;BlockIndex < EndSim_NumberOfOutputBlocks;BlockIndex++)
	{
		strcat(Str2, " ");
		strcat(Str2, EndSim_OutputNames[BlockIndex]);
		strcat(Str2, ": ");

		sprintf(Str2, "%s%d'%c", Str2, EndSim_NumberOfOutputsInBlock[BlockIndex],
			(EndSim_Outputs_Base[BlockIndex] == 16) ? 'h' : 'b');

		Str3[0] = 0;
		v = 0;
		for (OutputIndex = 0;OutputIndex < EndSim_NumberOfOutputsInBlock[BlockIndex];OutputIndex++)
		{
			if (EndSim_Outputs_Base[BlockIndex] == 16)
			{
				i = OutputIndex % 4;
				v |= SignalValues[EndSim_OutputsInBlock[BlockIndex][OutputIndex]] << i;

				if ((i == 3) | (OutputIndex == (EndSim_NumberOfOutputsInBlock[BlockIndex] - 1)))
				{
					sprintf(Str4, "%x%s", v, Str3);
					strcpy(Str3, Str4);
					v = 0;
				}
			}
			else if (EndSim_Outputs_Base[BlockIndex] == 2)
			{
				sprintf(Str4, "%d%s", SignalValues[EndSim_OutputsInBlock[BlockIndex][OutputIndex]], Str3);
				strcpy(Str3, Str4);
			}
		}

		strcat(Str2, Str3);
	}

	strcat(Str1, " (faulty)");
	strcat(Str1, Str2);

	strcat(Str1, " ineffective: ");
	if (SimulationResult.Result == Result_Ineffective)
		strcat(Str1, "1\n");
	else
		strcat(Str1, "0\n");

	SimulationResult.Output = (char *)malloc(strlen(Str1) * sizeof(char));
	strcpy(SimulationResult.Output, Str1);

	free(Str1);
	free(Str2);
	free(Str3);
	free(Str4);
	return;
}

//***************************************************************************************

void PrintResultInFile(char* ResultFileName, int NumberOfSimulations,
	SimulationResultStruct* SimulationResults)
{
	FILE *ResultFile = NULL;
	int  SimulationIndex;

	ResultFile = fopen(ResultFileName, "wt");

	for (SimulationIndex = 0;SimulationIndex < NumberOfSimulations; SimulationIndex++)
		if (SimulationResults[SimulationIndex].Valid == 1)
			fprintf(ResultFile, "%s", SimulationResults[SimulationIndex].Output);

	fclose(ResultFile);
}

//***************************************************************************************
