#pragma once
#include <type_traits>
#include <unordered_map>
#include <variant>

enum class InvokeTime {
    Before,
    After
};

namespace internal {
    struct Il2CppType {};
    struct Il2CppClass {};

    struct FieldInfo {};
	struct PropertyInfo{};

    struct MethodInfo {
        void *methodPtr;
    };

    struct Il2CppImage {};
    struct Il2CppAssembly {};
    struct Il2CppDomain {};

    struct Il2CppObject {
        void *ptr;

		operator bool() {
			return ptr != nullptr;
		}
    };
};

template<typename T>
struct function_traits {
};

template<typename Ret, typename... Args>
struct function_traits<Ret(Args...)> {
	using PtrType = Ret(__thiscall *)(internal::Il2CppObject, Args...);
	static const int numArgs = sizeof...(Args);
};

class il2cpp_context;

namespace il2cppapi {
	struct Class;

	template<typename T>
	class Field {
	public:
		Field(const il2cpp_context &ctx, internal::Il2CppObject obj, std::variant<const internal::FieldInfo *, const internal::PropertyInfo *> fieldValue) : ctx(ctx), obj(obj), fieldValue(fieldValue) {}

		T get() {
			T value;
			if (obj) {
				if (std::holds_alternative< const internal::FieldInfo *>(fieldValue)) {
					ctx.getValueFromField(obj, std::get<const internal::FieldInfo *>(fieldValue), &value);
				}
				else {
					auto method = ctx.getPropertyGetter(std::get<const internal::PropertyInfo *>(fieldValue));
					if (method) {
						value = static_cast<T(*)(internal::Il2CppObject)>(method->methodPtr)(obj);
					}
				}
			}
			else {
				ctx.getValueFromStaticField(std::get<const internal::FieldInfo *>(fieldValue), &value);
			}
			return value;
		}

		void set(const T &rhs) {
			if (obj) {
				if (std::holds_alternative< const internal::FieldInfo *>(fieldValue)) {
					ctx.setValueFromField(obj, std::get<const internal::FieldInfo *>(fieldValue), &rhs);
				}
				else {
					auto method = ctx.getPropertySetter(std::get<const internal::PropertyInfo *>(fieldValue));
					if (method) {
						static_cast<void(*)(internal::Il2CppObject, const T*)>(method->methodPtr)(obj, &rhs);
					}
				}
			}
			else {
				ctx.setValueFromStaticField(std::get<const internal::FieldInfo *>(fieldValue), &rhs);
			}
		}

		operator T() {
			return get();
		}

		Field& operator=(const T &rhs) {
			set(rhs);
			return *this;
		}

		operator const internal::FieldInfo *() {
			return std::get<const internal::FieldInfo *>(fieldValue);
		}

		operator const internal::PropertyInfo *() {
			return std::get<const internal::PropertyInfo *>(fieldValue);
		}

		Class *getClass() {
			if (std::holds_alternative< const internal::FieldInfo *>(fieldValue)) {
				return ctx.getClassFromField(std::get<const internal::FieldInfo *>(fieldValue));
			}
			else {
				return nullptr;
			}
		}

	private:
		const il2cpp_context &ctx;
		internal::Il2CppObject obj;
		std::variant<const internal::FieldInfo *, const internal::PropertyInfo *> fieldValue;
	};

    struct Class {
        Class(const il2cpp_context& ctx, internal::Il2CppClass *klass) : ctx(ctx), klass(klass) {}

        template<typename Fn>
        typename function_traits<Fn>::PtrType method(const std::string &methodName) const {
            auto it = methods.find(methodName);
            if (it != methods.end()) {
                return static_cast<typename function_traits<Fn>::PtrType>(it->second);
            }

            auto internalMethod = ctx.getClassMethod(klass, methodName, function_traits<Fn>::numArgs);
            if (!internalMethod) {
                return nullptr;
            }

			methods.emplace(std::make_pair(methodName, internalMethod->methodPtr));
            return static_cast<typename function_traits<Fn>::PtrType>(internalMethod->methodPtr);
        }

		template<typename T>
		Field<T> field(internal::Il2CppObject obj, const std::string &fieldName) const {
			auto internalField = ctx.getClassFieldInfo(klass, fieldName, false);
			if (internalField != nullptr) {
				return Field<T>(ctx, obj, internalField);
			}

			//It might be a property, so try that
			auto internalProperty = ctx.getClassPropertyInfo(klass, fieldName, false);
			if (internalProperty != nullptr) {
				return Field<T>(ctx, obj, internalProperty);
			}

			printf("ERROR: Cannot find field/property %s for class!", fieldName.c_str());
			const internal::FieldInfo *null = nullptr;
			return Field<T>(ctx, obj, null);
		}

		template<typename T>
		Field<T> static_field(const std::string &fieldName) const {
			auto internalField = ctx.getClassFieldInfo(klass, fieldName);
			return Field<T>(ctx, internal::Il2CppObject{ nullptr }, internalField);
		}

		operator internal::Il2CppClass*() {
			return klass;
		}

    private:
        const il2cpp_context& ctx;
        internal::Il2CppClass *klass;
        mutable std::unordered_map<std::string, const void *> methods;
    };

    struct Object {
        Object(void *ptr, Class *klass) : ptr(ptr), klass(klass) {}

        void *ptr;
        Class *klass;

        template<typename Fn>
        typename function_traits<Fn>::PtrType method(const std::string &methodName) const {
            return klass->method<Fn>(methodName);
        }

		template<typename T>
		Field<T> field(const std::string &fieldName) const {
			return klass->field<T>(internal::Il2CppObject{ ptr }, fieldName);
		}

		template<typename T>
		Field<T> static_field(const std::string &fieldName) const {
			return klass->static_field<T>(fieldName);
		}

        operator internal::Il2CppObject() {
            return internal::Il2CppObject{ ptr };
        }
    };
}

using ThisPtr = il2cppapi::Object;