#pragma once
#include <Windows.h>
#include <Winuser.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <time.h>
#include <vector>
#include "defItems.h"

std::map<DWORD, HWND> pMap;
std::map<DWORD, HWND>::iterator processMapIt;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char clsName[80];
	char title[80];
	GetClassName(hwnd, (LPWSTR)clsName, sizeof(clsName));
	GetWindowText(hwnd, (LPWSTR)title, sizeof(title));

	if (strcmp(title, __FLYFF__) == 0 && strcmp(clsName, __CLASS__) == 0)
	{
		DWORD pid = 0;
		GetWindowThreadProcessId(hwnd, &pid);
		//std::cout << "ProcessIDentifier: " << pid << ", Hwnd: " << hwnd << std::endl;
		if (pid != 0) {
			pMap.insert(std::make_pair(pid, hwnd));
		}
	}
	return TRUE;
}

void EnableDebugPriv()
{
	// grant debug priv to read/write mem in another proc
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tkp;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);
	CloseHandle(hToken);
}

// Bypassing anti hack gameguard
DWORD originalPostMessage = (DWORD)GetProcAddress(LoadLibrary(TEXT("User32.dll")), "PostMessageA") + 5;
__declspec(naked) BOOL WINAPI hPostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	__asm
	{
		mov edi, edi
		push ebp
		mov ebp, esp
		jmp dword ptr ds : [originalPostMessage]
	}
}

void PressKeyByPassed(const HWND hGameWindow, const DWORD & key, const int duration = 0)
{
	srand((unsigned int)time(NULL)); // random seeds for some random shitty
	Sleep(rand() % 10 + 10); // random 10ms -20ms, any delay will do
	hPostMessage(hGameWindow, WM_KEYDOWN, key, NULL);
	Sleep(1000 * duration);
	Sleep(rand() % 10 + 10); // random 10ms -20ms, as above 
	hPostMessage(hGameWindow, WM_KEYUP, key, NULL);
}



void GenerateRandomMovement(HWND hWnd)
{
	std::vector<DWORD> directions{ vk_a, vk_d, vk_w, vk_s };
	int indexDirection = rand() % directions.size(); // pick a random index
	std::cout << "indexDirection: " << indexDirection << std::endl;
	int direction = directions[indexDirection]; // a random value taken from that list

	std::vector<int> durations{ 1, 2}; // 1,2 or x seconds
	int indexDuration = rand() % durations.size(); // pick a random index
	std::cout << "indexDuration: " << indexDuration << std::endl;
	int duration = durations[indexDuration]; // a random value taken from that list


	std::vector<int> b{ 0, 1 }; // yes=1, no =0
	int indexb = rand() % b.size(); // pick a random index
	std::cout << "indexb: " << indexb << std::endl;
	bool bias = b[indexb]; // a random value taken from that list

	if (bias)
	{
		// Generate here some random movement
		PressKeyByPassed(hWnd, direction, duration);
	}

}


int main()
{
	// process id
	DWORD pid = 0;

	// window handle
	HWND hWnd = 0;

	// delay to kill a single target
	int nKillDelay = 0;

	// timer to kill a single target
	int nKillTimer = 0;

	// enable debug privilege
	EnableDebugPriv();

	// find all currently running FLYFF windows
	std::cout << "Enumerating all windows named: " << __FLYFF__ << std::endl;
	if (!EnumWindows(EnumWindowsProc, NULL))
	{
		std::cout <<  "[-] Failed to EnumWindows!" << std::endl;
		system("pause");
		return -1;
	}

	processMapIt = pMap.begin();

	if (pMap.size() == 0)
	{
		std::cout << "[-] Failed to find any process!" << std::endl;
		system("pause");
		return -1;
	}
	else if (pMap.size() == 1)
	{
		pid = processMapIt->first;
		hWnd = processMapIt->second;
		std::cout << "[+] Process ID: " << std::dec << pid << std::endl;
		std::cout << "[+] Hwnd: " << std::hex << hWnd << std::endl;
	}
	else if (pMap.size() > 1) {
		std::cout << "Write down the target process ID from the list below and hit 'Enter':" << std::endl << std::endl;
		std::cout << "--------------------" << std::endl;
		while (processMapIt != pMap.end())
		{
			std::cout << "Process ID: " << processMapIt->first << std::endl;
			processMapIt++;
		}
		std::cout << "--------------------" << std::endl;

		// Get the written pid
		std::cin >> pid;

		processMapIt = pMap.begin();

		// Get the HWND corresponding to the entered PID
		while (processMapIt != pMap.end())
		{
			if (processMapIt->first == pid)
			{
				std::cout << "You selected -> Process: " << processMapIt->first << ", HWND: " << processMapIt->second << std::endl;
				hWnd = processMapIt->second;
				break;
			}
			processMapIt++;
		}
	}// end else if

	// ------------------------------------- Delay between each target selection ----------------------------------------------
	std::cout << "Write down the delay between each target selection (= 'kill' time + 'run to target' time)" << std::endl;
	std::cin >> nKillDelay;
	if (nKillDelay > 0)
	{
		std::cout << "You entered [" << std::dec << nKillDelay << "] seconds betwwen each selection." << std::endl;
	}
	else {
		std::cout << "[-] Please enter a bigger value (noob)" << std::endl;
		system("pause");
		return -1;
	}
	//-------------------------------------------------------------------------------------------------------------------------
	

	std::cout << ">>>>>>> Put your kill skill in F1 <<<<<<<" << std::endl;

	std::cout << ">>>>>>> Press F5 to start/pause the bot <<<<<<<" << std::endl;

	// loop starts here
	bool bRun = false;
	while (true)
	{
		// sleep to not kill your CPU
		Sleep(10);

		// todo create a thread for this toggle
		if (GetAsyncKeyState(VK_F5))
		{
			// Pressing F5 will toggle between running/not running
			bRun = !bRun;
			if (bRun)
			{
				std::cout << ">>>>>>> Bot resumed <<<<<<<" << std::endl;
			}
			else {
				std::cout << ">>>>>>> Bot paused <<<<<<<" << std::endl;
			}
		}


		if (bRun)
		{
			// generate random movement
			//GenerateRandomMovement(hWnd);

			// press Tab many times to have more chance to select a target
			PressKeyByPassed(hWnd, VK_TAB);
			PressKeyByPassed(hWnd, VK_RIGHT);

			Sleep(10);
			PressKeyByPassed(hWnd, VK_TAB);
			PressKeyByPassed(hWnd, VK_RIGHT);

			Sleep(10);
			PressKeyByPassed(hWnd, VK_TAB);
			PressKeyByPassed(hWnd, VK_RIGHT);

			std::cout << "Tab pressed.." << std::endl;

			// restart the timer for killing
			nKillTimer = clock();

			// Loop to kill the target within the kill time delay
			while (clock() - nKillTimer < 1000 * nKillDelay)
			{
				PressKeyByPassed(hWnd, VK_F1);
				PressKeyByPassed(hWnd, VK_RIGHT);
				Sleep(10);

			}


		}

	}



	system("pause");

	return 0;
}