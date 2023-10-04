#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

using namespace std;

//template <typename T>
int GetCorrectNumber(int min = 0, int max = 10000000)
{
	int val;
	while (!(std::cin >> val && (std::cin.peek() == EOF || std::cin.peek() == '\n') && (val >= min) && (val <= max)))
	{
		std::cin.clear();
		std::cin.ignore(10000, '\n');
		std::cout << "Ентер зе коррект намбер... " << std::endl;
	}
	return val;
}

unsigned char Vigener(unsigned char symbol, unsigned char key, int mode)
{
	return (symbol +  mode * key);
}

void coding(bool mode)
{
	setlocale(LC_ALL, "Russian");
	string readFile, writeFile, key;

	cout << "Введите название файла для считывания: " << endl;
	cin.ignore();
	getline(cin, readFile);
	cout << "Введите название файла для записи: " << endl;
	getline(cin, writeFile);
	cout << "Введите ключ: " << endl;
	getline(cin, key);

	ifstream fin(readFile + ".txt", ios::binary);
	ofstream fout(writeFile + ".txt", ios::binary);

	if (!fin.is_open() or !fout.is_open())
	{
		cout << "Ошибка открытия файла!";
	}

	int token = mode ? 1 : -1;

	char symbol;
	int i = 0;
	unsigned char codedSymbol, keySymbol;
	while (fin.get(symbol))
	{
		keySymbol = key[i % key.size()];
		codedSymbol = Vigener(symbol, keySymbol, token);
		fout.put(codedSymbol);
		i++;
	}
	fin.close();
	fout.close();
}

int main()
{
	setlocale(LC_ALL, "Russian");
	int menu;

	while (true)
	{
		cout << "Меню \n1. Зашифровать \n2. Расшифровать \n0. Выход \n\n ";
		menu = GetCorrectNumber(0, 2);
		if (menu == 1)
		{
			coding(true);
		}
		if (menu == 2)
		{
			coding(false);
		}
		if ( menu == 0)
		{
			cout << "Программа завершена." << endl;
			return 0;
		}
	}
}

