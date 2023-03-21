#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

/// <summary>
/// JSON�� �Ľ��ؼ� �ܼ��� �ؽ�Ʈ�� ����ϴ� �Լ�
/// </summary>
/// <param name="filePath">json ���� ���</param>
void LoadJson(std::wstring& filePath)
{
	// �׽�Ʈ�� �ּ�
	const wchar_t* path = filePath.c_str();
	std::wifstream inputStream;
	inputStream.open(path);

	if (!inputStream.is_open())
	{
		if (inputStream.fail())
		{
			char buffer[256] = { 0 };
			strerror_s(buffer, errno);
			std::cout << "���� ���� ����\n" << buffer << std::endl;
		}
		return;
	}

	std::string str((std::istreambuf_iterator<wchar_t>(inputStream)), std::istreambuf_iterator<wchar_t>());

	rapidjson::Document document;
	document.Parse(str.c_str());

	rapidjson::StringBuffer buffer;
	rapidjson::Writer writer(buffer);
	document.Accept(writer);

	std::cout << buffer.GetString() << std::endl;
	inputStream.close();
	return;
}

int main()
{
	std::wcout.imbue(std::locale("korean"));

	// ini ���Ϸ� ���� ��θ� �����´�.
	wchar_t path[256] = { 0 };
	DWORD size = GetPrivateProfileString(L"JSON", L"directory", L"", path, 256, L"./filePath.ini");
	if (size == 0)
	{
		std::cout << "��θ� �������� ���߽��ϴ�.";
		return 0;
	}

	std::wcout << L"��δ� : ";
	std::wcout << path << std::endl << std::endl;

	std::wstring wildCard = path;
	wildCard += L"\\*.json";

	WIN32_FIND_DATA fileInfo;
	HANDLE hfind = FindFirstFile(wildCard.c_str(), &fileInfo);
	std::vector<std::wstring> fileNames;

	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			fileNames.emplace_back(fileInfo.cFileName);
		} while (FindNextFile(hfind, &fileInfo));
	}
	else
	{
		std::cout << "������ �����ϴ�.";
		return 0;
	}

	FindClose(hfind);

	std::cout << fileNames.size() << "���� ������ �߰��߽��ϴ�." << std::endl;
	for (int i = 0; i < fileNames.size(); ++i)
	{
		std::wcout << i + 1 << " - " << fileNames[i] << std::endl;
	}

	std::cout << "�ε��� ���� �ε����� �Է��ϼ���" << std::endl;

	int index = -1;
	std::cin >> index;
	index -= 1;

	if (index > fileNames.size() || index < 0)
	{
		std::cout << "�߸��� �ε��� �Դϴ�.";
		return 0;
	}

	std::wstring filePath = path;
	filePath += L"\\";
	filePath += fileNames[index];

	LoadJson(filePath);

	HANDLE hfile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		std::cout << "���� ���� �������� ���� ���� ���� : " << GetLastError();
		return 0;
	}
	_FILETIME changeTime;
	GetFileTime(hfile, NULL, NULL, &changeTime);
	CloseHandle(hfile);

	HANDLE hchange = FindFirstChangeNotificationW(path, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

	if (hchange == INVALID_HANDLE_VALUE)
	{
		std::cout << "����͸� ���� : " << GetLastError();
		return 0;
	}

	while (true)
	{
		DWORD result = WaitForSingleObject(hchange, INFINITE);
		if (result == WAIT_OBJECT_0)
		{
			HANDLE tempfile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			_FILETIME tempTime;
			GetFileTime(tempfile, NULL, NULL, &tempTime);
			CloseHandle(tempfile);

			if (tempTime.dwHighDateTime != changeTime.dwHighDateTime || tempTime.dwLowDateTime != changeTime.dwLowDateTime)
			{
				changeTime.dwHighDateTime = tempTime.dwHighDateTime;
				changeTime.dwLowDateTime = tempTime.dwLowDateTime;
				std::cout << "���� ������ �����Ǿ����ϴ�." << std::endl;
				Sleep(10);
				LoadJson(filePath);
			}

			FindNextChangeNotification(hchange);
		}
		else
		{
			std::cout << "���� ����";
			return 0;
		}
	}

	return 0;
}