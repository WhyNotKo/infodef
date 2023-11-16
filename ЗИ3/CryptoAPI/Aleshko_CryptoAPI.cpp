// CryptoAPI.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "Aleshko_CryptoAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#pragma comment(lib, "crypt32.lib")

CWinApp theApp;

class CryptoAPI
{
	HCRYPTPROV m_hCP = NULL;
	HCRYPTKEY m_hExchangeKey = NULL;
	HCRYPTKEY m_hSessionKey = NULL;
	HCRYPTKEY m_hExportKey = NULL;
public:

	HCRYPTKEY GetExchangeKey()
	{
		return m_hExchangeKey;
	}

	HCRYPTKEY GetSessionKey()
	{
		return m_hSessionKey;
	}

	HCRYPTKEY GetExportKey()
	{
		return m_hExportKey;
	}

	CryptoAPI()
	{
		if (!CryptAcquireContext(&m_hCP, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))	// использование временных ключей, не сохраняющихся в контейнере
			PrintError();
		//		if (!CryptAcquireContext(&m_hCP, "My Container", MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0))
		//		{
		//			if (GetLastError() == NTE_BAD_KEYSET)
		//			{
		//				if (!CryptAcquireContext(&m_hCP, "My Container", MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET))
		//					PrintError();
		//			}
		//		}
	}

	~CryptoAPI()
	{
		DestroyKeys();
		if (m_hCP)
		{
			if (!CryptReleaseContext(m_hCP, 0))
				PrintError();
		}
	}

	void GenKeyPair()
	{
		if (!CryptGenKey(m_hCP, CALG_RSA_KEYX, CRYPT_EXPORTABLE, &m_hExchangeKey))
			PrintError();
	}

	void GenSessionKey()
	{
		if (!CryptGenKey(m_hCP, CALG_AES_256, CRYPT_EXPORTABLE, &m_hSessionKey))
			PrintError();
	}

	void GenExportKey(const string& sPassword)
	{
		HCRYPTHASH hHash;
		if (!CryptCreateHash(m_hCP, CALG_SHA_256, NULL, 0, &hHash))
		{
			PrintError();
			return;
		}
		if (!CryptHashData(hHash, (BYTE*)sPassword.c_str(), sPassword.length(), 0))
		{
			PrintError();
			return;
		}

		if (!CryptDeriveKey(m_hCP, CALG_AES_256, hHash, CRYPT_EXPORTABLE, &m_hExportKey))
			PrintError();

		CryptDestroyHash(hHash);
	}

	void DestroyKey(HCRYPTKEY& hKey)
	{
		if (hKey)
		{
			if (!CryptDestroyKey(hKey))
				PrintError();
			hKey = NULL;
		}
	}

	void DestroyKeys()
	{
		DestroyKey(m_hExchangeKey);
		DestroyKey(m_hSessionKey);
		DestroyKey(m_hExportKey);
	}

	void DoExportKey(vector<char>& v, HCRYPTKEY hKey, HCRYPTKEY hExpKey, DWORD dwType)
	{
		DWORD dwLen = 0;
		if (!CryptExportKey(hKey, hExpKey, dwType, 0, NULL, &dwLen))
		{
			PrintError();
			return;
		}
		v.resize(dwLen);
		if (!CryptExportKey(hKey, hExpKey, dwType, 0, (BYTE*)v.data(), &dwLen))
			PrintError();
		v.resize(dwLen);		// поскольку для некоторых ключей реальный размер экспортированных данных 
		// может быть меньше размера, необходимого для экспорта
	}

	void DoImportKey(vector<char>& v, HCRYPTKEY& hKey, HCRYPTKEY hPubKey, DWORD dwType)
	{
		if (!CryptImportKey(m_hCP, (BYTE*)v.data(), v.size(), hPubKey, CRYPT_EXPORTABLE, &hKey))
			PrintError();
	}

	void ExportPublicKey(vector<char>& v)
	{
		DoExportKey(v, m_hExchangeKey, NULL, PUBLICKEYBLOB);
	}

	void ExportPrivateKey(vector<char>& v)
	{
		DoExportKey(v, m_hExchangeKey, m_hExportKey, PRIVATEKEYBLOB);
	}

	void ExportSessionKey(vector<char>& v)
	{
		DoExportKey(v, m_hSessionKey, m_hExchangeKey, SIMPLEBLOB);
	}

