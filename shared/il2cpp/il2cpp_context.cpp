#include "il2cpp_context.h"


#include "il2cpp_binding.h"
using namespace internal;

il2cpp_context::~il2cpp_context() = default;

il2cpp_context::il2cpp_context(HMODULE gameAssemblyModule) {
	*(void**)(&il2cpp_class_get_field_from_name) = GetProcAddress(gameAssemblyModule, "il2cpp_class_get_field_from_name");
	*(void**)(&il2cpp_field_get_value) = GetProcAddress(gameAssemblyModule, "il2cpp_field_get_value");
	*(void**)(&il2cpp_field_set_value) = GetProcAddress(gameAssemblyModule, "il2cpp_field_set_value");
	*(void**)(&il2cpp_field_static_get_value) = GetProcAddress(gameAssemblyModule, "il2cpp_field_static_get_value");
	*(void**)(&il2cpp_field_static_set_value) = GetProcAddress(gameAssemblyModule, "il2cpp_field_static_set_value");
	*(void**)(&il2cpp_class_get_method_from_name) = GetProcAddress(gameAssemblyModule, "il2cpp_class_get_method_from_name");
	*(void**)(&il2cpp_class_from_name) = GetProcAddress(gameAssemblyModule, "il2cpp_class_from_name");
	*(void**)(&il2cpp_domain_get_assemblies) = GetProcAddress(gameAssemblyModule, "il2cpp_domain_get_assemblies");
	*(void**)(&il2cpp_domain_get) = GetProcAddress(gameAssemblyModule, "il2cpp_domain_get");
	*(void**)(&il2cpp_assembly_get_image) = GetProcAddress(gameAssemblyModule, "il2cpp_assembly_get_image");
	*(void**)(&il2cpp_field_get_type) = GetProcAddress(gameAssemblyModule, "il2cpp_field_get_type");
	*(void**)(&il2cpp_class_from_type) = GetProcAddress(gameAssemblyModule, "il2cpp_class_from_type");
	*(void**)(&il2cpp_type_get_name) = GetProcAddress(gameAssemblyModule, "il2cpp_type_get_name");
	*(void**)(&il2cpp_class_get_property_from_name) = GetProcAddress(gameAssemblyModule, "il2cpp_class_get_property_from_name");
	*(void**)(&il2cpp_property_get_get_method) = GetProcAddress(gameAssemblyModule, "il2cpp_property_get_get_method");
	*(void**)(&il2cpp_property_get_set_method) = GetProcAddress(gameAssemblyModule, "il2cpp_property_get_set_method");

	mBindingContext = std::make_unique<il2cpp_binding>(*this);
}

il2cpp_binding &il2cpp_context::getBinding() const {
	return *mBindingContext;
}

il2cppapi::Class* il2cpp_context::getClass(const std::string &namespaceName, const std::string &className) const {
	std::string key = namespaceName + "." + className;
	auto it = mClasses.find(key);
	if(it != mClasses.end()) {
		return &it->second;
	}

	auto dom = il2cpp_domain_get();
	if (!dom) {
		printf("ERROR: getClass: Could not get domain!\n");
		return nullptr;
	}
	size_t assemb_count;
	const Il2CppAssembly** allAssemb = il2cpp_domain_get_assemblies(dom, &assemb_count);

	

	for (int i = 0; i < assemb_count; i++) {
		auto assemb = allAssemb[i];
		auto img = il2cpp_assembly_get_image(assemb);
		if (!img) {
			printf("ERROR: Assembly has a null image!\n");
			continue;
		}

		auto klass = il2cpp_class_from_name(img, namespaceName.c_str(), className.c_str());
		if (klass) {
			il2cppapi::Class newClass(*this, klass);
			auto p = mClasses.emplace(std::make_pair(key, std::move(newClass)));
			return &p.first->second;
		}
	}
	printf("ERROR: getClass: Could not find class with namespace %s and name: %s\n", namespaceName.c_str(), className.c_str());
	return nullptr;
}

il2cppapi::Class* il2cpp_context::getClassFromField(const internal::FieldInfo* field) const {
	auto type = il2cpp_field_get_type(field);
	if (!type) {
		printf("ERROR: getClassFromField: Field has no type!\n");
		return nullptr;
	}

	auto name = il2cpp_type_get_name(type);
	auto it = mClasses.find(name);
	if (it != mClasses.end()) {
		return &it->second;
	}

	auto klass = il2cpp_class_from_type(type);
	if (!klass) {
		printf("ERROR: getClassFromField: Field has no class type!\n");
		return nullptr;
	}

	il2cppapi::Class newClass(*this, klass);
	auto p = mClasses.emplace(std::make_pair(name, std::move(newClass)));
	return &p.first->second;
}

void il2cpp_context::getValueFromField(internal::Il2CppObject obj, const internal::FieldInfo * field, void * value) const {
	il2cpp_field_get_value(obj, field, value);
}

void il2cpp_context::setValueFromField(internal::Il2CppObject obj, const internal::FieldInfo * field, const void * value) const {
	il2cpp_field_set_value(obj, field, value);
}

void il2cpp_context::getValueFromStaticField(const internal::FieldInfo *field, void * value) const {
	il2cpp_field_static_get_value(field, value);
}

void il2cpp_context::setValueFromStaticField(const internal::FieldInfo *field, const void * value) const {
	il2cpp_field_static_set_value(field, value);
}

const MethodInfo *il2cpp_context::getClassMethod(Il2CppClass* klass, const std::string &methodName, int argsCount) const {
    auto method = il2cpp_class_get_method_from_name(klass, methodName.c_str(), argsCount);;
    if(method == nullptr) {
        printf("ERROR: getClassMethod: Could not find method %s with args %d on class!\n", methodName.c_str(), argsCount);
    }

	return method;
}

const FieldInfo *il2cpp_context::getClassFieldInfo(Il2CppClass* klass, const std::string &fieldName, bool error) const {
    auto field = il2cpp_class_get_field_from_name(klass, fieldName.c_str());
    if(error && field == nullptr) {
        printf("ERROR: getClassFieldInfo: Could not find field %s on class!\n", fieldName.c_str());
    }
    
	return field;
}

const internal::PropertyInfo *il2cpp_context::getClassPropertyInfo(internal::Il2CppClass* klass, const std::string &propName, bool error) const {
	auto prop = il2cpp_class_get_property_from_name(klass, propName.c_str());
	if (error && prop == nullptr) {
		printf("ERROR: getClassFieldInfo: Could not find property %s on class!\n", propName.c_str());
	}

	return prop;
}

const internal::MethodInfo *il2cpp_context::getPropertyGetter(const internal::PropertyInfo *propertyInfo) const {
	auto method = il2cpp_property_get_get_method(propertyInfo);
	if (method == nullptr) {
		printf("ERROR: getClassMethod: Could not find get method for property!\n");
	}

	return method;
}

const internal::MethodInfo *il2cpp_context::getPropertySetter(const internal::PropertyInfo *propertyInfo) const {
	auto method = il2cpp_property_get_set_method(propertyInfo);
	if (method == nullptr) {
		printf("ERROR: getClassMethod: Could not find set method for property!\n");
	}

	return method;
}