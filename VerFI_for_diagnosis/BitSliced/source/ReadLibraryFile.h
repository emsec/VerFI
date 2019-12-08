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

int MakeFormulaForCellInLibrary(CellTypeStruct *CellType)
{
	char *Str  = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str2 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *Str3 = (char *)malloc(Max_Name_Length * sizeof(char));
	char *pos;
	char *Start=NULL;
	char *End=NULL;
	char OperationIndex;
	char *TempChar;
	char **TempCharChar;
	char Intermediate;
	int  i, l;
	   	 
	Intermediate = CellType->NumberOfInputs + CellType->NumberOfOutputs;
	CellType->Intermediate = Intermediate;
	CellType->Operations = (OperationStruct*)malloc(CellType->NumberOfOutputs * sizeof(OperationStruct));

	for (i = 0;i < CellType->NumberOfOutputs;i++)
	{
		CellType->Operations[i].NumberOfClauses = 0;
		CellType->Operations[i].OperationOfClause = NULL;
		CellType->Operations[i].NumberOfOperandsInClause = NULL;
		CellType->Operations[i].OperandsInClause = NULL;

		strcpy(Str, CellType->Expresions[i]);

		do {
			TempChar = (char*)malloc((CellType->Operations[i].NumberOfClauses + 1) * sizeof(char));
			memcpy(TempChar, CellType->Operations[i].OperationOfClause, CellType->Operations[i].NumberOfClauses * sizeof(char));
			free(CellType->Operations[i].OperationOfClause);
			CellType->Operations[i].OperationOfClause = TempChar;

			TempChar = (char*)calloc(CellType->Operations[i].NumberOfClauses + 1, sizeof(char));
			memcpy(TempChar, CellType->Operations[i].NumberOfOperandsInClause, CellType->Operations[i].NumberOfClauses * sizeof(char));
			free(CellType->Operations[i].NumberOfOperandsInClause);
			CellType->Operations[i].NumberOfOperandsInClause = TempChar;
			
			TempCharChar = (char**)malloc((CellType->Operations[i].NumberOfClauses + 1) * sizeof(char*));
			memcpy(TempCharChar, CellType->Operations[i].OperandsInClause, CellType->Operations[i].NumberOfClauses * sizeof(char*));
			free(CellType->Operations[i].OperandsInClause);
			CellType->Operations[i].OperandsInClause = TempCharChar;
			CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses] = NULL;

			pos = Str;
			while ((*pos != ')') & (*pos != 0))
			{
				if (*pos == '(')
					Start = pos;
				pos++;
			}
			End = pos;

			if (*pos != 0)
			{
				memcpy(Str2, Start + 1, End - Start - 1);
				Str2[End - Start - 1] = 0;
			}
			else
				strcpy(Str2, Str);

			if (strstr(Str2, " and "))
				OperationIndex = Operation_AND;
			else if (strstr(Str2, " or "))
				OperationIndex = Operation_OR;
			else if (strstr(Str2, " xor "))
				OperationIndex = Operation_XOR;
			else if (strstr(Str2, "not "))
				OperationIndex = Operation_NOT;
			else
			{
				printf("Error in Library file %s\n", Str);
				free(Str);
				free(Str2);
				free(Str3);
				return 1;
			}

			CellType->Operations[i].OperationOfClause[CellType->Operations[i].NumberOfClauses] = OperationIndex;

			for (l = 0;l < CellType->NumberOfInputs;l++)
				if (strstr(Str2, CellType->Inputs[l]))
				{
					TempChar = (char*)malloc((CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses] + 1) * sizeof(char));
					memcpy(TempChar, CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses], CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses] * sizeof(char));
					free(CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses]);
					CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses] = TempChar;

					CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses][CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses]] = l;
					CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses]++;
				}

			for (l = 0;l < CellType->NumberOfOutputs;l++)
				if (strstr(Str2, CellType->Outputs[l]))
				{
					TempChar = (char*)malloc((CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses] + 1) * sizeof(char));
					memcpy(TempChar, CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses], CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses] * sizeof(char));
					free(CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses]);
					CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses] = TempChar;

					CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses][CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses]] = CellType->NumberOfInputs + l;
					CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses]++;
				}

			for (l = 0;l < CellType->Operations[i].NumberOfClauses;l++)
			{
				sprintf(Str3, "I%d", Intermediate + l + 1);
				if (strstr(Str2, Str3))
				{
					TempChar = (char*)malloc((CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses] + 1) * sizeof(char));
					memcpy(TempChar, CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses], CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses] * sizeof(char));
					free(CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses]);
					CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses] = TempChar;

					CellType->Operations[i].OperandsInClause[CellType->Operations[i].NumberOfClauses][CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses]] = Intermediate + l;
					CellType->Operations[i].NumberOfOperandsInClause[CellType->Operations[i].NumberOfClauses]++;
				}
			}

			if (*pos != 0)
			{
				sprintf(Start, "I%d", Intermediate + CellType->Operations[i].NumberOfClauses + 1);
				strcat(Str, End + 1);
			}

			CellType->Operations[i].NumberOfClauses++;
		} while (*pos != 0);
	}

	free(Str);
	free(Str2);
	free(Str3);
	return 0;
}

//***************************************************************************************