	void ImportPublicKey(vector<char>& v)
	{
		DoImportKey(v, m_hExchangeKey, NULL, PUBLICKEYBLOB);
	}

	void ImportPrivateKey(vector<char>& v)
	{
		DoImportKey(v, m_hExchangeKey, m_hExportKey, PRIVATEKEYBLOB);
	}

	void ImportSessionKey(vector<char>& v)
	{
		DoImportKey(v, m_hSessionKey, NULL, SIMPLEBLOB);
	}

	void EncryptData(ifstream& in, ofstream& out, DWORD dwSize, HCRYPTKEY hKey = NULL, bool bRSA = false)
		// CryptGetKeyParam с KP_BLOCKLEN возвращает размер блока в битах, 
		// для большинства алгоритмов можно использовать кратное значение,
		// но RSA требует точного соответствия длине блока в байтах,
		// причем 11 байт нужны для обязательного заполнителя (padding)
	{
		if (!hKey)
			hKey = m_hSessionKey;
		DWORD dwBlockLen = 0;
		DWORD dwDataLen = sizeof(DWORD);
		if (!CryptGetKeyParam(hKey, KP_BLOCKLEN, (BYTE*)&dwBlockLen, &dwDataLen, 0))
			PrintError();
		writeln("Block length: ", dwBlockLen);

		if (bRSA)
		{
			dwBlockLen >>= 3;
			dwBlockLen -= 11;
		}

		DWORD dwDone = 0;
		vector<char> v(dwBlockLen);

		bool bDone = false;
		while (!bDone)
		{
			in.read(v.data(), dwBlockLen);
			DWORD dwRead = (DWORD)in.gcount();
			dwDone += dwRead;
			bDone = (dwDone == dwSize);
			dwDataLen = dwRead;
			if (!CryptEncrypt(hKey, NULL, bDone, 0, NULL, &dwDataLen, 0))
				PrintError();
			if (dwDataLen > v.size())
				v.resize(dwDataLen);
			if (!CryptEncrypt(hKey, NULL, bDone, 0, (BYTE*)v.data(), &dwRead, v.size()))
				PrintError();
			out.write(v.data(), dwRead);
		}
	}

	void DecryptData(ifstream& in, ofstream& out, DWORD dwSize, HCRYPTKEY hKey = NULL, bool bRSA = false)
	{
		if (!hKey)
			hKey = m_hSessionKey;
		DWORD dwBlockLen = 0;
		DWORD dwDataLen = sizeof(DWORD);
		if (!CryptGetKeyParam(hKey, KP_BLOCKLEN, (BYTE*)&dwBlockLen, &dwDataLen, 0))
			PrintError();
		writeln("Block length: ", dwBlockLen);

		if (bRSA)
		{
			dwBlockLen >>= 3;
		}

		DWORD dwDone = 0;
		vector<char> v(dwBlockLen);

		bool bDone = false;
		while (!bDone)
		{
			in.read(v.data(), dwBlockLen);
			DWORD dwRead = (DWORD)in.gcount();
			dwDone += dwRead;
			bDone = (dwDone == dwSize);
			if (!CryptDecrypt(hKey, NULL, bDone, 0, (BYTE*)v.data(), &dwRead))
				PrintError();
			out.write(v.data(), dwRead);
		}
	}

	void EncryptData(vector<char>& vIn, vector<char>& vOut, HCRYPTKEY hKey = NULL, bool bRSA = false)
	{
		if (!hKey)
			hKey = m_hSessionKey;
		DWORD dwBlockLen = 0;
		DWORD dwDataLen = sizeof(DWORD);
		if (!CryptGetKeyParam(hKey, KP_BLOCKLEN, (BYTE*)&dwBlockLen, &dwDataLen, 0))
			PrintError();
		writeln("Block length: ", dwBlockLen);

		if (bRSA)
		{
			dwBlockLen >>= 3;
			dwBlockLen -= 11;
		}

		DWORD dwDone = 0;
		vector<char> v(dwBlockLen);

		bool bDone = false;
		while (!bDone)
		{
			DWORD dwRead = min(dwBlockLen, vIn.size() - dwDone);
			memcpy(v.data(), vIn.data() + dwDone, dwRead);
			dwDone += dwRead;
			bDone = (dwDone == vIn.size());
			dwDataLen = dwRead;
			if (!CryptEncrypt(hKey, NULL, bDone, 0, NULL, &dwDataLen, 0))
				PrintError();
			if (dwDataLen > v.size())
				v.resize(dwDataLen);
			if (!CryptEncrypt(hKey, NULL, bDone, 0, (BYTE*)v.data(), &dwRead, v.size()))
				PrintError();
			vOut.insert(vOut.end(), v.begin(), v.begin() + dwRead);
		}
	}

