#pragma once
#include <iostream>
#include <fstream>
#include <string>
//#include<list> // for list functions
#include <vector>
#include <sstream>
#include <iterator>
#include <time.h>

using namespace std;

#define ValueCount                  6    //transactions values count  of each  file
#define FileCount                   4    //count of transaction files

//state definition
#define SUCCESS                "SUCCESS"
#define FAILED                 "FAILED"
#define PENDING                "PENDING"

//file definition
#define PproIndex                0
#define SimplerpayIndex          1
#define SimplepayIndex           2
#define HarderpayIndex           3

//transaction definition;TotalCount,MatchedCount,MissedCount,TotalVolume,MatchedVolume,MissedVolume
#define TotalCountIn           0
#define MatchedCountIn         1
#define MissedCountIn          2
#define TotalVolumeIn          3
#define MatchedVolumeIn        4
#define MissedVolumeIn         5




class Reconciliation
{
public:
		
	int StartLoadAndParse(double data[FileCount][ValueCount]);
	void CheckTransationFile(string TxnId, string MrTxnId, string State, double AmountFrom, double data[FileCount][ValueCount], string FileName, int FileIndex, int LoopIndex);
	bool IsDouble(const std::string& s);
	std::vector<std::string> SplitLine(const std::string& StreamLine, char delimiter);
	void Output(double data[FileCount][ValueCount]);

	Reconciliation();
	~Reconciliation();
};

