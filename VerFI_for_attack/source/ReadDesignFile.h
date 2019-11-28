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

int ReadDesignFile(SignalStruct** &Signals, int &NumberOfSignals, int* &Inputs, int &NumberOfInputs,
	int* &Outputs, int &NumberOfOutputs, CellStruct** &Cells, int &NumberOfCells,
	int* &Gates, int &NumberOfGates, int* &Regs, int &NumberOfRegs, int &NumberOfRegValues,
	char* InputVerilogFileName, char* MainModuleName,
	CellTypeStruct** CellTypes, int NumberOfCellTypes,
	HierarchyStruct** Hierarchy, int NumberOfHierarchy)
{
	FILE *DesignFile;
	char finished;
	char ReadSignalsFinished;
	int  CellTypeIndex;
	int  CaseIndex;
	char *Str1 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str2 = (char *)malloc(Max_Name_Length * sizeof(char));
	char ch;
	int  i, j;
	int  MyNumberofIO;
	int  CurrentIO;
	int  InputIndex;
	int  OutputIndex;
	int  SignalIndex;
	int* Buffer_int;
	int  IndexH, IndexL;
	int  HierarchyIndex;
	SignalStruct** TempSignals;
	int*           TempInputs;
	int*           TempOutputs;
	CellStruct**   TempCells;
	int*           TempGates;
	int*           TempRegs;

	NumberOfSignals = 0;
	NumberOfOutputs = 0;
	NumberOfInputs = 0;

	char *Phrase = (char *)malloc(Max_Name_Length * sizeof(char));
	char Task;
	char IO_port_found;

	NumberOfCells = 0;
	NumberOfGates = 0;
	NumberOfRegs = 0;
	NumberOfRegValues = 0;

	// --------- adding 0 and 1 signals --------------

	NumberOfSignals = 2;
	Signals = (SignalStruct **)malloc(NumberOfSignals * sizeof(SignalStruct *));

	Signals[0] = (SignalStruct *)malloc(sizeof(SignalStruct));
	Signals[0]->Name = (char *)malloc(strlen("1'b0") + 2);
	strcpy(Signals[0]->Name, "1'b0");
	Signals[0]->Type = SignalType_wire;
	Signals[0]->NumberOfInputs = 0;
	Signals[0]->Inputs = NULL;
	Signals[0]->Depth = 0;
	Signals[0]->Output = -1;

	Signals[1] = (SignalStruct *)malloc(sizeof(SignalStruct));
	Signals[1]->Name = (char *)malloc(strlen("1'b1") + 2);
	strcpy(Signals[1]->Name, "1'b1");
	Signals[1]->Type = SignalType_wire;
	Signals[1]->NumberOfInputs = 0;
	Signals[1]->Inputs = NULL;
	Signals[1]->Depth = 0;
	Signals[1]->Output = -1;

	//---------------------------------------------------------------------------------------------//
	//------------------- reading the signals from the design file --------------------------------//

	DesignFile = fopen(InputVerilogFileName, "rt");

	if (DesignFile == NULL)
	{
		printf("design file ""%s"" not found\n", InputVerilogFileName);
		free(Str1);
		free(Str2);
		free(Phrase);
		return 1;
	}

	finished = 0;
	ReadSignalsFinished = 0;

	do {
		do {
			ReadNonCommentFromFile(DesignFile, Str1, "//");
		} while ((!feof(DesignFile)) & strcmp(Str1, "module"));

		if (!feof(DesignFile))
		{
			ReadNonCommentFromFile(DesignFile, Str1, "//");
			if (strcmp(Str1, MainModuleName))
			{
				do {
					ReadNonCommentFromFile(DesignFile, Str1, "//");
				} while ((!feof(DesignFile)) & strcmp(Str1, "endmodule"));
			}
			else
			{
				do {
					ch = fgetc(DesignFile);
				} while ((ch != ';') & (!feof(DesignFile)));  // ignoring the entire list of input/output ports

				while ((!ReadSignalsFinished) & (!feof(DesignFile)))
				{
					ReadNonCommentFromFile(DesignFile, Str1, "//");

					if ((!strcmp(Str1, "input")) | (!strcmp(Str1, "output")) | (!strcmp(Str1, "wire")))
					{
						strcpy(Phrase, Str1);

						i = 0;
						IndexH = -1;
						IndexL = -1;

						do {
							ch = fgetc(DesignFile);

							if (ch == '[')
							{
								// do nothing
							}
							else if (ch == ':')
							{
								IndexH = atoi(Str1);
								i = 0;
							}
							else if (ch == ']')
							{
								IndexL = atoi(Str1);
								i = 0;
							}
							else if ((ch == ',') | (ch == ';'))
							{
								for (j = IndexL;j <= IndexH;j++)
								{
									if (IndexL != -1)
										sprintf(Str2, "%s[%d]", Str1, j);
									else
										sprintf(Str2, "%s", Str1);

									TempSignals = (SignalStruct **)malloc((NumberOfSignals + 1) * sizeof(SignalStruct *));
									memcpy(TempSignals, Signals, NumberOfSignals * sizeof(SignalStruct *));
									free(Signals);
									Signals = TempSignals;

									Signals[NumberOfSignals] = (SignalStruct *)malloc(sizeof(SignalStruct));
									Signals[NumberOfSignals]->Name = (char *)malloc(strlen(Str2) + 2);
									strcpy(Signals[NumberOfSignals]->Name, Str2);
									Signals[NumberOfSignals]->NumberOfInputs = 0;
									Signals[NumberOfSignals]->Inputs = NULL;
									Signals[NumberOfSignals]->Output = -1;

									if (!strcmp(Phrase, "input"))
									{
										Signals[NumberOfSignals]->Type = SignalType_input;
										Signals[NumberOfSignals]->Depth = 0;

										TempInputs = (int *)malloc((NumberOfInputs + 1) * sizeof(int));
										memcpy(TempInputs, Inputs, NumberOfInputs * sizeof(int));
										free(Inputs);
										Inputs = TempInputs;

										Inputs[NumberOfInputs] = NumberOfSignals;
										NumberOfInputs++;
									}
									else if (!strcmp(Phrase, "output"))
									{
										Signals[NumberOfSignals]->Type = SignalType_output;
										Signals[NumberOfSignals]->Depth = -1;

										TempOutputs = (int *)malloc((NumberOfOutputs + 1) * sizeof(int));
										memcpy(TempOutputs, Outputs, NumberOfOutputs * sizeof(int));
										free(Outputs);
										Outputs = TempOutputs;

										Outputs[NumberOfOutputs] = NumberOfSignals;
										NumberOfOutputs++;
									}
									else  // if (!strcmp(Phrase, "wire"))
									{
										Signals[NumberOfSignals]->Type = SignalType_wire;
										Signals[NumberOfSignals]->Depth = -1;
									}

									NumberOfSignals++;
								}

								i = 0;
							}
							else if ((ch != ' ') & (ch != '\n'))
							{
								Str1[i++] = ch;
								Str1[i] = 0;
							}
						} while ((ch != ';') & (!feof(DesignFile)));
					}
					else
						ReadSignalsFinished = 1;
				}

				//---------------------------------------------------------------------------------------------//
				//------------------- reading the cells from the design file ----------------------------------//

				if (!feof(DesignFile))
				{
					strcpy(Str2, Str1);

					do {
						ReadNonCommentFromFile(DesignFile, Str1, "//");
						strcat(Str2, " ");
						strcat(Str2, Str1);

						if (Str1[strlen(Str1) - 1] == ';')
						{
							i = 0;
							j = 0;
							Str1[0] = 0;
							Task = Task_find_module_type;

							do {
								ch = Str2[i++];

								if (ch == ' ')
								{
									if (j)
									{
										if (Task == Task_find_module_type)
										{
											for (CellTypeIndex = 0;CellTypeIndex < NumberOfCellTypes;CellTypeIndex++)
											{
												for (CaseIndex = 0;CaseIndex < CellTypes[CellTypeIndex]->NumberOfCases;CaseIndex++)
													if (!strcmp(Str1, CellTypes[CellTypeIndex]->Cases[CaseIndex]))
													{
														CaseIndex = -1;
														break;
													}

												if (CaseIndex == -1)
													break;
											}

											if (CellTypeIndex < NumberOfCellTypes)
											{
												TempCells = (CellStruct **)malloc((NumberOfCells + 1) * sizeof(CellStruct *));
												memcpy(TempCells, Cells, NumberOfCells * sizeof(CellStruct *));
												free(Cells);
												Cells = TempCells;

												Cells[NumberOfCells] = (CellStruct *)malloc(sizeof(CellStruct));
												Cells[NumberOfCells]->Type = CellTypeIndex;
												Cells[NumberOfCells]->NumberOfInputs = CellTypes[CellTypeIndex]->NumberOfInputs;
												Cells[NumberOfCells]->Inputs = (int *)malloc(CellTypes[CellTypeIndex]->NumberOfInputs * sizeof(int));
												Cells[NumberOfCells]->NumberOfOutputs = CellTypes[CellTypeIndex]->NumberOfOutputs;
												Cells[NumberOfCells]->Outputs = (int *)malloc(CellTypes[CellTypeIndex]->NumberOfOutputs * sizeof(int));

												if (CellTypes[CellTypeIndex]->GateOrReg == CellType_Gate)
												{
													Cells[NumberOfCells]->Depth = -1;

													TempGates = (int *)malloc((NumberOfGates + 1) * sizeof(int));
													memcpy(TempGates, Gates, NumberOfGates * sizeof(int));
													free(Gates);
													Gates = TempGates;

													Gates[NumberOfGates] = NumberOfCells;
													NumberOfGates++;
												}
												else // CellType_Reg
												{
													Cells[NumberOfCells]->Depth = 0;
													Cells[NumberOfCells]->RegValueIndexes = (int *)malloc(CellTypes[CellTypeIndex]->NumberOfOutputs * sizeof(int));
													for (OutputIndex = 0;OutputIndex < CellTypes[CellTypeIndex]->NumberOfOutputs;OutputIndex++)
														Cells[NumberOfCells]->RegValueIndexes[OutputIndex] = NumberOfRegValues++;

													TempRegs = (int *)malloc((NumberOfRegs + 1) * sizeof(int));
													memcpy(TempRegs, Regs, NumberOfRegs * sizeof(int));
													free(Regs);
													Regs = TempRegs;

													Regs[NumberOfRegs] = NumberOfCells;
													NumberOfRegs++;
												}

												Task = Task_find_module_name;
												MyNumberofIO = CellTypes[CellTypeIndex]->NumberOfInputs + CellTypes[CellTypeIndex]->NumberOfOutputs;
												CurrentIO = 0;
											}
											else
											{
												printf("cell type ""%s"" not found\n", Str1);
												fclose(DesignFile);
												free(Str1);
												free(Str2);
												free(Phrase);
												return 1;
											}
										}
										else if (Task == Task_find_module_name)
										{
											Cells[NumberOfCells]->Name = (char *)malloc(strlen(Str1) + 2);
											strcpy(Cells[NumberOfCells]->Name, Str1);

											for (HierarchyIndex = 0;HierarchyIndex < NumberOfHierarchy;HierarchyIndex++)
											{
												if (!strcmp(Hierarchy[HierarchyIndex]->CellName, Str1))
													break;
											}

											if (HierarchyIndex < NumberOfHierarchy)
											{
												if (Cells[NumberOfCells]->Type != Hierarchy[HierarchyIndex]->Type)
												{
													printf("type cell ""%s"" does not match to type of hierarchy ""%s""\n", Str1, Hierarchy[HierarchyIndex]->Name);
													fclose(DesignFile);
													free(Str1);
													free(Str2);
													free(Phrase);
													return 1;
												}

												Cells[NumberOfCells]->Hierarchy = HierarchyIndex;
											}
											else
											{
												printf("hierarchy ""%s"" not found\n", Str1);
												fclose(DesignFile);
												free(Str1);
												free(Str2);
												free(Phrase);
												return 1;
											}

											Task = Task_find_open_bracket;
											IO_port_found = 0;
										}
										else if (Task == Task_find_IO_port)
										{
											if (Str1[0] == '.')
											{
												for (InputIndex = 0;InputIndex < CellTypes[CellTypeIndex]->NumberOfInputs;InputIndex++)
												{
													if (!strcmp(Str1 + 1, CellTypes[CellTypeIndex]->Inputs[InputIndex]))
														break;
												}

												if (InputIndex >= CellTypes[CellTypeIndex]->NumberOfInputs) // the IO port NOT found in the inputs
												{
													for (OutputIndex = 0;OutputIndex < CellTypes[CellTypeIndex]->NumberOfOutputs;OutputIndex++)
													{
														if (!strcmp(Str1 + 1, CellTypes[CellTypeIndex]->Outputs[OutputIndex]))
															break;
													}

													if (OutputIndex >= CellTypes[CellTypeIndex]->NumberOfOutputs) // the IO port NOT found in the outputs
													{
														printf("IO port ""%s"" not found in cell type ""%s""\n", Str1 + 1, CellTypes[CellTypeIndex]->Cases[CaseIndex]);
														fclose(DesignFile);
														free(Str1);
														free(Str2);
														free(Phrase);
														return 1;
													}

													InputIndex = -1;
												}

												IO_port_found = 1;
												Task = Task_find_open_bracket;
											}
											else
											{
												printf("error in reading the netlist, '.' is expected in %s\n", Str1);
												fclose(DesignFile);
												free(Str1);
												free(Str2);
												free(Phrase);
												return 1;
											}
										}
										else if (Task == Task_find_signal_name)
										{
											if (strlen(Str1))
											{
												for (SignalIndex = 0;SignalIndex < NumberOfSignals;SignalIndex++)
												{
													if (!strcmp(Str1, Signals[SignalIndex]->Name))
														break;
												}
											}
											else
												SignalIndex = -1;

											if (SignalIndex < NumberOfSignals)
											{
												if (InputIndex != -1)
												{
													Cells[NumberOfCells]->Inputs[InputIndex] = SignalIndex;

													Buffer_int = (int *)malloc((Signals[SignalIndex]->NumberOfInputs + 1) * sizeof(int));
													memcpy(Buffer_int, Signals[SignalIndex]->Inputs, Signals[SignalIndex]->NumberOfInputs * sizeof(int));
													free(Signals[SignalIndex]->Inputs);
													Signals[SignalIndex]->Inputs = Buffer_int;

													Signals[SignalIndex]->Inputs[Signals[SignalIndex]->NumberOfInputs] = NumberOfCells;
													Signals[SignalIndex]->NumberOfInputs++;
												}
												else
												{
													Cells[NumberOfCells]->Outputs[OutputIndex] = SignalIndex;

													if (SignalIndex != -1)
													{
														Signals[SignalIndex]->Output = NumberOfCells;
														if (CellTypes[CellTypeIndex]->GateOrReg == CellType_Reg)
															Signals[SignalIndex]->Depth = 0;
													}
												}
											}
											else
											{
												printf("signal ""%s"" not found\n", Str1);
												fclose(DesignFile);
												free(Str1);
												free(Str2);
												free(Phrase);
												return 1;
											}

											Task = Task_find_close_bracket;
										}

										j = 0;
										Str1[0] = 0;
									}
								}
								else if (ch == '(')
								{
									if (Task == Task_find_open_bracket)
									{
										if (IO_port_found)
											Task = Task_find_signal_name;
										else
											Task = Task_find_IO_port;
									}
									else if (Task == Task_find_IO_port)
									{
										if (Str1[0] == '.')
										{
											for (InputIndex = 0;InputIndex < CellTypes[CellTypeIndex]->NumberOfInputs;InputIndex++)
											{
												if (!strcmp(Str1 + 1, CellTypes[CellTypeIndex]->Inputs[InputIndex]))
													break;
											}

											if (InputIndex >= CellTypes[CellTypeIndex]->NumberOfInputs) // the IO port NOT found in the inputs
											{
												for (OutputIndex = 0;OutputIndex < CellTypes[CellTypeIndex]->NumberOfOutputs;OutputIndex++)
												{
													if (!strcmp(Str1 + 1, CellTypes[CellTypeIndex]->Outputs[OutputIndex]))
														break;
												}

												if (OutputIndex >= CellTypes[CellTypeIndex]->NumberOfOutputs) // the IO port NOT found in the outputs
												{
													printf("IO port ""%s"" did not found in cell type ""%s""\n", Str1 + 1, CellTypes[CellTypeIndex]->Cases[0]);
													fclose(DesignFile);
													free(Str1);
													free(Str2);
													free(Phrase);
													return 1;
												}

												InputIndex = -1;
											}

											Task = Task_find_signal_name;
										}
										else
										{
											printf("error in reading the netlist, '.' is expected in %s\n", Str1);
											fclose(DesignFile);
											free(Str1);
											free(Str2);
											free(Phrase);
											return 1;
										}
									}
									else
									{
										printf("Error!\n");
										fclose(DesignFile);
										free(Str1);
										free(Str2);
										free(Phrase);
										return 1;
									}

									j = 0;
									Str1[0] = 0;
								}
								else if (ch == ')')
								{
									if (Task == Task_find_close_bracket)
									{
										if (CurrentIO < MyNumberofIO)
											Task = Task_find_comma;
									}
									else if (Task == Task_find_signal_name)
									{
										if (strlen(Str1))
										{
											for (SignalIndex = 0;SignalIndex < NumberOfSignals;SignalIndex++)
											{
												if (!strcmp(Str1, Signals[SignalIndex]->Name))
													break;
											}
										}
										else
											SignalIndex = -1;

										if (SignalIndex < NumberOfSignals)
										{
											if (InputIndex != -1)
											{
												Cells[NumberOfCells]->Inputs[InputIndex] = SignalIndex;

												Buffer_int = (int *)malloc((Signals[SignalIndex]->NumberOfInputs + 1) * sizeof(int));
												memcpy(Buffer_int, Signals[SignalIndex]->Inputs, Signals[SignalIndex]->NumberOfInputs * sizeof(int));
												free(Signals[SignalIndex]->Inputs);
												Signals[SignalIndex]->Inputs = Buffer_int;

												Signals[SignalIndex]->Inputs[Signals[SignalIndex]->NumberOfInputs] = NumberOfCells;
												Signals[SignalIndex]->NumberOfInputs++;
											}
											else
											{
												Cells[NumberOfCells]->Outputs[OutputIndex] = SignalIndex;

												if (SignalIndex != -1)
												{
													Signals[SignalIndex]->Output = NumberOfCells;
													if (CellTypes[CellTypeIndex]->GateOrReg == CellType_Reg)
														Signals[SignalIndex]->Depth = 0;
												}
											}
										}
										else
										{
											printf("signal ""%s"" not found\n", Str1);
											fclose(DesignFile);
											free(Str1);
											free(Str2);
											free(Phrase);
											return 1;
										}

										CurrentIO++;
										if (CurrentIO < MyNumberofIO)
											Task = Task_find_comma;
										else
											Task = Task_find_close_bracket;
									}
									else
									{
										printf("Error!\n");
										fclose(DesignFile);
										free(Str1);
										free(Str2);
										free(Phrase);
										return 1;
									}

									j = 0;
									Str1[0] = 0;
								}
								else if (ch == ',')
								{
									IO_port_found = 0;
									Task = Task_find_IO_port;

									j = 0;
									Str1[0] = 0;
								}
								else
								{
									Str1[j++] = ch;
									Str1[j] = 0;
								}

							} while (ch != ';');

							NumberOfCells++;

							Str1[0] = 0;
							Str2[0] = 0;
						}

					} while ((strcmp(Str1, "endmodule")) & (!feof(DesignFile)));

					finished = 1;
				}
			}
		}
	} while ((!feof(DesignFile)) & (!finished));

	fclose(DesignFile);
	free(Str1);
	free(Str2);
	free(Phrase);

	if (!finished)
	{
		printf("The target module %s not found\n", MainModuleName);
		return 1;
	}

	return 0;
}

//***************************************************************************************