	void DecryptData(vector<char>& vIn, vector<char>& vOut, HCRYPTKEY hKey = NULL, bool bRSA = false)
	{
		if (!hKey)
			hKey = m_hSessionKey;
		DWORD dwBlockLen = 0;
		DWORD dwDataLen = sizeof(DWORD);
		if (!CryptGetKeyParam(hKey, KP_BLOCKLEN, (BYTE*)&dwBlockLen, &dwDataLen, 0))
			PrintError();
		writeln("Block length: ", dwBlockLen);

		if (bRSA)
		{
			dwBlockLen >>= 3;
		}

		DWORD dwDone = 0;
		vector<char> v(dwBlockLen);

		bool bDone = false;
		while (!bDone)
		{
			DWORD dwRead = min(dwBlockLen, vIn.size() - dwDone);
			memcpy(v.data(), vIn.data() + dwDone, dwRead);
			dwDone += dwRead;
			bDone = (dwDone == vIn.size());
			if (!CryptDecrypt(hKey, NULL, bDone, 0, (BYTE*)v.data(), &dwRead))
				PrintError();
			vOut.insert(vOut.end(), v.begin(), v.begin() + dwRead);
		}
	}
};
//void CryptoTest()
//{
//	{
//		CryptoAPI crypto;
//
//		crypto.GenKeyPair();
//		crypto.GenSessionKey();
//		crypto.GenExportKey("12345");
//
//		{
//			vector<char> v;
//			crypto.ExportPrivateKey(v);
//			ofstream out("private.key", ios::binary);
//			out.write(v.data(), v.size());
//		}
//
//		{
//			vector<char> v;
//			crypto.ExportPublicKey(v);
//			ofstream out("public.key", ios::binary);
//			out.write(v.data(), v.size());
//		}
//
//		// CryptExportKey не шифрует, а подписывает сессионные ключи, поэтому лучше использовать следующий блок
//		{
//			vector<char> v;
//			crypto.ExportSessionKey(v);
//			ofstream out("session.key", ios::binary);
//			out.write(v.data(), v.size());
//		}
//
//		{
//			vector<char> v1;
//			vector<char> v2;
//			crypto.ExportSessionKey(v1);
//			crypto.EncryptData(v1, v2, crypto.GetExchangeKey(), true);
//			ofstream out("session.enc.key", ios::binary);
//			out.write(v2.data(), v2.size());
//		}
//
//		{
//			ifstream in("CryptoAPI.cpp", ios::binary);
//			ofstream out("CryptoAPI.cpp.enc", ios::binary);
//			crypto.EncryptData(in, out, (DWORD)filesystem::file_size("CryptoAPI.cpp"));
//		}
//	}
//
//	
//	{
//		CryptoAPI crypto;
//
//		crypto.GenExportKey("12345");
//		{
//			ifstream in("private.key", ios::binary);
//			vector v(istreambuf_iterator<char>{in}, {});
//			crypto.ImportPrivateKey(v);
//		}
//
//		{
//			ifstream in("public.key", ios::binary);
//			vector v(istreambuf_iterator<char>{in}, {});
//			crypto.ImportPublicKey(v);
//		}
//
//		{
//			ifstream in("session.key", ios::binary);
//			vector v(istreambuf_iterator<char>{in}, {});
//			crypto.ImportSessionKey(v);
//		}
//
//		{
//			ifstream in("session.enc.key", ios::binary);
//			vector v1(istreambuf_iterator<char>{in}, {});
//			vector<char> v2;
//			crypto.DecryptData(v1, v2, crypto.GetExchangeKey(), true);
//			crypto.ImportSessionKey(v2);
//		}
//
//		{
//			ifstream in("CryptoAPI.cpp.enc", ios::binary);
//			ofstream out("CryptoAPI.cpp.dec", ios::binary);
//			crypto.DecryptData(in, out, (DWORD)filesystem::file_size("CryptoAPI.cpp.enc"));
//		}
//	}
//}

