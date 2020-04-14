#pragma once
#include <il2cpp/il2cpp_context.h>


class InternalClass : public il2cppapi::Class {
public:
	InternalClass(const il2cpp_context& ctx, internal::Il2CppClass *klass);

	std::unordered_map<std::string, const void *> methods;
};

// This is the internals for il2cpp_context, this header is fully safe to modify without breaking api compatability
class il2cpp_context_internal : public il2cpp_context {
public:
	il2cpp_context_internal(HMODULE gameAssemblyModule);
	~il2cpp_context_internal();

	std::unique_ptr<il2cpp_binding> mBindingContext;
	mutable std::unordered_map<std::string, InternalClass> mClasses;
};