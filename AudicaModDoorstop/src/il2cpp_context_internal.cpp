#include "il2cpp_context_internal.h"
#include "il2cpp_binding_internal.h"

#include <il2cpp/il2cpp_types.h>

//Il2CPP functions for internal data
static il2cpp_context_internal *GlobalContext;

namespace api {
	static internal::FieldInfo* (*il2cpp_class_get_field_from_name)(internal::Il2CppClass* klass, const char* name);
	static void(*il2cpp_field_get_value)(internal::Il2CppObject obj, const internal::FieldInfo* field, void* value);
	static void(*il2cpp_field_set_value)(internal::Il2CppObject obj, const internal::FieldInfo* field, const void *value);
	static void(*il2cpp_field_static_get_value)(const internal::FieldInfo * field, void *value);
	static void(*il2cpp_field_static_set_value)(const internal::FieldInfo * field, const void *value);
	static const internal::MethodInfo* (*il2cpp_class_get_method_from_name)(internal::Il2CppClass* klass, const char* name, int argsCount);
	static internal::Il2CppClass* (*il2cpp_class_from_name)(const internal::Il2CppImage* image, const char* namespaze, const char* name);
	static const internal::Il2CppAssembly** (*il2cpp_domain_get_assemblies)(const internal::Il2CppDomain* domain, size_t* size);
	static internal::Il2CppDomain* (*il2cpp_domain_get)();
	static const internal::Il2CppImage* (*il2cpp_assembly_get_image)(const internal::Il2CppAssembly* assembly);
	static const internal::Il2CppType* (*il2cpp_field_get_type)(const internal::FieldInfo * field);
	static internal::Il2CppClass* (*il2cpp_class_from_type)(const internal::Il2CppType * type);
	static const char* (*il2cpp_type_get_name)(const internal::Il2CppType * type);
	static const internal::PropertyInfo* (*il2cpp_class_get_property_from_name)(internal::Il2CppClass * klass, const char *name);
	static const internal::MethodInfo* (*il2cpp_property_get_get_method)(const internal::PropertyInfo * prop);
	static const internal::MethodInfo* (*il2cpp_property_get_set_method)(const internal::PropertyInfo * prop);
	static const int32_t(*il2cpp_string_length)(const internal::Il2CppString str);
	static const internal::Il2CppChar* (*il2cpp_string_chars)(const internal::Il2CppString str);
	static internal::Il2CppString(*il2cpp_string_new_len)(const char* str, uint32_t length);
	static uint32_t(*il2cpp_array_length)(internal::Il2CppObject arr);
	static uint32_t(*il2cpp_array_get_byte_length)(internal::Il2CppObject arr);
};

///////////////////////////////

const void *_getClassMethod(InternalClass *klass, const char *name, uint32_t numArgs) {
	auto it = klass->methods.find(name);
	if (it != klass->methods.end()) {
		return it->second;
	}

	auto internalMethod = GlobalContext->getClassMethod(*klass, name, numArgs);
	if (!internalMethod) {
		return nullptr;
	}

	klass->methods.emplace(std::make_pair(name, internalMethod->methodPtr));
	return internalMethod->methodPtr;
}

InternalClass::InternalClass(const il2cpp_context & ctx, internal::Il2CppClass * klass) 
	: il2cppapi::Class(ctx, klass) {
	mGetMethod = (const void *(*)(const il2cppapi::Class *, const char *, uint32_t))_getClassMethod;
}

///////////////////////////////

il2cpp_binding &_getBinding() {
	return *GlobalContext->mBindingContext;
}

static il2cppapi::Class* _getClass(const char *namespaceName, const char *className) {
	std::string key = std::string(namespaceName) + "." + className;
	auto it = GlobalContext->mClasses.find(key);
	if (it != GlobalContext->mClasses.end()) {
		return &it->second;
	}

	auto dom = api::il2cpp_domain_get();
	if (!dom) {
		printf("ERROR: getClass: Could not get domain!\n");
		return nullptr;
	}
	size_t assemb_count;
	const internal::Il2CppAssembly** allAssemb = api::il2cpp_domain_get_assemblies(dom, &assemb_count);

	for (int i = 0; i < assemb_count; i++) {
		auto assemb = allAssemb[i];
		auto img = api::il2cpp_assembly_get_image(assemb);
		if (!img) {
			printf("ERROR: Assembly has a null image!\n");
			continue;
		}

		auto klass = api::il2cpp_class_from_name(img, namespaceName, className);
		if (klass) {
			InternalClass newClass(*GlobalContext, klass);
			auto p = GlobalContext->mClasses.emplace(std::make_pair(key, std::move(newClass)));
			return &p.first->second;
		}
	}
	printf("ERROR: getClass: Could not find class with namespace %s and name: %s\n", namespaceName, className);
	return nullptr;
}


il2cppapi::Class* _getClassFromField(const internal::FieldInfo* field) {
	auto type = api::il2cpp_field_get_type(field);
	if (!type) {
		printf("ERROR: getClassFromField: Field has no type!\n");
		return nullptr;
	}

	auto name = api::il2cpp_type_get_name(type);
	auto it = GlobalContext->mClasses.find(name);
	if (it != GlobalContext->mClasses.end()) {
		return &it->second;
	}

	auto klass = api::il2cpp_class_from_type(type);
	if (!klass) {
		printf("ERROR: getClassFromField: Field has no class type!\n");
		return nullptr;
	}

	InternalClass newClass(*GlobalContext, klass);
	auto p = GlobalContext->mClasses.emplace(std::make_pair(name, std::move(newClass)));
	return &p.first->second;
}