// шифрование данных файла  
void CryptoEnc(CryptoAPI& crypto, string filename)
{
	ifstream in("public.txt", ios::binary);
	// Std::istreambuf_iterator - это однопроходный входной итератор, 
	// который считывает последовательные символы из объекта
	// запись в вектор с которым будем работать 
	vector v(istreambuf_iterator<char>{in}, {});
	// получение из каналов информации значения ключа.
	crypto.ImportPublicKey(v);
	//создаем сессионный ключ для шифрования данных 
	crypto.GenSessionKey();

	{
		vector<char> v1;
		vector<char> v2;
		// передаем сессионный ключ 
		crypto.ExportSessionKey(v1);
		// шифруем данные по переданному сессионному ключу  
		crypto.EncryptData(v1, v2, crypto.GetExchangeKey(), true);

		ofstream out(filename + "enc.txt", ios::binary);
		ifstream in(filename + ".txt", ios::binary);
		int keysize = v2.size();
		out << keysize;
		//cout << "Длина ключа: ";
		//cout << keysize << endl;
		out << ";";
		out.write(v2.data(), v2.size());
		vector<char> v3(istreambuf_iterator<char>{in}, {});
		vector<char> v4;

		crypto.EncryptData(v3, v4); 
		out.write(v4.data(), v4.size());

	}
}
// расшифровывание закодироаваного фала  
void CryptoDec(CryptoAPI& crypto, string password, string filename)
{
	// получаем закрытый ключ
	crypto.GenExportKey(password);
	{

		ifstream in("private.key", ios::binary);
		vector v(istreambuf_iterator<char>{in}, {});
		// используем полученый приватный ключ 
		crypto.ImportPrivateKey(v);
	}

	{
		string length;
		vector<char> v1;
		ifstream in(filename + "enc.txt", ios::binary);
		while (true)
		{
			char symb;
			in.get(symb);
			if (symb == ';')
				break;
			length.push_back(symb);
		}
		int int_length = stoi(length);
		for (int i = 0; i < int_length; ++i) {
			char symb;
			in.get(symb);
			v1.push_back(symb);
		}

		vector<char> v2;
		crypto.DecryptData(v1, v2, crypto.GetExchangeKey(), true);
		crypto.ImportSessionKey(v2);
		vector<char> v3(istreambuf_iterator<char>{in}, {});
		vector<char> v4;

		// расшифровываем файл 
		crypto.DecryptData(v3, v4);
		ofstream out(filename + "dec.txt", ios::binary);
		out.write(v4.data(), v4.size() - 1);
	}

}

void CryptoKey(CryptoAPI& crypto, string password)
{
	// создается случайный ключ 
	crypto.GenKeyPair();
	// хеширование ключа создали на основе пароля 
	crypto.GenExportKey(password);

	{
		vector<char> v;
		crypto.ExportPrivateKey(v);
		// передаем приватный ключ в файл 
		ofstream out("private.key", ios::binary);
		out.write(v.data(), v.size());
	}

	{
		vector<char> v;
		crypto.ExportPublicKey(v);
		// выводим второй ключ в файл
		ofstream out("public.txt", ios::binary);
		out.write(v.data(), v.size());
	}
}


int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: code your application's behavior here.
			wprintf(L"Fatal Error: MFC initialization failed\n");
			nRetCode = 1;
		}
		else
		{
			CryptoAPI crypto;
			string filename = "default";

			while (true)
			{
				cout << "1. Зашифровать" << endl;
				cout << "2. Расшифровать" << endl;
				cout << "3. Создать ключи" << endl;
				cout << "4. Ввести имя файла" << endl;
				cout << "0. Выход" << endl;
				cout << "Выберите пункт: ";
				int a;
				cin >> a;
				switch (a)
				{
				case(1):
				{
					CryptoEnc(crypto, filename);
					break;
				}
				case(2):
				{
					cout << endl << "Введите ключ: ";
					string password;
					cin.ignore();
					getline(cin, password);
					CryptoDec(crypto, password, filename);
					break;
				}
				case(3):
				{
					cout << endl << "Введите ключ: ";
					string password;
					cin.ignore();
					getline(cin, password);
					CryptoKey(crypto, password);
					break;
				}
				case(4):
				{

					auto s = crypto.GetSessionKey();
					cout << s;
					cin.ignore();
					filename = "";
					getline(cin, filename);
					//Да тут нужны были бы проверки на открываемость файла но мне лень
					break;
				}
				case(0):
				{
					return 0;
					break;
				}
				default:
					break;
				}

			}
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		wprintf(L"Fatal Error: GetModuleHandle failed\n");
		nRetCode = 1;
	}

	return nRetCode;
}


// aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa