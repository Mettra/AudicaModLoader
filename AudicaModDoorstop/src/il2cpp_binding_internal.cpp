#include "il2cpp_binding_internal.h"

#include <il2cpp/il2cpp_context.h>

const size_t page_size = 0x1000; /* 4K */

#define FATAL_ERROR(msg, ...) printf(msg, __VA_ARGS__); system("pause"); exit(1)

thread_local uint64_t functionId;
__declspec(noinline) uint64_t *getThreadLocal() { return &functionId; }

#include <funchook.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef max
#undef min

#define CHECK_FUNCHOOK_ERR(arg, message) \
	if (arg != FUNCHOOK_ERROR_SUCCESS) printf("%s%s: %s", message, "fn", funchook_error_message(handle))

using u8 = unsigned char;

/******************************
 * This is how we get a unique funciton for every hook, we generate the assembly to set `functionId`
 * This will be placed in memory, then markes as executable, and used as the jump point for our hook
 *******************************/
u8* createUniqueFn(u8 *start, uint64_t value, uint64_t functionToJumpTo) {
	//push rcx, rdx, r8, r9 (Volatile registers coming from GameAssembly)
	(*start++) = 0x51;
	(*start++) = 0x52;
	(*start++) = 0x41;
	(*start++) = 0x50;
	(*start++) = 0x41;
	(*start++) = 0x51;

	//Call getThreadLocal
	(*start++) = 0xE8;
	int32_t relativeCall = static_cast<int32_t>((u8*)getThreadLocal - start - 0x4);
	*reinterpret_cast<int32_t *>(start) = relativeCall;
	start += sizeof(int32_t);

	//Load value into ecx
	(*start++) = 0x48;
	(*start++) = 0xB9;
	*reinterpret_cast<uint64_t *>(start) = value;
	start += sizeof(uint64_t);

	//Mov [eax], ecx, writing `value` to `functionId` (This will be used to get hook data)
	(*start++) = 0x48;
	(*start++) = 0x89;
	(*start++) = 0x08;

	//Load jmp address in eax
	(*start++) = 0x48;
	(*start++) = 0xB8;
	{
		*reinterpret_cast<uint64_t *>(start) = functionToJumpTo;
		start += sizeof(uint64_t);
	}

	//pop r9, r8, rdx, rcx (Volatile registers coming from GameAssembly)
	(*start++) = 0x41;
	(*start++) = 0x59;

	(*start++) = 0x41;
	(*start++) = 0x58;

	(*start++) = 0x5a;
	(*start++) = 0x59;

	//Jump eax (To functionToJumpTo)
	(*start++) = 0xFF;
	(*start++) = 0xE0;

	void *p = start;
	size_t sz = page_size;
	start = (u8*)std::align(8, sizeof(u8*), p, sz);

	return start;
}


