#pragma once
#include <il2cpp/il2cpp_binding.h>


// This is the internals for il2cpp_binding, this header is fully safe to modify without breaking api compatability

class il2cpp_binding_internal : public il2cpp_binding {
public:
	il2cpp_binding_internal(const il2cpp_context &ctx);
	~il2cpp_binding_internal();

	void setupHooks();

	const il2cpp_context &mCtx;
	std::vector<HookCall> mHooks;
	std::unordered_map<std::string, size_t> mHooksByMethodName;
	std::unordered_map<uint64_t, size_t> mHooksByInvocation;

	uint64_t mFnId;
	std::vector<void *> mBuffers;
	void *mCurrentPtr;
};