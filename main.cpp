#include <iostream>
#include <fstream>
#include <string>
#include<list> // for list functions
#include <vector>
#include <sstream>
#include <iterator>

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


void CheckTransationFile(string TxnId, string MrTxnId, string State, double AmountFrom, double data[FileCount][ValueCount], string FileName, int FileIndex, int LoopIndex);
bool IsDouble(const std::string& s);
std::vector<std::string> SplitLine(const std::string& StreamLine, char delimiter);
void Output(double data[FileCount][ValueCount]);


int main()
{
	static double data[FileCount][ValueCount];
	int indexOfState, indexOfAmount, indexOfTxnId = -1, indexOfMrId;
	string Header, sLine;
	double Amount;
	std::string::size_type sz;

	ifstream ip("transactions-ppro.csv");
	if (!ip.is_open()) std::cout << "ERROR: File Open" << '\n';
 	getline(ip, Header);

	int LoopIndex = 0;
	while (ip.good()) {

		getline(ip, sLine);
		std::vector<std::string> FromArr = SplitLine(sLine, ',');

		for (size_t i = 0; i < FromArr.size(); i++)
		{
			if (IsDouble(FromArr[i]))
				indexOfAmount = i;
			else if (FromArr[i] == SUCCESS || FromArr[i] == FAILED || FromArr[i] == PENDING)
				indexOfState = i;
			else if (indexOfTxnId == -1)
				indexOfTxnId = i;
			else
				indexOfMrId = i;
		}

		data[PproIndex][TotalCountIn] += 1;//increase count of Txn in main file(ppro)
		Amount = std::stod(FromArr[indexOfAmount], &sz);
		data[PproIndex][TotalVolumeIn] += std::stod(FromArr[indexOfAmount], &sz);//amount for calculate total of Main file(ppro)

		CheckTransationFile(FromArr[indexOfTxnId], FromArr[indexOfMrId], FromArr[indexOfState], Amount, data, "transactions-simplerpay.csv",SimplerpayIndex, LoopIndex);
		CheckTransationFile(FromArr[indexOfTxnId], FromArr[indexOfMrId], FromArr[indexOfState], Amount, data, "transactions-simplepay.csv",SimplepayIndex, LoopIndex);
		CheckTransationFile(FromArr[indexOfTxnId], FromArr[indexOfMrId], FromArr[indexOfState], Amount, data, "transactions-harderpay.csv",HarderpayIndex, LoopIndex);

		LoopIndex++;
	}

	ip.close();


	Output(data);

	return 0;
}

void Output(double data[FileCount][ValueCount])
{
	std::ofstream out("output.txt");
	string FilesName[4] = {"transactions-ppro.csv","transactions-simplerpay.csv","transactions-simplepay.csv","transactions-harderpay.csv"};
	double count,Amount;
	
	for (size_t i = 0; i < FileCount; i++)
	{
		out <<FilesName[i];
		out << "\nThis File has ";
		count=data[i][TotalCountIn] - data[i][MatchedCountIn];
		out <<count; 
		out << " Unknown transaction \n";

		Amount= data[i][TotalVolumeIn];
		out << "Total Volume :";
		out <<Amount ;
		out << "\n";
		
		Amount= data[i][MatchedVolumeIn];
		out << "Matched Volume :";
		out << Amount;
		out << "\n";
		
		Amount= data[i][TotalVolumeIn]- data[i][MatchedVolumeIn];
		out << "Missing Volume :";
		out << Amount;
		
		out << "\n";
		out << "\n";

	}
		
	out.close();
}

bool IsDouble(const std::string& s)
{
	int dotCount = 0;
	if (s.empty())
		return false;

	for (char c : s)
	{
		if (!(isdigit(c) || c == '.') && dotCount > 1)
		{
			return false;
		}
		dotCount += (c == '.');
	}

	if (dotCount == 1)
		return true;
	else
		return false;
}

//create vector of of txn value
std::vector<std::string> SplitLine(const std::string& StreamLine, char delimiter)
{
	std::vector<std::string> Values;
	std::string Value;
	std::istringstream ValueStream(StreamLine);
	while (std::getline(ValueStream, Value, delimiter))
	{
		Values.push_back(Value);
	}
	return Values;
}

void CheckTransationFile(string TxnId, string MrTxnId, string State, double AmountFrom, double data[FileCount][ValueCount], string FileName, int FileIndex, int LoopIndex)
{
	ifstream ip(FileName);
	if (!ip.is_open()) std::cout << "ERROR: File Open" << '\n';

	string Header, ToLine;
	std::string::size_type sz;
	getline(ip, Header);

	int indexOfState, indexOfAmount, indexOfTxnId = -1;// , indexOfMrId;


	while (ip.good())
	{
		getline(ip, ToLine);
		std::vector<std::string> ToArr = SplitLine(ToLine, ',');
		
		if (LoopIndex == 0)//in first loop of Ppro could enough for calculate total count and totalVolume of second file
		{
			data[FileIndex][TotalCountIn] += 1;
			data[FileIndex][TotalVolumeIn] += std::stod(ToArr[indexOfAmount], &sz);
		}



		int indexFound = std::find(ToArr.begin(), ToArr.end(), TxnId) - ToArr.begin();
		if (indexFound >= ToArr.size())//if TransactionId not find try find merchantTransactionId
			indexFound = std::find(ToArr.begin(), ToArr.end(), MrTxnId) - ToArr.begin();

		//find amuount and state values in line
		for (size_t i = 0; i < ToArr.size(); i++)
		{
			if (IsDouble(ToArr[i]))
				indexOfAmount = i;
			else if (ToArr[i] == SUCCESS || ToArr[i] == FAILED || ToArr[i] == PENDING)
				indexOfState = i;
		}

		
		if (indexFound < ToArr.size())//transaction or merchantid is matched
		{
			//check state,and calculate matchedcount and matchedvolume
			indexFound = std::find(ToArr.begin(), ToArr.end(), State) - ToArr.begin();
			if (indexFound < ToArr.size())
			{
				data[PproIndex][MatchedCountIn] += 1;
				data[FileIndex][MatchedCountIn] += 1;
				data[PproIndex][MatchedVolumeIn] += AmountFrom;
				data[FileIndex][MatchedVolumeIn] += std::stod(ToArr[indexOfAmount], &sz);

			}
		}

	}

	ip.close();

}
