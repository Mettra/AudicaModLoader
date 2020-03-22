#include <il2cpp/il2cpp_binding.h>
#include <il2cpp/il2cpp_context.h>
#include <il2cpp/il2cpp_types.h>
#include <chrono>

struct Vector2 {
	float x = 0.0f;
	float y = 0.0f;
};

struct Vector3 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	Vector3 operator*(float a) {
		return Vector3{ x * a, y * a, z * a };
	}

	Vector3 operator+(const Vector3 &rhs) {
		return Vector3{ x + rhs.x,  y + rhs.y, z + rhs.z };
	}

	Vector3 operator-(const Vector3 &rhs) {
		return Vector3{ x - rhs.x,  y - rhs.y, z - rhs.z };
	}
};

struct CustomGunData {
	int32_t lastTrickState = 0;

	internal::Il2CppObject hitTarget;
	bool hitChain = false;
	std::optional< std::chrono::high_resolution_clock::time_point> returnPoint;
	Vector3 returnPosStart;
};
std::unordered_map<void *, CustomGunData> gunData;


extern "C" __declspec(dllexport) void registerHooks(il2cpp_binding &bindingCtx) {
	bindingCtx.bindClassFunction("", "Gun", "IsTriggerHeld", InvokeTime::Before, [](const MethodInvocationContext& ctx, ThisPtr ths) -> std::optional<bool> {
		if (gunData[ths.ptr].hitChain) {
			ctx.stopExecution();
			return true;
		}

		return std::nullopt;
	});

    bindingCtx.bindClassFunction("", "Target", "OnHit", InvokeTime::After, [](const MethodInvocationContext& ctx, ThisPtr ths, internal::Il2CppObject* gun, int32_t attackType, float aim, Vector2 targetHitPos, Vector3 intersectionPoint, float meleeVelocity, bool forceSuccessful) -> std::optional<bool> {
        auto wasSuccessful = ctx.getReturn<bool>();
        gunData[gun].hitTarget = wasSuccessful ? ths : internal::Il2CppObject{ nullptr };
        return std::nullopt;
    });

    bindingCtx.bindClassFunction("", "KataConfig", "SetupGuns", InvokeTime::After, [](const MethodInvocationContext& ctx, ThisPtr kataConfig) -> void {
        auto getGun = kataConfig.method<void*(int32_t)>("GetGun");
        gunData[getGun(kataConfig, 2)] = {};
        gunData[getGun(kataConfig, 1)] = {};
    });

    bindingCtx.bindClassFunction("", "ScoreKeeper", "CheckCuePass", InvokeTime::Before, [](const MethodInvocationContext& ctx, ThisPtr scoreKeeper, float tick) -> std::optional<void*> {
        auto gunClass = ctx.getGlobalContext().getClass("", "Gun");

        auto GetCurrentTrick = gunClass->method<int32_t()>("GetCurrentTrick");
        auto Fire = gunClass->method<void(internal::Il2CppObject target, Vector3 intersection, int firepointHistoryIndex, bool forceSuccessful, bool useChainBeam, internal::Il2CppObject shotToy)>("Fire");
        auto FindBestIntersection = gunClass->method<bool(internal::Il2CppObject target, float aimRadius, bool temporalAssist, Vector3 *intersection, int *firepointHistoryIndex)>("FindBestIntersection");
        auto IsUsingPhysics = gunClass->method<bool()>("IsUsingPhysics");
        auto EndThrowTrick = gunClass->method<void()>("EndThrowTrick");


        auto rigidBodyClass = ctx.getGlobalContext().getClass("UnityEngine", "Rigidbody");

        for (auto &&[gunPtr, data] : gunData) {
            il2cppapi::Object gun(gunPtr, gunClass);

            int32_t newState = GetCurrentTrick(gun);

            if (data.returnPoint == std::nullopt && IsUsingPhysics(gun) && newState == 0) {
                data.returnPoint = std::chrono::high_resolution_clock::now();

                auto rigidBody = gun.field<internal::Il2CppObject>("rigidBody");
                if (rigidBody.get()) {
                    data.returnPosStart = rigidBodyClass->field<Vector3>(rigidBody.get(), "position").get();
                }
            }

            if (data.returnPoint) {
                auto diff = std::chrono::high_resolution_clock::now() - data.returnPoint.value();
                const std::chrono::duration<double, std::milli> dtMs = diff;
                double dt = dtMs.count();
                dt /= 1000.0f;

                if (dt < 0.1f) {
                    auto rigidBody = gun.field<internal::Il2CppObject>("rigidBody");
                    if (rigidBody.get()) {
                        auto position = rigidBodyClass->field<Vector3>(rigidBody.get(), "position");
                        auto transform = gunClass->field<internal::Il2CppObject>(gun, "transform");

                        Vector3 gunPos = ctx.getGlobalContext().getClass("UnityEngine", "Transform")->field<Vector3>(transform, "position");
                        Vector3 vec = data.returnPosStart + (gunPos - data.returnPosStart) * (dt / 0.1f);
                        position = vec;
                    }
                }
                else {
                    EndThrowTrick(gun);
                    data.returnPoint = std::nullopt;
                }

            }

            //We just started a throw trick
            if (data.lastTrickState == 0 && newState == 2) {
                data.hitTarget = internal::Il2CppObject{ nullptr };
                data.hitChain = false;

                if (data.returnPoint) {
                    EndThrowTrick(gun);
                    data.returnPoint = std::nullopt;
                }

                auto targetClass = ctx.getGlobalContext().getClass("", "Target");
                auto activeTargets = targetClass->static_field<internal::Il2CppObject>("ActiveTargets");

                if (activeTargets.get()) {
                    auto listClass = activeTargets.getClass();

                    auto getTarget = listClass->method<internal::Il2CppObject(int32_t)>("get_Item");
                    int32_t activeTargetArraySize = listClass->field<int32_t>(activeTargets, "Count");

                    for (int32_t i = 0; i < activeTargetArraySize; ++i) {
                        internal::Il2CppObject target = getTarget(activeTargets.get(), i);

                        auto isValidTarget = targetClass->method<bool(internal::Il2CppObject, int32_t, bool)>("IsValidTarget");
                        if (isValidTarget(target, gun, 0, false)) {
                            Vector3 intersect;
                            int32_t firePointHistory = 0;
                            if (FindBestIntersection(gun, target, 5.0f, true, &intersect, &firePointHistory)) {
                                Fire(gun, target, intersect, firePointHistory, false, false, internal::Il2CppObject{ nullptr });

                                if (data.hitTarget.ptr != nullptr) {
                                    auto cue = targetClass->field<internal::Il2CppObject>(data.hitTarget, "mCue");
                                    int32_t behavior = cue.getClass()->field<int32_t>(cue, "behavior");
                                    data.hitChain = behavior == 4;

                                    auto rigidBody = gun.field<internal::Il2CppObject>("rigidBody");
                                    if (rigidBody.get()) {
                                        auto position = rigidBodyClass->field<Vector3>(rigidBody.get(), "position");
                                        position = intersect;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }

            data.lastTrickState = newState;
        }

        return std::nullopt;
    });
}