//Ensure that the memory we write to is available, and near enough to short jump
void ensurePage(il2cpp_binding_internal &bnd) {
	if (!bnd.mBuffers.empty() && std::abs((u8*)bnd.mBuffers.back() - (u8*)bnd.mCurrentPtr) + 50 < page_size) {
		return;
	}

	void *page = nullptr;
	u8 *tryAddr = (u8*)ensurePage;
	void *p = tryAddr;
	size_t sz = page_size * 4;
	tryAddr = (u8*)std::align(page_size, sizeof(u8*), p, sz);

	int incr = page_size;
	while (page == nullptr) {
		tryAddr += incr;

		if (std::abs((u8*)ensurePage - (u8*)tryAddr) > std::numeric_limits<uint32_t>::max()) {
			if (incr < 0) {
				FATAL_ERROR("There was no memory close enough to generate a page! This might be able to be fixed by restarting, but it might be worse.");
			}

			incr *= -1;
			tryAddr = (u8*)ensurePage;
		}
		else {
			page = VirtualAlloc(tryAddr, page_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		}
	}

	bnd.mCurrentPtr = page;
	bnd.mBuffers.emplace_back(page);
}


void _addHookCall(il2cpp_binding_internal &bnd, const char *namespaceName, const char *className, const char *methodName, size_t numArgs, il2cpp_binding::HookCall &&call) {
	std::string key = std::string(namespaceName) + "::" + className + "::" + methodName;

	auto it = bnd.mHooksByMethodName.find(key);
	if (it == bnd.mHooksByMethodName.end()) {
		auto klass = bnd.mCtx.getClass(namespaceName, className);
		if (!klass) {
			printf("Failed to find class %s::%s", namespaceName, className);
			return;
		}

		auto method = bnd.mCtx.getClassMethod(*klass, methodName, (int)numArgs);
		if (!method) {
			printf("Failed to bind to find class method %s::%s::%s with %zd args!\n", namespaceName, className, methodName, numArgs);
			return;
		}

		ensurePage(bnd);

		call.originalFn = method->methodPtr;
		call.klass = klass;
		call.uniqueFn = bnd.mCurrentPtr;

		auto id = bnd.mFnId++;
		call.id = id;
		bnd.mCurrentPtr = createUniqueFn((u8*)bnd.mCurrentPtr, id, *(uint64_t*)&call.invokeFn);

		bnd.mHooks.emplace_back(std::move(call));
		bnd.mHooksByInvocation.emplace(std::make_pair(id, bnd.mHooks.size() - 1));
		bnd.mHooksByMethodName.emplace(std::make_pair(key, bnd.mHooks.size() - 1));
		return;
	}

	auto &&hook = bnd.mHooks[it->second];

	//We alwyas want to use the latest binding code, so update it if a new one comes along
	if (hook.hookVersion < call.hookVersion) {
		hook.invokeFn = call.invokeFn;
		hook.invokeNodeFunction = call.invokeNodeFunction;
		hook.invokeOriginalFunction = call.invokeOriginalFunction;

		//Remake unique fn with new invoke function
		createUniqueFn((u8*)hook.uniqueFn, hook.id, *(uint64_t*)&hook.invokeFn);
	}

	auto currentNode = hook.node;
	while (currentNode->next != nullptr) {
		currentNode = currentNode->next;
	}
	currentNode->next = call.node;
}

const il2cpp_binding::HookCall& _getDataForInvocation(const il2cpp_binding_internal &bnd) {
	auto it = bnd.mHooksByInvocation.find(functionId);
	if (it != bnd.mHooksByInvocation.end()) {
		return bnd.mHooks[it->second];
	}

	FATAL_ERROR("We needed data for invocation, but none was present. This is bad!\n");
}

const il2cpp_context& _getIL2CPPContext(const il2cpp_binding_internal &bnd) {
	return bnd.mCtx;
}

void _invokeFunctionChain(MethodInvocationContext &ctx, std::optional<void *> thsPtr) {
	auto &&data = _getDataForInvocation(static_cast<il2cpp_binding_internal&>(ctx.getGlobalContext().getBinding()));

	std::optional<ThisPtr> ths;
	if (thsPtr.has_value()) {
		ths = ThisPtr(internal::Il2CppObject{ *thsPtr }, data.klass);
	}

	MethodHookNode *current = data.node;
	while (current != nullptr) {
		if (current->invokeTime == InvokeTime::Before) {
			data.invokeNodeFunction(ctx, ths, current->data);
			if (ctx.didStopExecution()) {
				return;
			}
		}

		current = current->next;
	}

	data.invokeOriginalFunction(ctx, thsPtr.has_value() ? *thsPtr : nullptr, data.originalFn);

	current = data.node;
	while (current != nullptr) {
		if (current->invokeTime == InvokeTime::After) {
			data.invokeNodeFunction(ctx, ths, current->data);
			if (ctx.didStopExecution()) {
				return;
			}
		}

		current = current->next;
	}
}

il2cpp_binding_internal::il2cpp_binding_internal(const il2cpp_context &ctx)
	: mCtx(ctx)
	, mFnId(0) {
	GetIL2CPPContext = reinterpret_cast<decltype(GetIL2CPPContext)>(_getIL2CPPContext);
	InvokeFunctionChain = _invokeFunctionChain;
	AddHookCall = reinterpret_cast<decltype(AddHookCall)>(_addHookCall);
}

il2cpp_binding_internal::~il2cpp_binding_internal() {

}

MethodHookNode *sortNodes(MethodHookNode *start) {
	std::vector<MethodHookNode *> allNodes;
	MethodHookNode *current = start;
	while (current != nullptr) {
		allNodes.emplace_back(current);
		current = current->next;
	}

	std::sort(allNodes.begin(), allNodes.end(), [](MethodHookNode *lhs, MethodHookNode *rhs) {
		return lhs->priority < rhs->priority;
	});

	MethodHookNode *prev = allNodes[0];
	for (int i = 1; i < allNodes.size(); ++i) {
		prev->next = allNodes[i];
		prev = allNodes[i];
	}

	prev->next = nullptr;

	return allNodes[0];
}


void il2cpp_binding_internal::setupHooks() {
	for (auto &&hook : mHooks) {
		//Sort all the hooks based on priority
		hook.node = sortNodes(hook.node);

		auto handle = funchook_create();
		CHECK_FUNCHOOK_ERR(funchook_prepare(handle, &hook.originalFn, hook.uniqueFn), "funchook_prepare returned error: ");
		CHECK_FUNCHOOK_ERR(funchook_install(handle, 0), "funchook_install returned error:");
	}

	//Protect executable space
	for (auto &&page : mBuffers) {
		DWORD old;
		VirtualProtect(page, page_size, PAGE_EXECUTE, &old);
	}
}