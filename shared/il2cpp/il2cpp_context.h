#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>
#include <string>

#include "il2cpp_types.h"

class il2cpp_binding;

//Main il2cpp class, holds all functions to interact with the C il2cpp api
//Also owns any auxilary il2cpp_ classes, like binding
class il2cpp_context {
public:
	il2cpp_context(HMODULE gameAssemblyModule);
	~il2cpp_context();
	il2cpp_binding &getBinding() const;

	il2cppapi::Class* getClass(const std::string &namespaceName, const std::string &className) const;
	il2cppapi::Class* getClassFromField(const internal::FieldInfo* field) const;

	const internal::MethodInfo *getClassMethod(internal::Il2CppClass* klass, const std::string &methodName, int argsCount) const;
	const internal::FieldInfo *getClassFieldInfo(internal::Il2CppClass* klass, const std::string &fieldName, bool error = true) const;
	const internal::PropertyInfo *getClassPropertyInfo(internal::Il2CppClass* klass, const std::string &propName, bool error = true) const;

	void getValueFromField(internal::Il2CppObject obj, const internal::FieldInfo* field, void *value) const;
	void setValueFromField(internal::Il2CppObject obj, const internal::FieldInfo* field, const void *value) const;

	const internal::MethodInfo *getPropertyGetter(const internal::PropertyInfo *propertyInfo) const;
	const internal::MethodInfo *getPropertySetter(const internal::PropertyInfo *propertyInfo) const;

	void getValueFromStaticField(const internal::FieldInfo* field, void *value) const;
	void setValueFromStaticField(const internal::FieldInfo* field, const void *value) const;

private:
	std::unique_ptr<il2cpp_binding> mBindingContext;
	mutable std::unordered_map<std::string, il2cppapi::Class> mClasses;

	//Il2CPP functions for internal data
	internal::FieldInfo* (*il2cpp_class_get_field_from_name)(internal::Il2CppClass* klass, const char* name);
	void(*il2cpp_field_get_value)(internal::Il2CppObject obj, const internal::FieldInfo* field, void* value);
	void(*il2cpp_field_set_value)(internal::Il2CppObject obj, const internal::FieldInfo* field, const void *value);
	void(*il2cpp_field_static_get_value)(const internal::FieldInfo * field, void *value);
	void(*il2cpp_field_static_set_value)(const internal::FieldInfo * field, const void *value);
	const internal::MethodInfo* (*il2cpp_class_get_method_from_name)(internal::Il2CppClass* klass, const char* name, int argsCount);
	internal::Il2CppClass* (*il2cpp_class_from_name)(const internal::Il2CppImage* image, const char* namespaze, const char* name);
	const internal::Il2CppAssembly** (*il2cpp_domain_get_assemblies)(const internal::Il2CppDomain* domain, size_t* size);
	internal::Il2CppDomain* (*il2cpp_domain_get)();
	const internal::Il2CppImage* (*il2cpp_assembly_get_image)(const internal::Il2CppAssembly* assembly);
	const internal::Il2CppType* (*il2cpp_field_get_type)(const internal::FieldInfo * field);
	internal::Il2CppClass* (*il2cpp_class_from_type)(const internal::Il2CppType * type);
	const char* (*il2cpp_type_get_name)(const internal::Il2CppType * type);
	const internal::PropertyInfo* (*il2cpp_class_get_property_from_name)(internal::Il2CppClass * klass, const char *name);
	const internal::MethodInfo* (*il2cpp_property_get_get_method)(const internal::PropertyInfo * prop);
	const internal::MethodInfo* (*il2cpp_property_get_set_method)(const internal::PropertyInfo * prop);
};