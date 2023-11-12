#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <Windows.h>

using namespace std;

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

// Определение частоты каждого символа в тексте
vector<int> get_period(vector<unsigned char>& text) {
    vector<int> period(256, 0);
    for (auto j : text) {
        period[j]++;
    }
    return period;

}

// Вывод n самых популярных символов эталонного текста
void print_first_n(vector<unsigned char>& text, int n) {
    vector<int > period = get_period(text);
    unordered_map <int, int> period1;
    for (int i = 0; i < 256; i++)
        period1.insert({ period[i], i });
    sort(period.begin(), period.end(), std::greater<int>());
    for (int i = 0; i < n; i++)
    {
        cout << i + 1 << ". " << char(period1[period[i]]) << "', количество: " << period[i] << endl;
    }
}

// нахождение индекса соответсвия
float get_index(vector<unsigned char>& text) {
    vector<int> period;
    period = get_period(text);
    float index = 0;
    float sum = 0;
    for (int j = 0; j < period.size(); j++)
        sum += (float)period[j] * (float)(period[j] - 1);
    index = sum / (float)(text.size() * (float)(text.size() - 1));
    return index;
}

// нахождение самого часто встречающегося символа
unsigned char the_most_popular(vector<int>& text) {
    unsigned char pop_char;
    int max = -1;
    for (int i = 0; i < 256; i++)
        if (text[i] > max) {
            pop_char = i;
            max = text[i];
        }
    return pop_char;
    //cout << "Самый популярный символ в группе: " << ;
}

//поиск длины ключа из заданого интервалла с помощью индексов соответсвия 
int key_length(vector<unsigned char>& text_vizh) {
    //setlocale(LC_ALL, "Russian");
    cout << "Введите через пробел диапазон символов искомого ключа:" << endl;
    int left = 1, right = 30;
    //cin >> left; cin >> right;

    int keylenght = 0;
    double max_index = 0;
    double index = 0;
    vector<double> all_index;

    for (int length = left; length <= right; length++) {
        cout << "Анализ " << length << " значного ключа" << endl;

        vector<unsigned char> temporary;
        for (int i = 0; i < text_vizh.size(); i+= length) {
            temporary.push_back(text_vizh[i]);
        }


        index = get_index(temporary);
        cout << "Полученый индекс соответсвия: " << index << endl;
        all_index.push_back(index);

    }
    for (int i = 1; i < all_index.size() - 1; i++) {
        if (all_index[i] > (all_index[i - 1] + all_index[i + 1])) {
            max_index = all_index[i + 1];
            keylenght = i + 1;
            break;
        }
    }
    cout << "\nПредполагаемая длина ключа: " << keylenght << endl;
    return keylenght;
}

// Нахождение ключа путем разбиения исходного текста на 
// количесво групп соответсвующих количеству символов в 
// ключе и нахождении самого популярного символа в каждой группе 
string find_key(vector<unsigned char>& text_vizh, vector<unsigned char>& text_etalon, const int& keylength) {
    //setlocale(LC_ALL, "Russian");
    //SetConsoleCP(1251);
    //SetConsoleOutputCP(1251);
    vector<int> period;
    period = get_period(text_etalon);
    unsigned char pop_char = the_most_popular(period);
    vector<vector<unsigned char>> groups;
    vector<unsigned char> temporary;
    for (int i = 0; i < keylength; i++) {
        for (int j = i; j < text_vizh.size(); j++) {
            if ((j - i) % keylength == 0)
                temporary.push_back(text_vizh[j]);

        }

        groups.push_back(temporary);
        temporary.clear();
    }

    string key;

    for (int j = 0; j < keylength; j++) {
        vector<int> temporary(256, 0);
        /* for (int i = 0; i < 256; i++)
             temporary.push_back(0);*/
        for (auto i : groups[j])
            temporary[i]++;
        unsigned char code = the_most_popular(temporary);
        key.push_back(code - pop_char);
    }
    
    ofstream fout("key.txt", ios::binary);
    
    cout << "\nНайденый ключ: ";
    for (int j = 0; j < keylength; j++) {

        cout << key[j];
        fout << key[j];
    }
    fout.close();
        
    cout << endl;
    return key;
}

// расшифровка текста по найденому ключу
void vizhener(string& inpath, string& key) {
    //setlocale(LC_ALL, "Russian");
    string output_file;
    cout << "Введите название файла для записи: " << endl;
    //cin.ignore();
    getline(cin, output_file);

    ifstream fin(inpath, ios::binary);
    ofstream fout(output_file + ".txt", ios::binary);

    if (!fin.is_open() or !fout.is_open())
    {
        cout << "Ошибка открытия файла!";
    }

    int token = -1;

    char symbol;
    int i = 0;
    unsigned char codedSymbol, keySymbol;
    while (fin.get(symbol))
    {
        keySymbol = key[i % key.size()];
        codedSymbol = symbol + token * keySymbol;
        fout.put(codedSymbol);
        i++;
    }
    fin.close();
    fout.close();
}

vector <unsigned char> get_text_vector(string& text)
{
    //char symbol;
    ifstream fin(text, ios::binary);
    /*while (fin.get(symbol))
    {
        finish.push_back(symbol);
    }*/
    vector<unsigned char> v((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();
    return v;
}

void decoding()
{
    //setlocale(LC_ALL, "Russian");
    string input_text1, input_text2;

    cout << "Введите имя файла закодированого текста: " << endl;
    cin.ignore();
    getline(cin, input_text1);
    input_text1 = input_text1 + ".txt";

    cout << "Введите имя эталонного текста: " << endl;
    getline(cin, input_text2);
    input_text2 = input_text2 + ".txt";


    vector <unsigned char> text_vizh = get_text_vector(input_text1);

    vector <unsigned char> text_etalon = get_text_vector(input_text2);

    print_first_n(text_etalon, 3);
    int keylength = key_length(text_vizh);
    string key = find_key(text_vizh, text_etalon, keylength);
    vizhener(input_text1, key);
}



int main()
{
    setlocale(LC_ALL, "");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    int menu = 0;

    while (true)
    {
        cout << "Меню \n1. Декодировать \n \n0. Выход \n\n ";
        menu = GetCorrectNumber(0, 1);
        if (menu == 1)
        {
            decoding();
        }
        if (menu == 0)
        {
            cout << "Программа завершена." << endl;
            return 0;
        }
    }
}