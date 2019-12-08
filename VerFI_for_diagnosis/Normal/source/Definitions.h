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

#define Max_Name_Length   10000

#define CellType_Gate     0
#define CellType_Reg      1

#define SignalType_input  0
#define SignalType_output 1
#define SignalType_wire   2

#define Task_find_module_type   0
#define Task_find_module_name   1
#define Task_find_open_bracket  2
#define Task_find_point         3
#define Task_find_IO_port       4
#define Task_find_signal_name   5
#define Task_find_close_bracket 6
#define Task_find_comma         7

#define Result_Ineffective      0
#define Result_Detected         1
#define Result_Nondetected      2
#define Result_RunTimeOver      3

#define FaultInjection_Exhaustive 0
#define FaultInjection_Random     1

#define FaultInjection_stuck_at_0   0
#define FaultInjection_stuck_at_1   1
#define FaultInjection_toggle       2
#define NumberOfFaultInjectionTypes 3

struct CellTypeStruct {
	char GateOrReg;
	char NumberOfCases;
	char **Cases;
	char NumberOfInputs;
	char **Inputs;
	char NumberOfOutputs;
	char **Outputs;
	char **Expresions;
	char **Tables;
};

struct HierarchyStruct {
	char *Name;
	int  Type;
	char *CellName;
};

struct CellStruct {
	int   Type;
	char  *Name;
	int   Hierarchy;
	char  FaultAllowed;
	short Depth;
	char  NumberOfInputs;
	int   *Inputs;
	char  NumberOfOutputs;
	int   *Outputs;
	int   *RegValueIndexes;
};

struct SignalStruct {
	char  *Name;
	char  Type;
	short Depth;
	int   Output;
	int   NumberOfInputs;
	int   *Inputs;
};

struct SimulationResultStruct {
	char  Valid;
	char  NumberOfInjectedFaults;
	int*  TaregtClockCycles;
	int*  TaregtCells;
	char  Result;
	char* Output;
};

//***************************************************************************************

void StrReplaceChar(char *Str, char ch_source, char ch_destination)
{
	int i;

	for (i = 0;i < strlen(Str);i++)
	{
		if (Str[i] == ch_source)
			Str[i] = ch_destination;
	}
}

//***************************************************************************************

void ReadNonCommentFromFile(FILE* FileHeader, char* Str, const char* CommentSyntax)
{
	int  l;
	char ch;

	l = strlen(CommentSyntax);
	do {
		fscanf(FileHeader, "%s", Str);
		if (! memcmp(CommentSyntax, Str, l))
		{
			do ch = fgetc(FileHeader);
			while ((ch != '\n') & (!feof(FileHeader)));
		}
	} while ((! memcmp(CommentSyntax, Str, l)) & (!feof(FileHeader)));
}

//***************************************************************************************
