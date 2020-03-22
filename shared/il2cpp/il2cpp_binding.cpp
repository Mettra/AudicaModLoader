#include "il2cpp_binding.h"
#include "il2cpp_context.h"

#ifdef AUDICA_MOD_LOADER
#include <funchook.h>

#define CHECK_FUNCHOOK_ERR(arg, message) \
	if (arg != FUNCHOOK_ERROR_SUCCESS) printf("%s%s: %s", message, "fn", funchook_error_message(handle))
#endif

#define FATAL_ERROR(msg, ...) printf(msg, __VA_ARGS__); system("pause"); exit(1)

il2cpp_binding::il2cpp_binding(const il2cpp_context &ctx) 
	: mCtx(ctx) {
}

void *il2cpp_binding::_getCallList(const std::string &namespaceName, const std::string& className, const std::string& methodName) {
	std::string key = namespaceName + "::" + className + "::" + methodName;
	auto it = mHooksByMethodName.find(key);
	if (it == mHooksByMethodName.end()) {
		return nullptr;
	}

	return mHooks[it->second].node;
}

void il2cpp_binding::_setCallList(const std::string &namespaceName, const std::string& className, const std::string& methodName, void *node, void *invokeFn, void *(*sortFn)(void *), size_t numArgs) {
	std::string key = namespaceName + "::" + className + "::" + methodName;
	auto it = mHooksByMethodName.find(key);
	if (it != mHooksByMethodName.end()) {
		FATAL_ERROR("Hook was already registered! This is bad!\n");
		return;
	}

	auto klass = getIL2CPPContext().getClass(namespaceName, className);
	if (!klass) {
		printf("Failed to find class %s::%s", namespaceName.c_str(), className.c_str());
		return;
	}

	auto method = getIL2CPPContext().getClassMethod(*klass, methodName, (int)numArgs);
	if (!method) {
		printf("Failed to bind to find class method %s::%s::%s with %zd args!\n", namespaceName.c_str(), className.c_str(), methodName.c_str(), numArgs);
		return;
	}

	HookCall call;
	call.node = node;
	call.invokeFn = invokeFn;
	call.originalFn = method->methodPtr;
	call.sortFn = sortFn;
	call.klass = klass;

	mHooks.emplace_back(std::move(call));
	mHooksByMethodName.emplace(std::make_pair(key, mHooks.size() - 1));
	mHooksByInvocation.emplace(std::make_pair(invokeFn, mHooks.size() - 1));
}

const il2cpp_binding::HookCall& il2cpp_binding::getDataForInvocation(void *invokeFn) const {
	auto it = mHooksByInvocation.find(invokeFn);
	if (it != mHooksByInvocation.end()) {
		return mHooks[it->second];
	}

	FATAL_ERROR("We needed data for invocation, but none was present. This is bad!\n");
}

const il2cpp_context& il2cpp_binding::getIL2CPPContext() const {
	return mCtx;
}

#ifdef AUDICA_MOD_LOADER
void il2cpp_binding::setupHooks() {
	for (auto &&hook : mHooks) {
		//Sort all the hooks based on priority
		hook.node = hook.sortFn(hook.node);

		auto handle = funchook_create();
		CHECK_FUNCHOOK_ERR(funchook_prepare(handle, &hook.originalFn, hook.invokeFn), "funchook_prepare returned error: ");
		CHECK_FUNCHOOK_ERR(funchook_install(handle, 0), "funchook_install returned error:");
	}
}
#endif