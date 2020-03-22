#pragma once
#include <vector>
#include <functional>
#include <optional>
#include <algorithm>
#include "functional_type.h"

#include "il2cpp_types.h"

class il2cpp_context;

class MethodInvocationContext {
public:
	MethodInvocationContext(const il2cpp_context &ctx, size_t returnData) 
		: mCtx(ctx) {
		mReturnData.resize(returnData);
	}

	const il2cpp_context &getGlobalContext() const {
		return mCtx;
	}

	template<typename T>
	const T& getReturn() const {
		return *(T*)mReturnData.data();
	}

	template<typename T>
	void setReturn(T&& ret) {
		*(T*)mReturnData.data() = std::move(ret);
	}

	void stopExecution() const {
		mStopExecution = true;
	}

	bool didStopExecution() const {
		return mStopExecution;
	}


private:
	const il2cpp_context &mCtx;
	std::vector<uint8_t> mReturnData;
	mutable bool mStopExecution = false;
};


template<typename Ret, typename... Args>
struct MethodHookBase {
	struct Node {
		std::function<Ret(const MethodInvocationContext& ctx, ThisPtr, Args...)> fn;
		InvokeTime invokeTime = InvokeTime::Before;
		int priority = 0;
		Node *next = nullptr;
	};

	static Node *getNewNode(std::function<Ret(const MethodInvocationContext&, ThisPtr, Args...)> &&fn, InvokeTime invokeTime, int priority = 0) {
		Node *node = new Node();
		node->fn = std::move(fn);
		node->priority = priority;
		node->invokeTime = invokeTime;
		return node;
	}

	static void *sortNodes(void *start_void) {
		Node *start = static_cast<Node *>(start_void);

		std::vector<Node *> allNodes;
		Node *current = start;
		while (current != nullptr) {
			allNodes.emplace_back(current);
			current = current->next;
		}

		std::sort(allNodes.begin(), allNodes.end(), [](Node *lhs, Node *rhs) {
			return lhs->priority < rhs->priority;
		});

		Node *prev = allNodes[0];
		for (int i = 1; i < allNodes.size(); ++i) {
			prev->next = allNodes[i];
			prev = allNodes[i];
		}

		prev->next = nullptr;
		return allNodes[0];
	}

	static const il2cpp_context *ctx;
};

template<typename Ret, typename... Args>
const il2cpp_context *MethodHookBase<Ret, Args...>::ctx = nullptr;

template<typename Ret, typename... Args>
struct MethodHook : public MethodHookBase<std::optional<Ret>, Args...> {
	using Node = typename MethodHookBase<std::optional<Ret>, Args...>::Node;

	__declspec(noinline) Ret invoke(Args... args) {
		auto memFn = &MethodHook<Ret, Args...>::invoke;
		auto data = ctx->getBinding().getDataForInvocation(*(void**)&memFn);

		ThisPtr ths((void*)this, data.klass);

		MethodInvocationContext methodCtx(*ctx, sizeof(Ret));
		invoke_all(methodCtx, static_cast<Ret(__thiscall *)(void*, Args...)>(data.originalFn), static_cast<Node *>(data.node), ths, args...);
		return methodCtx.getReturn<Ret>();
	}

	template<typename std::size_t... I>
    static void invoke_all(MethodInvocationContext& ctx, Ret(*originalFn)(void*, Args...), Node *invokeList, ThisPtr ths, Args... args) {
        Node *current = invokeList;
        while(current != nullptr) {
            if(current->invokeTime == InvokeTime::Before) {
                auto ret = current->fn(ctx, ths, args...);
                if(ret) {
					ctx.setReturn(std::move(ret.value()));
                }

				if (ctx.didStopExecution()) {
					return;
				}
            }

            current = current->next;
        }

        auto originalRet = originalFn(ths.ptr, args...);
		ctx.setReturn(std::move(originalRet));

        current = invokeList;
        while(current != nullptr) {
            if(current->invokeTime == InvokeTime::After) {
                auto ret = current->fn(ctx, ths, args...);
                if(ret) {
					ctx.setReturn(std::move(ret.value()));
                }

				if (ctx.didStopExecution()) {
					return;
				}
            }

            current = current->next;
        }
    }
};

template<typename... Args>
struct MethodHook<void, Args...> : public MethodHookBase<void, Args...> {
	using Ret = void;
	using Node = typename MethodHookBase<Ret, Args...>::Node;

	__declspec(noinline) Ret invoke(Args... args) {
		auto memFn = &MethodHook<Ret, Args...>::invoke;
		auto data = ctx->getBinding().getDataForInvocation(*(void**)&memFn);
		ThisPtr ths((void*)this, data.klass);

		MethodInvocationContext methodCtx(*ctx, 0);
		invoke_all(methodCtx, static_cast<Ret(__thiscall *)(void*, Args...)>(data.originalFn), static_cast<Node *>(data.node), ths, args...);
	}

