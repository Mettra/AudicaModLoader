#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <il2cpp/il2cpp_binding.h>
#include <il2cpp/il2cpp_context.h>
#include <il2cpp/semver.h>

semver MOD_LOADER_VERSION = { 1, 0, 0 };


#define ADD_ORIGINAL(i, name) originalFunctions[i] = GetProcAddress(dll, #name)

#define PROXY(i, name) \
	__declspec(dllexport) INT_PTR __stdcall name() \
	{ \
		return originalFunctions[i](); \
	}

FARPROC originalFunctions[50] = { 0 };

void loadFunctions(HMODULE dll)
{
	ADD_ORIGINAL(0, WinHttpAddRequestHeaders);
	ADD_ORIGINAL(1, WinHttpAutoProxySvcMain);
	ADD_ORIGINAL(2, WinHttpCheckPlatform);
	ADD_ORIGINAL(3, WinHttpCloseHandle);
	ADD_ORIGINAL(4, WinHttpConnect);
	ADD_ORIGINAL(5, WinHttpConnectionDeleteProxyInfo);
	ADD_ORIGINAL(6, WinHttpConnectionFreeNameList);
	ADD_ORIGINAL(7, WinHttpConnectionFreeProxyInfo);
	ADD_ORIGINAL(8, WinHttpConnectionFreeProxyList);
	ADD_ORIGINAL(9, WinHttpConnectionGetNameList);
	ADD_ORIGINAL(10, WinHttpConnectionGetProxyInfo);
	ADD_ORIGINAL(11, WinHttpConnectionGetProxyList);
	ADD_ORIGINAL(12, WinHttpConnectionSetProxyInfo);
	ADD_ORIGINAL(13, WinHttpCrackUrl);
	ADD_ORIGINAL(14, WinHttpCreateProxyResolver);
	ADD_ORIGINAL(15, WinHttpCreateUrl);
	ADD_ORIGINAL(16, WinHttpDetectAutoProxyConfigUrl);
	ADD_ORIGINAL(17, WinHttpFreeProxyResult);
	ADD_ORIGINAL(18, WinHttpGetDefaultProxyConfiguration);
	ADD_ORIGINAL(19, WinHttpGetIEProxyConfigForCurrentUser);
	ADD_ORIGINAL(20, WinHttpGetProxyForUrl);
	ADD_ORIGINAL(21, WinHttpGetProxyForUrlEx);
	ADD_ORIGINAL(22, WinHttpGetProxyResult);
	ADD_ORIGINAL(23, WinHttpGetTunnelSocket);
	ADD_ORIGINAL(24, WinHttpOpen);
	ADD_ORIGINAL(25, WinHttpOpenRequest);
	ADD_ORIGINAL(26, WinHttpProbeConnectivity);
	ADD_ORIGINAL(27, WinHttpQueryAuthSchemes);
	ADD_ORIGINAL(28, WinHttpQueryDataAvailable);
	ADD_ORIGINAL(29, WinHttpQueryHeaders);
	ADD_ORIGINAL(30, WinHttpQueryOption);
	ADD_ORIGINAL(31, WinHttpReadData);
	ADD_ORIGINAL(32, WinHttpReceiveResponse);
	ADD_ORIGINAL(33, WinHttpResetAutoProxy);
	ADD_ORIGINAL(34, WinHttpSaveProxyCredentials);
	ADD_ORIGINAL(35, WinHttpSendRequest);
	ADD_ORIGINAL(36, WinHttpSetCredentials);
	ADD_ORIGINAL(37, WinHttpSetDefaultProxyConfiguration);
	ADD_ORIGINAL(38, WinHttpSetOption);
	ADD_ORIGINAL(39, WinHttpSetStatusCallback);
	ADD_ORIGINAL(40, WinHttpSetTimeouts);
	ADD_ORIGINAL(41, WinHttpTimeFromSystemTime);
	ADD_ORIGINAL(42, WinHttpTimeToSystemTime);
	ADD_ORIGINAL(43, WinHttpWebSocketClose);
	ADD_ORIGINAL(44, WinHttpWebSocketCompleteUpgrade);
	ADD_ORIGINAL(45, WinHttpWebSocketQueryCloseStatus);
	ADD_ORIGINAL(46, WinHttpWebSocketReceive);
	ADD_ORIGINAL(47, WinHttpWebSocketSend);
	ADD_ORIGINAL(48, WinHttpWebSocketShutdown);
	ADD_ORIGINAL(49, WinHttpWriteData);

}

