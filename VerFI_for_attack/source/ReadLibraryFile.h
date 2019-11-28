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

void MakeTableForCellInLibrary(CellTypeStruct *CellType)
{
	typedef exprtk::expression<double>     expression_t;
	typedef exprtk::parser<double>             parser_t;

	std::string  expression_str;
	expression_t expression;
	parser_t     parser;

	int  i, j, k, l;
	char *Str = (char *)malloc(Max_Name_Length * sizeof(char));
	char *pos;
	int  add;

	if (CellType->GateOrReg == CellType_Gate)
		add = 0;
	else
		add = CellType->NumberOfOutputs;

	CellType->Tables = (char **)malloc(CellType->NumberOfOutputs * sizeof(char *));
	j = 1;
	j <<= (CellType->NumberOfInputs + add);

	for (i = 0;i < CellType->NumberOfOutputs;i++)
	{
		CellType->Tables[i] = (char *)malloc(j);

		for (k = 0;k < j;k++) // all possible input values
		{
			strcpy(Str, CellType->Expresions[i]);

			for (l = 0;l < CellType->NumberOfInputs;l++)
			{
				while (pos = strstr(Str, CellType->Inputs[l]))
				{
					sprintf(pos, "%d%s", (k & (1 << l)) ? 1 : 0, pos + strlen(CellType->Inputs[l]));
				}
			}

			if (add)
			{
				for (l = 0;l < CellType->NumberOfOutputs;l++)
				{
					while (pos = strstr(Str, CellType->Outputs[l]))
					{
						sprintf(pos, "%d%s", (k & (1 << (l + CellType->NumberOfInputs))) ? 1 : 0, pos + strlen(CellType->Outputs[l]));
					}
				}
			}

			expression_str.assign(Str);
			parser.compile(expression_str, expression);
			CellType->Tables[i][k] = char(expression.value());
		}
	}

	free(Str);
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

					MakeTableForCellInLibrary(CellTypes[NumberOfCellTypes]);

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
