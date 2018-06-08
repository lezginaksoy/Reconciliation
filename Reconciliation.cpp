#include "Reconciliation.h"

Reconciliation _Reconciliation;


Reconciliation::Reconciliation()
{
}

int main(void)
{
	static double Data[FileCount][ValueCount];
	
	//clock_t start, stop;
	//start = clock();

	int Ret = _Reconciliation.StartLoadAndParse(Data);

	if (Ret == 1)
		_Reconciliation.Output(Data);

	/*stop = clock();
	printf("Elapsed time: %f\n", (float)(stop - start) / CLOCKS_PER_SEC);
*/

	return Ret;

}


//Start Load File and parsing
int  Reconciliation::StartLoadAndParse(double Data[FileCount][ValueCount])
{
	try
	{
		//Reconciliation _Reconciliation;
		
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
			std::vector<std::string> FromArr = _Reconciliation.SplitLine(sLine, ',');

			for (size_t i = 0; i < FromArr.size(); i++)
			{
				if (_Reconciliation.IsDouble(FromArr[i]))
					indexOfAmount = i;
				else if (FromArr[i] == SUCCESS || FromArr[i] == FAILED || FromArr[i] == PENDING)
					indexOfState = i;
				else if (indexOfTxnId == -1)
					indexOfTxnId = i;
				else
					indexOfMrId = i;
			}

			Data[PproIndex][TotalCountIn] += 1;//increase count of Txn in main file(ppro)
			Amount = std::stod(FromArr[indexOfAmount], &sz);
			Data[PproIndex][TotalVolumeIn] += std::stod(FromArr[indexOfAmount], &sz);//amount for calculate total of Main file(ppro)

			_Reconciliation.CheckTransationFile(FromArr[indexOfTxnId], FromArr[indexOfMrId], FromArr[indexOfState], Amount, Data, "transactions-simplerpay.csv", SimplerpayIndex, LoopIndex);
			_Reconciliation.CheckTransationFile(FromArr[indexOfTxnId], FromArr[indexOfMrId], FromArr[indexOfState], Amount, Data, "transactions-simplepay.csv", SimplepayIndex, LoopIndex);
			_Reconciliation.CheckTransationFile(FromArr[indexOfTxnId], FromArr[indexOfMrId], FromArr[indexOfState], Amount, Data, "transactions-harderpay.csv", HarderpayIndex, LoopIndex);

			LoopIndex++;
		}

		ip.close();


		return 1;
	}
	catch (const std::exception&)
	{
		return 0;
	}


}

//for each transaction of main file, check it in other transaction files 
void Reconciliation::CheckTransationFile(string TxnId, string MrTxnId, string State, double AmountFrom, double Data[FileCount][ValueCount], string FileName, int FileIndex, int LoopIndex)
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

		int indexFound =int(std::find(ToArr.begin(), ToArr.end(), TxnId) - ToArr.begin());
		if (indexFound >= (int)ToArr.size())//if TransactionId not find try find merchantTransactionId
			indexFound = int( std::find(ToArr.begin(), ToArr.end(), MrTxnId) - ToArr.begin());

		//find amuount and state values in line
		for (size_t i = 0; i < ToArr.size(); i++)
		{
			if (IsDouble(ToArr[i]))
				indexOfAmount =int(i);
			else if (ToArr[i] == SUCCESS || ToArr[i] == FAILED || ToArr[i] == PENDING)
				indexOfState = i;
		}

		if (LoopIndex == 0)//in first loop of Ppro could enough for calculate total count and totalVolume of second file
		{
			Data[FileIndex][TotalCountIn] += 1;
			Data[FileIndex][TotalVolumeIn] += std::stod(ToArr[indexOfAmount], &sz);
		}


		if (indexFound < (int)ToArr.size())//transaction or merchantid is matched
		{
			//check state,and calculate matchedcount and matchedvolume
			indexFound = std::find(ToArr.begin(), ToArr.end(), State) - ToArr.begin();
			if (indexFound < (int)ToArr.size())
			{
				Data[PproIndex][MatchedCountIn] += 1;
				Data[FileIndex][MatchedCountIn] += 1;
				Data[PproIndex][MatchedVolumeIn] += AmountFrom;
				Data[FileIndex][MatchedVolumeIn] += std::stod(ToArr[indexOfAmount], &sz);

			}
		}

	}

	ip.close();

}

//find amount coloumn
bool Reconciliation::IsDouble(const std::string& s)
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

//parse line and create vector of of txn value
std::vector<std::string> Reconciliation::SplitLine(const std::string& StreamLine, char delimiter)
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

//write conc.
void Reconciliation:: Output(double Data[FileCount][ValueCount])
{
	std::ofstream out("output.txt");
	string FilesName[4] = { "transactions-ppro.csv","transactions-simplerpay.csv","transactions-simplepay.csv","transactions-harderpay.csv" };
	double count, Amount;

	for (size_t i = 0; i < FileCount; i++)
	{
		out << FilesName[i];
		out << "\nThis File has ";
		count = Data[i][TotalCountIn] - Data[i][MatchedCountIn];
		out << count;
		out << " Unknown transaction \n";

		Amount = Data[i][TotalVolumeIn];
		out << "Total Volume :";
		out << Amount;
		out << "\n";

		Amount = Data[i][MatchedVolumeIn];
		out << "Matched Volume :";
		out << Amount;
		out << "\n";

		Amount = Data[i][TotalVolumeIn] - Data[i][MatchedVolumeIn];
		out << "Missing Volume :";
		out << Amount;

		out << "\n";
		out << "\n";

	}

	out.close();
}


















Reconciliation::~Reconciliation()
{
}