int ReadLibrryFile(CellTypeStruct** &CellTypes, int &NumberOfCellTypes, char* LibraryFileName, char* LibraryName)
{
	FILE *LibraryFile;
	char *Str1 = (char *)malloc(Max_Name_Length * sizeof(char));
	char ch;
	int  i, j;
	char LibraryRead = 0;
	CellTypeStruct** TempCellTypes;

	NumberOfCellTypes = 0;
	LibraryFile = fopen(LibraryFileName, "rt");

	if (LibraryFile == NULL)
	{
		printf("library file ""%s"" not found\n", LibraryFileName);
		free(Str1);
		return 1;
	}

	while ((!LibraryRead) & (!feof(LibraryFile)))
	{
		Str1[0] = 0;
		while (strcmp(Str1, "Library") & (!feof(LibraryFile)))
			ReadNonCommentFromFile(LibraryFile, Str1, "%");

		if (!feof(LibraryFile))
		{
			ReadNonCommentFromFile(LibraryFile, Str1, "%");
			if (!strcmp(Str1, LibraryName))
			{
				ReadNonCommentFromFile(LibraryFile, Str1, "%");

				while (strcmp(Str1, "Library") & (!feof(LibraryFile)))
				{
					TempCellTypes = (CellTypeStruct **)malloc((NumberOfCellTypes + 1) * sizeof(CellTypeStruct *));
					memcpy(TempCellTypes, CellTypes, NumberOfCellTypes * sizeof(CellTypeStruct *));
					free(CellTypes);
					CellTypes = TempCellTypes;

					CellTypes[NumberOfCellTypes] = (CellTypeStruct *)malloc(sizeof(CellTypeStruct));
					if (!strcmp(Str1, "Gate"))
						CellTypes[NumberOfCellTypes]->GateOrReg = CellType_Gate;
					else if (!strcmp(Str1, "Reg"))
						CellTypes[NumberOfCellTypes]->GateOrReg = CellType_Reg;
					else
					{
						printf("CellType ""%s"" in library not known\n", Str1);
						fclose(LibraryFile);
						free(Str1);
						return 1;
					}

					ReadNonCommentFromFile(LibraryFile, Str1, "%");
					CellTypes[NumberOfCellTypes]->NumberOfCases = atoi(Str1);
					CellTypes[NumberOfCellTypes]->Cases = (char **)malloc(CellTypes[NumberOfCellTypes]->NumberOfCases * sizeof(char *));

					for (i = 0;i < CellTypes[NumberOfCellTypes]->NumberOfCases;i++)
					{
						ReadNonCommentFromFile(LibraryFile, Str1, "%");
						CellTypes[NumberOfCellTypes]->Cases[i] = (char *)malloc(strlen(Str1) + 2);
						strcpy(CellTypes[NumberOfCellTypes]->Cases[i], Str1);
					}

					ReadNonCommentFromFile(LibraryFile, Str1, "%");
					CellTypes[NumberOfCellTypes]->NumberOfInputs = atoi(Str1);
					CellTypes[NumberOfCellTypes]->Inputs = (char **)malloc(CellTypes[NumberOfCellTypes]->NumberOfInputs * sizeof(char *));

					for (i = 0;i < CellTypes[NumberOfCellTypes]->NumberOfInputs;i++)
					{
						ReadNonCommentFromFile(LibraryFile, Str1, "%");
						CellTypes[NumberOfCellTypes]->Inputs[i] = (char *)malloc(strlen(Str1) + 2);
						strcpy(CellTypes[NumberOfCellTypes]->Inputs[i], Str1);
					}

					ReadNonCommentFromFile(LibraryFile, Str1, "%");
					CellTypes[NumberOfCellTypes]->NumberOfOutputs = atoi(Str1);
					CellTypes[NumberOfCellTypes]->Outputs = (char **)malloc(CellTypes[NumberOfCellTypes]->NumberOfOutputs * sizeof(char *));

					for (i = 0;i < CellTypes[NumberOfCellTypes]->NumberOfOutputs;i++)
					{
						ReadNonCommentFromFile(LibraryFile, Str1, "%");
						CellTypes[NumberOfCellTypes]->Outputs[i] = (char *)malloc(strlen(Str1) + 2);
						strcpy(CellTypes[NumberOfCellTypes]->Outputs[i], Str1);
					}

					CellTypes[NumberOfCellTypes]->Expresions = (char **)malloc(CellTypes[NumberOfCellTypes]->NumberOfOutputs * sizeof(char *));

					for (i = 0;i < CellTypes[NumberOfCellTypes]->NumberOfOutputs;i++)
					{
						Str1[0] = 0;
						j = 0;
						do {
							ch = fgetc(LibraryFile);
							if (ch != '\n')
								Str1[j++] = ch;
						} while ((ch != '\n') | (!j));

						Str1[j] = 0;
						CellTypes[NumberOfCellTypes]->Expresions[i] = (char *)malloc(strlen(Str1) + 2);
						strcpy(CellTypes[NumberOfCellTypes]->Expresions[i], Str1);
					}

					if (MakeFormulaForCellInLibrary(CellTypes[NumberOfCellTypes]))
						return 1;

					NumberOfCellTypes++;
					ReadNonCommentFromFile(LibraryFile, Str1, "%");
				}

				LibraryRead = 1;
			}
		}
	}

	fclose(LibraryFile);
	free(Str1);

	if (!LibraryRead)
	{
		printf("Library could not be found\n");
		return 1;
	}

	return 0;
}

//***************************************************************************************
