#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

/// <summary>
/// JSON을 파싱해서 단순히 텍스트로 출력하는 함수
/// </summary>
/// <param name="filePath">json 파일 경로</param>
void LoadJson(std::wstring& filePath)
{
	// 테스트용 주석
	const wchar_t* path = filePath.c_str();
	std::wifstream inputStream;
	inputStream.open(path);

	if (!inputStream.is_open())
	{
		if (inputStream.fail())
		{
			char buffer[256] = { 0 };
			strerror_s(buffer, errno);
			std::cout << "파일 오픈 실패\n" << buffer << std::endl;
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

	// ini 파일로 부터 경로를 가져온다.
	wchar_t path[256] = { 0 };
	DWORD size = GetPrivateProfileString(L"JSON", L"directory", L"", path, 256, L"./filePath.ini");
	if (size == 0)
	{
		std::cout << "경로를 가져오지 못했습니다.";
		return 0;
	}

	std::wcout << L"경로는 : ";
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
		std::cout << "파일이 없습니다.";
		return 0;
	}

	FindClose(hfind);

	std::cout << fileNames.size() << "개의 파일을 발견했습니다." << std::endl;
	for (int i = 0; i < fileNames.size(); ++i)
	{
		std::wcout << i + 1 << " - " << fileNames[i] << std::endl;
	}

	std::cout << "로드할 파일 인덱스를 입력하세요" << std::endl;

	int index = -1;
	std::cin >> index;
	index -= 1;

	if (index > fileNames.size() || index < 0)
	{
		std::cout << "잘못된 인덱스 입니다.";
		return 0;
	}

	std::wstring filePath = path;
	filePath += L"\\";
	filePath += fileNames[index];

	LoadJson(filePath);

	HANDLE hfile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		std::cout << "파일 정보 가져오기 위한 오픈 실패 : " << GetLastError();
		return 0;
	}
	_FILETIME changeTime;
	GetFileTime(hfile, NULL, NULL, &changeTime);
	CloseHandle(hfile);

	HANDLE hchange = FindFirstChangeNotificationW(path, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

	if (hchange == INVALID_HANDLE_VALUE)
	{
		std::cout << "모니터링 실패 : " << GetLastError();
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
				std::cout << "파일 변경이 감지되었습니다." << std::endl;
				Sleep(10);
				LoadJson(filePath);
			}

			FindNextChangeNotification(hchange);
		}
		else
		{
			std::cout << "뭔가 오류";
			return 0;
		}
	}

	return 0;
}