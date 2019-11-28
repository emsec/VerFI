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

int ReadCellReportFile(HierarchyStruct** &Hierarchy, int &NumberOfHierarchy, char* InputCellReportFileName,
	CellTypeStruct** CellTypes, int NumberOfCellTypes)
{
	FILE *CellReportFile;
	char *StrList[5];
	int  CellTypeIndex;
	int  CaseIndex;
	char *Str1 = (char *)malloc(Max_Name_Length * sizeof(char));
	char ch;
	int  i;
	HierarchyStruct** TempHierarchy;

	for (i = 0;i < 5;i++)
		StrList[i] = (char *)malloc(Max_Name_Length * sizeof(char));

	NumberOfHierarchy = 0;
	CellReportFile = fopen(InputCellReportFileName, "rt");

	if (CellReportFile == NULL)
	{
		printf("cell report file ""%s"" not found\n", InputCellReportFileName);
		free(Str1);
		return 1;
	}

	StrList[0][0] = 0;
	StrList[1][0] = 0;
	StrList[2][0] = 0;
	StrList[3][0] = 0;
	StrList[4][0] = 0;

	do {
		for (i = 4; i > 0;i--)
			strcpy(StrList[i], StrList[i - 1]);

		ReadNonCommentFromFile(CellReportFile, StrList[0], "**");
	} while ((strcmp(StrList[4], "Cell") |
		strcmp(StrList[3], "Reference") |
		strcmp(StrList[2], "Library") |
		strcmp(StrList[1], "Area") |
		strcmp(StrList[0], "Attributes")) & (!feof(CellReportFile)));

	if (feof(CellReportFile))
	{
		printf("cell report file seems not correct\n");
		fclose(CellReportFile);
		free(Str1);
		for (i = 0;i < 5;i++)
			free(StrList[i]);
		return 1;
	}

	ReadNonCommentFromFile(CellReportFile, StrList[0], "**");

	if (strstr(StrList[0], "--------------------") == NULL)
	{
		printf("cell report file seems not correct\n");
		fclose(CellReportFile);
		free(Str1);
		for (i = 0;i < 5;i++)
			free(StrList[i]);
		return 1;
	}

	do
	{
		for (i = 0;i < 4;i++)
			ReadNonCommentFromFile(CellReportFile, StrList[i], "**");

		if (strstr(StrList[0], "--------------------") == NULL)
		{
			TempHierarchy = (HierarchyStruct **)malloc((NumberOfHierarchy + 1) * sizeof(HierarchyStruct *));
			memcpy(TempHierarchy, Hierarchy, NumberOfHierarchy * sizeof(HierarchyStruct *));
			free(Hierarchy);
			Hierarchy = TempHierarchy;

			Hierarchy[NumberOfHierarchy] = (HierarchyStruct *)malloc(sizeof(HierarchyStruct));
			Hierarchy[NumberOfHierarchy]->Name = (char *)malloc(strlen(StrList[0]) + 2);
			strcpy(Hierarchy[NumberOfHierarchy]->Name, StrList[0]);

			strcpy(Str1, StrList[0]);
			StrReplaceChar(Str1, '/', '_');
			StrReplaceChar(Str1, '[', '_');
			StrReplaceChar(Str1, ']', '_');
			Hierarchy[NumberOfHierarchy]->CellName = (char *)malloc(strlen(Str1) + 2);
			strcpy(Hierarchy[NumberOfHierarchy]->CellName, Str1);

			for (CellTypeIndex = 0;CellTypeIndex < NumberOfCellTypes;CellTypeIndex++)
			{
				for (CaseIndex = 0;CaseIndex < CellTypes[CellTypeIndex]->NumberOfCases;CaseIndex++)
					if (!strcmp(StrList[1], CellTypes[CellTypeIndex]->Cases[CaseIndex]))
					{
						CaseIndex = -1;
						break;
					}

				if (CaseIndex == -1)
					break;
			}

			if (CellTypeIndex >= NumberOfCellTypes)
			{
				printf("type ""%s"" in cell report file not found in the library\n", StrList[1]);
				fclose(CellReportFile);
				free(Str1);
				for (i = 0;i < 5;i++)
					free(StrList[i]);
				return 1;
			}

			Hierarchy[NumberOfHierarchy]->Type = CellTypeIndex;
			NumberOfHierarchy++;

			do {
				ch = fgetc(CellReportFile);
			} while (ch != '\n');
		}
	} while ((strstr(StrList[0], "--------------------") == NULL) & (!feof(CellReportFile)));

	fclose(CellReportFile);
	free(Str1);

	if (strstr(StrList[0], "--------------------") == NULL)
	{
		printf("cell file seems not correct\n");
		for (i = 0;i < 5;i++)
			free(StrList[i]);		return 1;
	}

	for (i = 0;i < 5;i++)
		free(StrList[i]);
	return 0;
}

//***************************************************************************************