	template<typename std::size_t... I>
	static Ret invoke_all(MethodInvocationContext& ctx, Ret(*originalFn)(void*, Args...), Node *invokeList, ThisPtr ths, Args... args) {
		Node *current = invokeList;
		while (current != nullptr) {
			if (current->invokeTime == InvokeTime::Before) {
				current->fn(ctx, ths, args...);
				if (ctx.didStopExecution()) {
					return;
				}
			}

			current = current->next;
		}

		originalFn(ths.ptr, args...);

		current = invokeList;
		while (current != nullptr) {
			if (current->invokeTime == InvokeTime::After) {
				current->fn(ctx, ths, args...);
				if (ctx.didStopExecution()) {
					return;
				}
			}

			current = current->next;
		}
	}
};

template<typename T>
struct is_valid_function_type {
	static const bool value = false;
};

template<typename Ret, typename... Args>
struct is_valid_function_type<std::function<Ret(Args...)>> {
	using tuple_type = std::tuple<Args...>;
	static const bool value = std::is_same_v<std::tuple_element_t<0, tuple_type>, const MethodInvocationContext&> && std::is_same_v<std::tuple_element_t<1, tuple_type>, ThisPtr>;
};

class il2cpp_binding {
public:
	il2cpp_binding(const il2cpp_context &ctx);


	struct HookCall {
		void *originalFn = nullptr;
		void *invokeFn = nullptr;
		void *node = nullptr;
		void *(*sortFn)(void *) = nullptr;
		il2cppapi::Class *klass = nullptr;
	};

	template<typename Ret, typename... Args>
	void bindClassFunction(const std::string &namespaceName, const std::string& className, const std::string& methodName, InvokeTime invokeTime, int priority, std::function<Ret(const MethodInvocationContext& ctx, ThisPtr ths, Args...)> &&callback) {
		_bindClassFunction(namespaceName, className, methodName, invokeTime, priority, std::move(callback));
	}

	template<typename Fn>
	void bindClassFunction(const std::string &namespaceName, const std::string& className, const std::string& methodName, InvokeTime invokeTime, int priority, Fn &&callback) {
		functional_type_t<Fn> fn = [callback = std::move(callback)](auto&&... args)
		{
			return callback(std::forward<decltype(args)>(args)...);
		};

		static_assert(is_valid_function_type<decltype(fn)>::value, "Invalid function signature! Make sure your function starts with `const il2cpp_context& ctx, ThisPtr ths`");

		_bindClassFunction(namespaceName, className, methodName, invokeTime, priority, std::move(fn));
	}

	//Default priority binding, where priority = 0
	template<typename Fn>
	void bindClassFunction(const std::string &namespaceName, const std::string& className, const std::string& methodName, InvokeTime invokeTime, Fn &&fn) {
		bindClassFunction(namespaceName, className, methodName, invokeTime, 0, std::move(fn));
	}

	const HookCall& getDataForInvocation(void *invokeFn) const;
	const il2cpp_context& getIL2CPPContext() const;

	void setupHooks();
private:

    void* _getCallList(const std::string &namespaceName, const std::string& className, const std::string& methodName);
    void _setCallList(const std::string &namespaceName, const std::string& className, const std::string& methodName, void *node, void *invokeFn, void *(*sortFn)(void *), size_t numArgs);


	template<typename Ret, typename... Args>
	void _bindClassFunction(const std::string &namespaceName, const std::string& className, const std::string& methodName, InvokeTime invokeTime, int priority, std::function<std::optional<Ret>(const MethodInvocationContext& ctx, ThisPtr ths, Args...)> &&callback) {
		auto *node = MethodHook<Ret, Args...>::getNewNode(std::move(callback), invokeTime, priority);
		_bindClassFunction<Ret, Args...>(namespaceName, className, methodName, invokeTime, priority, node);
	}

	template<typename... Args>
	void _bindClassFunction(const std::string &namespaceName, const std::string& className, const std::string& methodName, InvokeTime invokeTime, int priority, std::function<void(const MethodInvocationContext& ctx, ThisPtr ths, Args...)> &&callback) {
		using FnRet = void;

		auto *node = MethodHook<FnRet, Args...>::getNewNode(std::move(callback), invokeTime, priority);
		_bindClassFunction<FnRet, Args...>(namespaceName, className, methodName, invokeTime, priority, node);
	}

	template<typename Ret, typename... Args>
	void _bindClassFunction(const std::string &namespaceName, const std::string& className, const std::string& methodName, InvokeTime invokeTime, int priority, typename MethodHook<Ret, Args...>::Node *node) {
		auto callList = _getCallList(namespaceName, className, methodName);
		if (callList == nullptr) {
			auto memberFn = &MethodHook<Ret, Args...>::invoke;
			_setCallList(namespaceName, className, methodName, node, *(void **)&memberFn, &MethodHook<Ret, Args...>::sortNodes, sizeof...(Args));
			MethodHook<Ret, Args...>::ctx = &getIL2CPPContext();
		}
		else {
			typename MethodHook<Ret, Args...>::Node *currentNode = static_cast<typename MethodHook<Ret, Args...>::Node *>(callList);
			while (currentNode->next != nullptr) {
				currentNode = currentNode->next;
			}

			currentNode->next = node;
		}
	}

	const il2cpp_context &mCtx;
	std::vector<HookCall> mHooks;
	std::unordered_map<std::string, size_t> mHooksByMethodName;
	std::unordered_map<void *, size_t> mHooksByInvocation;
};