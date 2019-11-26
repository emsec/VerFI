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

int ReadSimulationFile(SignalStruct** Signals, int NumberOfSignals, char* InputSimulationFileName,
	CellStruct** Cells, int NumberOfCells, HierarchyStruct** Hierarchy, int &Max_no_of_Threads,
	int &ClockSignal, int &Max_No_ClockCycles, int &InitialSim_NumberOfClockCycles,
	int** &InitialSim_Inputs, char** &InitialSim_Values, int &InitialSim_NumberOfInputs,
	int* &EndSimCondition_Signals, char* &EndSimCondition_Values, int &EndSimCondition_NumberOfSignals,
	int &EndSim_NumberOfWaitCycles, char** &EndSim_OutputNames,
	int* &EndSim_Outputs_IndexL, int* &EndSim_Outputs_IndexH,
	char* &EndSim_Outputs_Base, int &EndSim_NumberOfOutputBlocks,
	char &FaultInjectionType, 
	char &FaultInjectionMethod, int &NumberOfSimulationsInFile,
	int  &NumberOfTargetClockCycles, int* &TargetClockCycles,
	int  &MaxNumberOfFaultsPerRun, int &MinNumberOfFaultsPerRun,
	int &MaxNumberOfFaultsPerCycle, int &MinNumberOfFaultsPerCycle,
	char &Print_Nondetected, char &Print_Detected, char &Print_Ineffective, char &Print_RunTimeOver)
{
	char  *Str1 = (char *)malloc(Max_Name_Length * sizeof(char));
	char  *Str2 = (char *)malloc(Max_Name_Length * sizeof(char));
	int   i, j;
	int   InputIndex;
	int   OutputIndex;
	int   SignalIndex;
	int   IndexH, IndexL;
	FILE  *SimulationFile;
	int   SimulationFileCheckList = 0;
	int   ClockCycle;
	int   InitialSim_NumberOfInputBlocks;
	int*  Buffer_int;
	char* Buffer_char;
	int   NumberOfBuffer_char;
	char  *str_ptr;
	int   v;
	int   NumberOfItems;
	int   CellIndex;
	char  OneFound;

	SimulationFile = fopen(InputSimulationFileName, "rt");

	if (SimulationFile == NULL)
	{
		printf("simulation file ""%s"" not found\n", InputSimulationFileName);
		free(Str1);
		free(Str2);
		return 1;
	}

	do {
		ReadNonCommentFromFile(SimulationFile, Str1, "%");

		if (!strcmp(Str1, "clock"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");

			for (SignalIndex = 0;SignalIndex < NumberOfSignals;SignalIndex++)
				if (!strcmp(Signals[SignalIndex]->Name, Str1))
					break;

			if (SignalIndex >= NumberOfSignals)
			{
				printf("simulation file: signal ""%s"" as clock signal not found\n", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			ClockSignal = SignalIndex;
			SimulationFileCheckList |= (1 << 0);
		}
		else if (!strcmp(Str1, "max_clock_cycle"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			Max_No_ClockCycles = atoi(Str1);
			SimulationFileCheckList |= (1 << 1);
		}
		else if (!strcmp(Str1, "no_of_inputs"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			InitialSim_NumberOfInputBlocks = atoi(Str1);
			SimulationFileCheckList |= (1 << 2);
		}
		else if (!strcmp(Str1, "no_of_initial_clock_cycles"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			InitialSim_NumberOfClockCycles = atoi(Str1);
			InitialSim_Inputs = (int **)malloc(InitialSim_NumberOfClockCycles * sizeof(int *));
			InitialSim_Values = (char **)malloc(InitialSim_NumberOfClockCycles * sizeof(char *));

			for (ClockCycle = 0;ClockCycle < InitialSim_NumberOfClockCycles;ClockCycle++)
			{
				InitialSim_Inputs[ClockCycle] = NULL;
				InitialSim_NumberOfInputs = 0;
				InitialSim_Values[ClockCycle] = NULL;
				NumberOfBuffer_char = 0;

				for (InputIndex = 0;InputIndex < InitialSim_NumberOfInputBlocks;InputIndex++)
				{
					ReadNonCommentFromFile(SimulationFile, Str1, "%");

					if ((Str1[0] == '[') & (Str1[strlen(Str1) - 1] == ']'))
					{
						strcpy(Str2, Str1 + 1);
						Str2[strlen(Str2) - 1] = 0;
						str_ptr = strchr(Str2, ':');
						*str_ptr = 0;
						IndexH = atoi(Str2);
						IndexL = atoi(str_ptr + 1);

						ReadNonCommentFromFile(SimulationFile, Str1, "%");
					}
					else
					{
						IndexL = -1;
						IndexH = -1;
					}

					for (j = IndexL;j <= IndexH;j++)
					{
						if (IndexL != -1)
							sprintf(Str2, "%s[%d]", Str1, j);
						else
							sprintf(Str2, "%s", Str1);

						for (SignalIndex = 0;SignalIndex < NumberOfSignals;SignalIndex++)
						{
							if (!strcmp(Signals[SignalIndex]->Name, Str2))
								break;
						}

						if (SignalIndex >= NumberOfSignals)
						{
							printf("simulation file: signal ""%s"" as input signal not found\n", Str2);
							fclose(SimulationFile);
							free(Str1);
							free(Str2);
							return 1;
						}

						Buffer_int = (int  *)malloc((InitialSim_NumberOfInputs + 1) * sizeof(int));
						memcpy(Buffer_int, InitialSim_Inputs[ClockCycle], InitialSim_NumberOfInputs * sizeof(int));
						free(InitialSim_Inputs[ClockCycle]);
						InitialSim_Inputs[ClockCycle] = Buffer_int;

						InitialSim_Inputs[ClockCycle][InitialSim_NumberOfInputs] = SignalIndex;
						InitialSim_NumberOfInputs++;
					}

					ReadNonCommentFromFile(SimulationFile, Str1, "%");

					strcpy(Str2, Str1);
					str_ptr = strchr(Str2, '\'');
					*str_ptr = 0;
					j = atoi(Str2);
					if (j != (IndexH - IndexL + 1))
					{
						printf("simulation file: length ""%s"" does not match to the given size\n", Str1);
						fclose(SimulationFile);
						free(Str1);
						free(Str2);
						return 1;
					}

					strcpy(Str2, str_ptr + 1);
					if (Str2[0] == 'h')
					{
						for (j = IndexL;j <= IndexH;j += 4)
						{
							Str2[0] = Str1[strlen(Str1) - 1];
							Str2[1] = 0;
							Str1[strlen(Str1) - 1] = 0;

							Buffer_char = (char  *)malloc((NumberOfBuffer_char + 4) * sizeof(int));
							memcpy(Buffer_char, InitialSim_Values[ClockCycle], NumberOfBuffer_char * sizeof(int));
							free(InitialSim_Values[ClockCycle]);
							InitialSim_Values[ClockCycle] = Buffer_char;

							v = strtol(Str2, NULL, 16);
							for (i = 0;i < 4;i++)
							{
								InitialSim_Values[ClockCycle][NumberOfBuffer_char] = (v & (1 << i)) ? 1 : 0;
								NumberOfBuffer_char++;
							}
						}
					}
					else if (Str2[0] == 'b')
					{
						for (j = IndexL;j <= IndexH;j += 1)
						{
							Str2[0] = Str1[strlen(Str1) - 1];
							Str2[1] = 0;
							Str1[strlen(Str1) - 1] = 0;

							Buffer_char = (char  *)malloc((NumberOfBuffer_char + 1) * sizeof(int));
							memcpy(Buffer_char, InitialSim_Values[ClockCycle], NumberOfBuffer_char * sizeof(int));
							free(InitialSim_Values[ClockCycle]);
							InitialSim_Values[ClockCycle] = Buffer_char;

							v = strtol(Str2, NULL, 2);
							InitialSim_Values[ClockCycle][NumberOfBuffer_char] = v;
							NumberOfBuffer_char++;
						}
					}
					else
					{
						printf("simulation file: base not known in ""%s""\n", Str1);
						fclose(SimulationFile);
						free(Str1);
						free(Str2);
						return 1;
					}
				}

				if (NumberOfBuffer_char != InitialSim_NumberOfInputs)
				{
					printf("simulation file: something is wrong in given initial inputs\n");
					fclose(SimulationFile);
					free(Str1);
					free(Str2);
					return 1;
				}
			}

			SimulationFileCheckList |= (1 << 3);
		}
		else if (!strcmp(Str1, "end_condition"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");

			if ((Str1[0] == '[') & (Str1[strlen(Str1) - 1] == ']'))
			{
				strcpy(Str2, Str1 + 1);
				Str2[strlen(Str2) - 1] = 0;
				str_ptr = strchr(Str2, ':');
				*str_ptr = 0;
				IndexH = atoi(Str2);
				IndexL = atoi(str_ptr + 1);

				ReadNonCommentFromFile(SimulationFile, Str1, "%");
			}
			else
			{
				IndexL = -1;
				IndexH = -1;
			}

			EndSimCondition_NumberOfSignals = 0;
			NumberOfBuffer_char = 0;

			for (j = IndexL;j <= IndexH;j++)
			{
				if (IndexL != -1)
					sprintf(Str2, "%s[%d]", Str1, j);
				else
					sprintf(Str2, "%s", Str1);

				for (SignalIndex = 0;SignalIndex < NumberOfSignals;SignalIndex++)
				{
					if (!strcmp(Signals[SignalIndex]->Name, Str2))
						break;
				}

				if (SignalIndex >= NumberOfSignals)
				{
					printf("simulation file: signal ""%s"" as input signal not found\n", Str2);
					fclose(SimulationFile);
					free(Str1);
					free(Str2);
					return 1;
				}

				Buffer_int = (int  *)malloc((EndSimCondition_NumberOfSignals + 1) * sizeof(int));
				memcpy(Buffer_int, EndSimCondition_Signals, EndSimCondition_NumberOfSignals * sizeof(int));
				free(EndSimCondition_Signals);
				EndSimCondition_Signals = Buffer_int;

				EndSimCondition_Signals[EndSimCondition_NumberOfSignals] = SignalIndex;
				EndSimCondition_NumberOfSignals++;
			}

			ReadNonCommentFromFile(SimulationFile, Str1, "%");

			strcpy(Str2, Str1);
			str_ptr = strchr(Str2, '\'');
			*str_ptr = 0;
			j = atoi(Str2);
			if (j != (IndexH - IndexL + 1))
			{
				printf("simulation file: length ""%s"" does not match to the given size\n", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			strcpy(Str2, str_ptr + 1);
			if (Str2[0] == 'h')
			{
				for (j = IndexL;j <= IndexH;j += 4)
				{
					Str2[0] = Str1[strlen(Str1) - 1];
					Str2[1] = 0;
					Str1[strlen(Str1) - 1] = 0;

					Buffer_char = (char  *)malloc((NumberOfBuffer_char + 4) * sizeof(int));
					memcpy(Buffer_char, EndSimCondition_Values, NumberOfBuffer_char * sizeof(int));
					free(EndSimCondition_Values);
					EndSimCondition_Values = Buffer_char;

					v = strtol(Str2, NULL, 16);
					for (i = 0;i < 4;i++)
					{
						EndSimCondition_Values[NumberOfBuffer_char] = (v & (1 << i)) ? 1 : 0;
						NumberOfBuffer_char++;
					}
				}
			}
			else if (Str2[0] == 'b')
			{
				for (j = IndexL;j <= IndexH;j += 1)
				{
					Str2[0] = Str1[strlen(Str1) - 1];
					Str2[1] = 0;
					Str1[strlen(Str1) - 1] = 0;

					Buffer_char = (char  *)malloc((NumberOfBuffer_char + 1) * sizeof(int));
					memcpy(Buffer_char, EndSimCondition_Values, NumberOfBuffer_char * sizeof(int));
					free(EndSimCondition_Values);
					EndSimCondition_Values = Buffer_char;

					v = strtol(Str2, NULL, 2);
					EndSimCondition_Values[NumberOfBuffer_char] = v;
					NumberOfBuffer_char++;
				}
			}
			else
			{
				printf("simulation file: base not known in ""%s""\n", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			if (NumberOfBuffer_char != EndSimCondition_NumberOfSignals)
			{
				printf("simulation file: something is wrong in given initial inputs\n");
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			SimulationFileCheckList |= (1 << 4);
		}
		else if (!strcmp(Str1, "end_wait_cycles"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			EndSim_NumberOfWaitCycles = atoi(Str1);
			SimulationFileCheckList |= (1 << 5);
		}
		else if (!strcmp(Str1, "no_of_outputs"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");

			EndSim_NumberOfOutputBlocks = atoi(Str1);
			EndSim_OutputNames = (char **)malloc(EndSim_NumberOfOutputBlocks * sizeof(char *));
			EndSim_Outputs_IndexL = (int   *)malloc(EndSim_NumberOfOutputBlocks * sizeof(int));
			EndSim_Outputs_IndexH = (int   *)malloc(EndSim_NumberOfOutputBlocks * sizeof(int));
			EndSim_Outputs_Base = (char  *)malloc(EndSim_NumberOfOutputBlocks * sizeof(char));

			for (OutputIndex = 0;OutputIndex < EndSim_NumberOfOutputBlocks;OutputIndex++)
			{
				ReadNonCommentFromFile(SimulationFile, Str1, "%");

				if ((Str1[0] == '[') & (Str1[strlen(Str1) - 1] == ']'))
				{
					strcpy(Str2, Str1 + 1);
					Str2[strlen(Str2) - 1] = 0;
					str_ptr = strchr(Str2, ':');
					*str_ptr = 0;
					EndSim_Outputs_IndexH[OutputIndex] = atoi(Str2);
					EndSim_Outputs_IndexL[OutputIndex] = atoi(str_ptr + 1);

					ReadNonCommentFromFile(SimulationFile, Str1, "%");
				}
				else
				{
					EndSim_Outputs_IndexL[OutputIndex] = -1;
					EndSim_Outputs_IndexH[OutputIndex] = -1;
				}

				EndSim_OutputNames[OutputIndex] = (char *)malloc(strlen(Str1) + 2);
				strcpy(EndSim_OutputNames[OutputIndex], Str1);

				ReadNonCommentFromFile(SimulationFile, Str1, "%");

				strcpy(Str2, Str1);
				str_ptr = strchr(Str2, '\'');
				*str_ptr = 0;
				j = atoi(Str2);
				if (j != (EndSim_Outputs_IndexH[OutputIndex] - EndSim_Outputs_IndexL[OutputIndex] + 1))
				{
					printf("simulation file: length ""%s"" does not match to the given size\n", Str1);
					fclose(SimulationFile);
					free(Str1);
					free(Str2);
					return 1;
				}

				strcpy(Str2, str_ptr + 1);
				if (Str2[0] == 'h')
					EndSim_Outputs_Base[OutputIndex] = 16;
				else if (Str2[0] == 'b')
					EndSim_Outputs_Base[OutputIndex] = 2;
				else
				{
					printf("simulation file: base not known in ""%s""\n", Str1);
					fclose(SimulationFile);
					free(Str1);
					free(Str2);
					return 1;
				}
			}

			SimulationFileCheckList |= (1 << 6);
		}
		else if (!strcmp(Str1, "faul_injection_include"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");

			if (!strcmp(Str1, "all"))
			{
				for (CellIndex = 0;CellIndex < NumberOfCells;CellIndex++)
					Cells[CellIndex]->FaultAllowed = 1;
			}
			else
			{
				NumberOfItems = atoi(Str1);

				for (i = 0;i < NumberOfItems;i++)
				{
					ReadNonCommentFromFile(SimulationFile, Str1, "%");
					OneFound = 0;

					for (CellIndex = 0;CellIndex < NumberOfCells;CellIndex++)
						if (strstr(Hierarchy[Cells[CellIndex]->Hierarchy]->Name, Str1) == Hierarchy[Cells[CellIndex]->Hierarchy]->Name)
						{
							Cells[CellIndex]->FaultAllowed = 1;
							OneFound = 1;
						}

					if (!OneFound)
					{
						printf("Hierarchy ""%s"" given in simulation file for faul_injection_include not found", Str1);
						fclose(SimulationFile);
						free(Str1);
						free(Str2);
						return 1;
					}
				}
			}

			SimulationFileCheckList |= (1 << 7);
		}
		else if (!strcmp(Str1, "faul_injection_exclude"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");

			if (!strcmp(Str1, "all"))
			{
				for (CellIndex = 0;CellIndex < NumberOfCells;CellIndex++)
					Cells[CellIndex]->FaultAllowed = 0;
			}
			else
			{
				NumberOfItems = atoi(Str1);

				for (i = 0;i < NumberOfItems;i++)
				{
					ReadNonCommentFromFile(SimulationFile, Str1, "%");
					OneFound = 0;

					for (CellIndex = 0;CellIndex < NumberOfCells;CellIndex++)
						if (strstr(Hierarchy[Cells[CellIndex]->Hierarchy]->Name, Str1) == Hierarchy[Cells[CellIndex]->Hierarchy]->Name)
						{
							Cells[CellIndex]->FaultAllowed = 0;
							OneFound = 1;
						}

					if (!OneFound)
					{
						printf("Hierarchy ""%s"" given in simulation file for faul_injection_exclude not found", Str1);
						fclose(SimulationFile);
						free(Str1);
						free(Str2);
						return 1;
					}
				}
			}

			SimulationFileCheckList |= (1 << 8);
		}
		else if (!strcmp(Str1, "fault_injection_method"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");

			if (!strcmp(Str1, "exhaustive"))
				FaultInjectionMethod = FaultInjection_Exhaustive;
			else if (!strcmp(Str1, "random"))
				FaultInjectionMethod = FaultInjection_Random;
			else
			{
				printf("given fault_injection_method ""%s"" in simulation file not known", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			SimulationFileCheckList |= (1 << 9);
		}
		else if (!strcmp(Str1, "number_of_random_fault_injections"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			NumberOfSimulationsInFile = atoi(Str1);

			if ((FaultInjectionMethod == FaultInjection_Random) & (!NumberOfSimulationsInFile))
			{
				printf("number_of_random_fault_injections ""%s"" in simulation file not valid", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			SimulationFileCheckList |= (1 << 10);
		}
		else if (!strcmp(Str1, "number_of_target_clock_cycles"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			NumberOfTargetClockCycles = atoi(Str1);

			if (!NumberOfTargetClockCycles)
			{
				printf("number_of_target_clock_cycles ""%s"" in simulation file not valid", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			TargetClockCycles = (int*)malloc(NumberOfTargetClockCycles * sizeof(int));

			for (i = 0;i < NumberOfTargetClockCycles;i++)
			{
				ReadNonCommentFromFile(SimulationFile, Str1, "%");
				TargetClockCycles[i] = atoi(Str1);
			}

			SimulationFileCheckList |= (1 << 11);
			SimulationFileCheckList |= (1 << 12);
		}
		else if (!strcmp(Str1, "max_no_faults_per_run"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			MaxNumberOfFaultsPerRun = atoi(Str1);

			if (!MaxNumberOfFaultsPerRun)
			{
				printf("max_no_faults_per_run ""%s"" in simulation file not valid", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			SimulationFileCheckList |= (1 << 13);
		}
		else if (!strcmp(Str1, "min_no_faults_per_run"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			MinNumberOfFaultsPerRun = atoi(Str1);

			if ((!MinNumberOfFaultsPerRun) | (MinNumberOfFaultsPerRun > MaxNumberOfFaultsPerRun))
			{
				printf("min_no_faults_per_run ""%s"" in simulation file not valid", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			SimulationFileCheckList |= (1 << 14);
		}
		else if (!strcmp(Str1, "max_no_faults_per_cycle"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			MaxNumberOfFaultsPerCycle = atoi(Str1);

			if ((!MaxNumberOfFaultsPerCycle) | (MaxNumberOfFaultsPerCycle > MaxNumberOfFaultsPerRun))
			{
				printf("max_no_faults_per_cycle ""%s"" in simulation file not valid", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			SimulationFileCheckList |= (1 << 15);
		}
		else if (!strcmp(Str1, "min_no_faults_per_cycle"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			MinNumberOfFaultsPerCycle = atoi(Str1);

			if ((!MinNumberOfFaultsPerCycle) | (MinNumberOfFaultsPerCycle > MaxNumberOfFaultsPerCycle) | (MinNumberOfFaultsPerCycle > MinNumberOfFaultsPerRun))
			{
				printf("min_no_faults_per_cycle ""%s"" in simulation file not valid", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			SimulationFileCheckList |= (1 << 16);
		}
		else if (!strcmp(Str1, "print_nondetected"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			Print_Nondetected = atoi(Str1);
			SimulationFileCheckList |= (1 << 17);
		}
		else if (!strcmp(Str1, "print_detected"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			Print_Detected = atoi(Str1);
			SimulationFileCheckList |= (1 << 18);
		}
		else if (!strcmp(Str1, "print_ineffective"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			Print_Ineffective = atoi(Str1);
			SimulationFileCheckList |= (1 << 19);
		}
		else if (!strcmp(Str1, "print_runtimeerror"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			Print_Ineffective = atoi(Str1);
			SimulationFileCheckList |= (1 << 20);
		}
		else if (!strcmp(Str1, "max_no_of_threads"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");
			Max_no_of_Threads = atoi(Str1);

			if ((!Max_no_of_Threads))
			{
				printf("given max_no_of_threads ""%s"" in simulation file not valid", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			SimulationFileCheckList |= (1 << 21);
		}
		else if (!strcmp(Str1, "fault_injection_type"))
		{
			ReadNonCommentFromFile(SimulationFile, Str1, "%");

			if (!strcmp(Str1, "stuck_at_0"))
				FaultInjectionType = FaultInjection_stuck_at_0;
			else if (!strcmp(Str1, "stuck_at_1"))
				FaultInjectionType = FaultInjection_stuck_at_1;
			else if (!strcmp(Str1, "toggle"))
				FaultInjectionType = FaultInjection_toggle;
			else
			{
				printf("given fault_injection_type ""%s"" in simulation file not known", Str1);
				fclose(SimulationFile);
				free(Str1);
				free(Str2);
				return 1;
			}

			SimulationFileCheckList |= (1 << 22);
		}
	} while ((SimulationFileCheckList != ((1 << 23) - 1)) & (!feof(SimulationFile)));


	fclose(SimulationFile);
	free(Str1);
	free(Str2);

	if (SimulationFileCheckList != ((1 << 23)-1))
	{
		printf("simulation file: all required information does not exist\n");
		return 1;
	}

	if (FaultInjectionMethod == FaultInjection_Exhaustive)
	{
		if (MinNumberOfFaultsPerRun < MinNumberOfFaultsPerCycle*NumberOfTargetClockCycles)
		{
			printf("given min_no_faults_per_run ""%d"" in simulation file is lower than expected ""%d"".", MinNumberOfFaultsPerRun, MinNumberOfFaultsPerCycle*NumberOfTargetClockCycles);
			return 1;
		}
	}

	if (FaultInjectionMethod == FaultInjection_Random)
	{
		if (MaxNumberOfFaultsPerRun > MaxNumberOfFaultsPerCycle*NumberOfTargetClockCycles)
		{
			printf("given max_no_faults_per_run ""%d"" in simulation file is higher than expected ""%d"".", MaxNumberOfFaultsPerRun, MaxNumberOfFaultsPerCycle*NumberOfTargetClockCycles);
			return 1;
		}
	}
	
	return 0;
}

//***************************************************************************************