il2cpp_context_internal::~il2cpp_context_internal() = default;

il2cpp_context_internal::il2cpp_context_internal(HMODULE gameAssemblyModule) {
	GlobalContext = this;
	mBindingContext = std::make_unique<il2cpp_binding_internal>(*this);

	// Load api functions
	*(void**)(&api::il2cpp_class_get_field_from_name) = GetProcAddress(gameAssemblyModule, "il2cpp_class_get_field_from_name");
	*(void**)(&api::il2cpp_field_get_value) = GetProcAddress(gameAssemblyModule, "il2cpp_field_get_value");
	*(void**)(&api::il2cpp_field_set_value) = GetProcAddress(gameAssemblyModule, "il2cpp_field_set_value");
	*(void**)(&api::il2cpp_field_static_get_value) = GetProcAddress(gameAssemblyModule, "il2cpp_field_static_get_value");
	*(void**)(&api::il2cpp_field_static_set_value) = GetProcAddress(gameAssemblyModule, "il2cpp_field_static_set_value");
	*(void**)(&api::il2cpp_class_get_method_from_name) = GetProcAddress(gameAssemblyModule, "il2cpp_class_get_method_from_name");
	*(void**)(&api::il2cpp_class_from_name) = GetProcAddress(gameAssemblyModule, "il2cpp_class_from_name");
	*(void**)(&api::il2cpp_domain_get_assemblies) = GetProcAddress(gameAssemblyModule, "il2cpp_domain_get_assemblies");
	*(void**)(&api::il2cpp_domain_get) = GetProcAddress(gameAssemblyModule, "il2cpp_domain_get");
	*(void**)(&api::il2cpp_assembly_get_image) = GetProcAddress(gameAssemblyModule, "il2cpp_assembly_get_image");
	*(void**)(&api::il2cpp_field_get_type) = GetProcAddress(gameAssemblyModule, "il2cpp_field_get_type");
	*(void**)(&api::il2cpp_class_from_type) = GetProcAddress(gameAssemblyModule, "il2cpp_class_from_type");
	*(void**)(&api::il2cpp_type_get_name) = GetProcAddress(gameAssemblyModule, "il2cpp_type_get_name");
	*(void**)(&api::il2cpp_class_get_property_from_name) = GetProcAddress(gameAssemblyModule, "il2cpp_class_get_property_from_name");
	*(void**)(&api::il2cpp_property_get_get_method) = GetProcAddress(gameAssemblyModule, "il2cpp_property_get_get_method");
	*(void**)(&api::il2cpp_property_get_set_method) = GetProcAddress(gameAssemblyModule, "il2cpp_property_get_set_method");
	*(void**)(&api::il2cpp_string_chars) = GetProcAddress(gameAssemblyModule, "il2cpp_string_chars");
	*(void**)(&api::il2cpp_string_length) = GetProcAddress(gameAssemblyModule, "il2cpp_string_length");
	*(void**)(&api::il2cpp_string_new_len) = GetProcAddress(gameAssemblyModule, "il2cpp_string_new_len");
	*(void**)(&api::il2cpp_array_length) = GetProcAddress(gameAssemblyModule, "il2cpp_array_length");
	*(void**)(&api::il2cpp_array_get_byte_length) = GetProcAddress(gameAssemblyModule, "il2cpp_array_get_byte_length");
	//


	//Set il2cpp_context functions
	mGetBinding = _getBinding;
	mGetClass = _getClass;
	mGetClassFromField = _getClassFromField;

	//Set all the member functions
	this->il2cpp_class_get_field_from_name = api::il2cpp_class_get_field_from_name;
	this->il2cpp_field_get_value = api::il2cpp_field_get_value;
	this->il2cpp_field_set_value = api::il2cpp_field_set_value;
	this->il2cpp_field_static_get_value = api::il2cpp_field_static_get_value;
	this->il2cpp_field_static_set_value = api::il2cpp_field_static_set_value;
	this->il2cpp_class_get_method_from_name = api::il2cpp_class_get_method_from_name;
	this->il2cpp_class_from_name = api::il2cpp_class_from_name;
	this->il2cpp_domain_get_assemblies = api::il2cpp_domain_get_assemblies;
	this->il2cpp_domain_get = api::il2cpp_domain_get;
	this->il2cpp_assembly_get_image = api::il2cpp_assembly_get_image;
	this->il2cpp_field_get_type = api::il2cpp_field_get_type;
	this->il2cpp_class_from_type = api::il2cpp_class_from_type;
	this->il2cpp_type_get_name = api::il2cpp_type_get_name;
	this->il2cpp_class_get_property_from_name = api::il2cpp_class_get_property_from_name;
	this->il2cpp_property_get_get_method = api::il2cpp_property_get_get_method;
	this->il2cpp_property_get_set_method = api::il2cpp_property_get_set_method;
	this->il2cpp_string_chars = api::il2cpp_string_chars;
	this->il2cpp_string_length = api::il2cpp_string_length;
	this->il2cpp_string_new_len = api::il2cpp_string_new_len;
	this->il2cpp_array_length = api::il2cpp_array_length;
	this->il2cpp_array_get_byte_length = api::il2cpp_array_get_byte_length;
}