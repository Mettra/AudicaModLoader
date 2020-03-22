#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../shared/il2cpp/il2cpp_binding.h"
#include "../shared/il2cpp/il2cpp_context.h"

HMODULE mHinst = 0, mHinstDLL = 0;
bool hooked = false;
HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule);

	return hModule;
}

// Loads the original DLL from the default system directory
//	Function originally written by Michael Koch
void load_original_dll()
{
	char buffer[MAX_PATH];

	// Get path to system dir and to IPHLPAPI.dll
	GetSystemDirectoryA(buffer, MAX_PATH);

	// Append DLL name
	strcat_s(buffer, "\\IPHLPAPI.dll");

	// Try to load the system's IPHLPAPI.dll, if pointer empty
	if (!mHinstDLL) {
		mHinstDLL = LoadLibraryA(buffer);
	}

	// Debug
	if (!mHinstDLL) {
		OutputDebugStringA("PROXYDLL: Original IPHLPAPI.dll not loaded ERROR ****\r\n");
		ExitProcess(0); // Exit the hard way
	}
}

typedef DWORD(WINAPI *RealGetNetworkParamsFn)(void *pFixedInfo, PULONG pOutBufLen);
RealGetNetworkParamsFn GetNetworkParamsPtr;

#ifdef _WIN64
#pragma comment(linker, "/export:GetNetworkParams=FakeGetNetworkParams")
#else
#pragma comment(linker, "/export:GetNetworkParams=_FakeGetNetworkParams@8")
#endif
extern "C" {
	__declspec(dllexport)
	DWORD
		WINAPI
		FakeGetNetworkParams(
			void * pFixedInfo,
			PULONG pOutBufLen
		) {
		return GetNetworkParamsPtr(pFixedInfo, pOutBufLen);
	}
}

#define LOG(msg, ...) printf(msg, __VA_ARGS__);
#define ASSERT(test, message)                    \
if(!(test))                                  \
{                                            \
	MessageBox(NULL, message, L"Modloader: Fatal", MB_OK | MB_ICONERROR); \
	ExitProcess(EXIT_FAILURE); \
}

std::unique_ptr<il2cpp_context> GlobalContext;

void LoadMods() {
	if (!CreateDirectory(L"Mods/", NULL) && ERROR_ALREADY_EXISTS != GetLastError())
	{
		LOG("FAILED TO CREATE MODS DIRECTORY!\n");
		MessageBoxW(NULL, L"Could not create Mods/ directory!", L"Could not create Mods directory!'",
			MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST | MB_SETFOREGROUND);
		ExitProcess(GetLastError());
	}

	WIN32_FIND_DATAW findDataAssemb;
	LOG("Attempting to find GameAssembly.dll\n");
	HANDLE findHandleAssemb = FindFirstFileW(L"GameAssembly.dll", &findDataAssemb);
	if (findHandleAssemb == INVALID_HANDLE_VALUE)
	{
		LOG("FAILED TO FIND GameAssembly.dll!\n");
		MessageBoxW(NULL, L"Could not locate game being injected!", L"Could not find GameAssembly.dll'",
			MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST | MB_SETFOREGROUND);
		ExitProcess(GetLastError());
		return;
	}
	auto gameassemb = LoadLibraryA("GameAssembly.dll");
	ASSERT(gameassemb, L"GameAssembly.dll failed to load!");

	WIN32_FIND_DATAW findData;
	LOG("Attempting to find all files that match: 'Mods/*.dll'\n");
	HANDLE findHandle = FindFirstFileW(L"Mods/*.dll", &findData);

	if (findHandle == INVALID_HANDLE_VALUE)
	{
		LOG("FAILED TO FIND ANY MODS TO LOAD!\n");
		//free_logger();
		return;
	}

	GlobalContext = std::make_unique<il2cpp_context>(gameassemb);

	do
	{
		// Maybe we can just call LoadLibrary on this file, and GetProcAddress of load and call that?
			// 240 = max path length
		wchar_t path[240] = { 0 };
		wcscpy_s(path, L"Mods/");
		wcscat_s(path, findData.cFileName);

		auto lib = LoadLibrary(path);
		if (!lib) {
			LOG("%ls: Failed to find library!\n", path);
			continue;
		}
		auto loadCall = GetProcAddress(lib, "registerHooks");
		if (!loadCall) {
			LOG("%ls: Failed to find load call!\n", path);
			continue;
		}

		try {
			auto func = reinterpret_cast<void(*)(il2cpp_binding &bindingCtx)>(loadCall);
			if (func) {
				func(GlobalContext->getBinding());
			}
		}
		catch (...) {
			LOG("%ls: FAILED TO CALL 'registerHooks' FUNCTION!\n", path);
		}

		LOG("%ls: Loaded!\n", path);
	} while (FindNextFileW(findHandle, &findData) != 0);


	//Finished loaded mods, now install all the hooks
	GlobalContext->getBinding().setupHooks();

	LOG("Loaded all mods!\n");
}


BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	mHinst = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH) {
		load_original_dll();
		GetNetworkParamsPtr = (RealGetNetworkParamsFn)GetProcAddress(mHinstDLL, "GetNetworkParams");
	}
	else if (fdwReason == DLL_PROCESS_DETACH) {
		FreeLibrary(mHinstDLL);
	}

	if (hooked) {
		return (TRUE);
	}

	if (fdwReason == DLL_THREAD_ATTACH) {
		hooked = true;
		
		//Use existing console, or create one if needed
		if (!AttachConsole(-1)) {
			AllocConsole();
			ShowWindow(GetConsoleWindow(), SW_SHOW);
		}

		//Redirect output to console
		FILE* fp;
		freopen_s(&fp, "CONOIN$", "r", stdin);
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
		
		LoadMods();
	}

	return (TRUE);
}