extern "C" {
	PROXY(0, WinHttpAddRequestHeaders);
	PROXY(1, WinHttpAutoProxySvcMain);
	PROXY(2, WinHttpCheckPlatform);
	PROXY(3, WinHttpCloseHandle);
	PROXY(4, WinHttpConnect);
	PROXY(5, WinHttpConnectionDeleteProxyInfo);
	PROXY(6, WinHttpConnectionFreeNameList);
	PROXY(7, WinHttpConnectionFreeProxyInfo);
	PROXY(8, WinHttpConnectionFreeProxyList);
	PROXY(9, WinHttpConnectionGetNameList);
	PROXY(10, WinHttpConnectionGetProxyInfo);
	PROXY(11, WinHttpConnectionGetProxyList);
	PROXY(12, WinHttpConnectionSetProxyInfo);
	PROXY(13, WinHttpCrackUrl);
	PROXY(14, WinHttpCreateProxyResolver);
	PROXY(15, WinHttpCreateUrl);
	PROXY(16, WinHttpDetectAutoProxyConfigUrl);
	PROXY(17, WinHttpFreeProxyResult);
	PROXY(18, WinHttpGetDefaultProxyConfiguration);
	PROXY(19, WinHttpGetIEProxyConfigForCurrentUser);
	PROXY(20, WinHttpGetProxyForUrl);
	PROXY(21, WinHttpGetProxyForUrlEx);
	PROXY(22, WinHttpGetProxyResult);
	PROXY(23, WinHttpGetTunnelSocket);
	PROXY(24, WinHttpOpen);
	PROXY(25, WinHttpOpenRequest);
	PROXY(26, WinHttpProbeConnectivity);
	PROXY(27, WinHttpQueryAuthSchemes);
	PROXY(28, WinHttpQueryDataAvailable);
	PROXY(29, WinHttpQueryHeaders);
	PROXY(30, WinHttpQueryOption);
	PROXY(31, WinHttpReadData);
	PROXY(32, WinHttpReceiveResponse);
	PROXY(33, WinHttpResetAutoProxy);
	PROXY(34, WinHttpSaveProxyCredentials);
	PROXY(35, WinHttpSendRequest);
	PROXY(36, WinHttpSetCredentials);
	PROXY(37, WinHttpSetDefaultProxyConfiguration);
	PROXY(38, WinHttpSetOption);
	PROXY(39, WinHttpSetStatusCallback);
	PROXY(40, WinHttpSetTimeouts);
	PROXY(41, WinHttpTimeFromSystemTime);
	PROXY(42, WinHttpTimeToSystemTime);
	PROXY(43, WinHttpWebSocketClose);
	PROXY(44, WinHttpWebSocketCompleteUpgrade);
	PROXY(45, WinHttpWebSocketQueryCloseStatus);
	PROXY(46, WinHttpWebSocketReceive);
	PROXY(47, WinHttpWebSocketSend);
	PROXY(48, WinHttpWebSocketShutdown);
	PROXY(49, WinHttpWriteData);
}

HMODULE mHinst = 0, mHinstDLL = 0;
bool hooked = false;

// Loads the original DLL from the default system directory
//	Function originally written by Michael Koch
void load_original_dll()
{
	char buffer[MAX_PATH];

	// Get path to system dir and to winhttp.dll
	GetSystemDirectoryA(buffer, MAX_PATH);

	// Append DLL name
	strcat_s(buffer, "\\winhttp.dll");

	// Try to load the system's winhttp.dll, if pointer empty
	if (!mHinstDLL) {
		mHinstDLL = LoadLibraryA(buffer);
	}

	// Debug
	if (!mHinstDLL) {
		OutputDebugStringA("PROXYDLL: Original winhttp.dll not loaded ERROR ****\r\n");
		ExitProcess(0); // Exit the hard way
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
	LOG("--------------------------------  Audica Mod Loader v%d.%d.%d --------------------------------\n", MOD_LOADER_VERSION.major, MOD_LOADER_VERSION.minor, MOD_LOADER_VERSION.patch);

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
			auto func = reinterpret_cast<ModDeclaration(*)(il2cpp_binding &bindingCtx)>(loadCall);
			if (func) {
				auto modDecl = func(GlobalContext->getBinding());

				if (MOD_LOADER_VERSION.major != modDecl.bindingVersion.major) {
					std::string message;
					std::string header;

					if (MOD_LOADER_VERSION.major > modDecl.bindingVersion.major) {
						header = std::string() + "Update " + modDecl.modName;
						message = std::string() + "Mod [" + modDecl.modName + "] depends on an older version of the mod loader. It needs to be updated to work with the new mod loader!";
					}
					else {
						header = std::string() + "Update Mod Loader!";
						message = std::string() + "Mod [" + modDecl.modName + "] requires a newer version of the mod loader in order to function!";
					}

					MessageBoxA(NULL, message.c_str(), header.c_str(), MB_OK | MB_ICONERROR);
					ExitProcess(EXIT_FAILURE);
				}

				LOG("Loaded %s!\n", modDecl.modName);
			}
		}
		catch (...) {
			LOG("%ls: FAILED TO CALL 'registerHooks' FUNCTION!\n", path);
		}
	} while (FindNextFileW(findHandle, &findData) != 0);


	//Finished loaded mods, now install all the hooks
	GlobalContext->getBinding().setupHooks();

	LOG("Loaded all mods!\n");
	LOG("-------------------------------------------------------------------------------------------\n\n");
}


BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	mHinst = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH) {
		load_original_dll();
		loadFunctions(mHinstDLL);
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