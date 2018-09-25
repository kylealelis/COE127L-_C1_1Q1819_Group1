
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <map>
#include <random>
#include <cmath>

using namespace std;

int main() {
	string line;
	vector <double> fa, va, ca, rs, c, fsd ; 
	vector <double> tsd, d, ph, s, a, q;



	ifstream wineFile;
	wineFile.open("file1.csv");
	double value;
	for (int i = 0; i < 11; i++)
	{
		getline(wineFile, line, ',');
		cout << line << endl;
	}

	while (wineFile.good())
	{
		for (int i = 1; i < 12; i++)
		{

			if (i == 1)
			{
				//getline(wineFile, line, ',') >> value;
				//fa.push_back(value);
				//cout << value << endl;
			}
			if (i == 2)
			{
				getline(wineFile, line, ',') >> value;
				va.push_back(value);
				//cout << "VA" << value << endl;
			}
			if (i == 3)
			{
				getline(wineFile, line, ',') >> value;
				ca.push_back(value);
				//cout << "CA" << value << endl;
			}
			if (i == 4)
			{
				getline(wineFile, line, ',') >> value;
				rs.push_back(value);
				//cout << value << endl;
			}
			if (i == 5)
			{
				getline(wineFile, line, ',') >> value;
				c.push_back(value);
				//cout << value << endl;
			}
			if (i == 6)
			{
				getline(wineFile, line, ',') >> value;
				fsd.push_back(value);
				//cout << value << endl;
			}
			if (i == 7)
			{
				getline(wineFile, line, ',') >> value;
				tsd.push_back(value);
				//cout << value << endl;
			}
			if (i == 8)
			{
				getline(wineFile, line, ',') >> value;
				d.push_back(value);
				//cout << value << endl;
			}
			if (i == 9)
			{
				getline(wineFile, line, ',') >> value;
				ph.push_back(value);
				//cout << value << endl;
			}
			if (i == 10)
			{
				getline(wineFile, line, ',') >> value;
				s.push_back(value);
				//cout << value << endl;
			}
			if (i == 11)
			{
				getline(wineFile, line, ',') >> value;
				a.push_back(value);
				//cout << value << endl;
			}
			if (i == 12)
			{
				getline(wineFile, line, ',') >> value;
				q.push_back(value);
				//cout << value << endl;
			}
		

		}
	}

	wineFile.close();

	random_device rd{};
	mt19937 gen{ rd() };
	
	normal_distribution <double> distribution(fa);

	cout << "random : " << distribution(gen) << endl;

	//else cout << "Unable to open file";

	system("pause");
	return 0;
}