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

	//Chain targets require the trigger be held down, otherwise they will fail. We hook that function and force the value to be true if we just hit a chain note.
	bindingCtx.bindClassFunction("", "Gun", "IsTriggerHeld", InvokeTime::Before, [](const MethodInvocationContext& ctx, ThisPtr ths) -> std::optional<bool> {
		if (gunData[ths.ptr].hitChain) {
			ctx.stopExecution();
			return true;
		}

		return std::nullopt;
	});

	//We hook the target OnHit function to know if our throw was successful. We dont't prevent any execution, and we don't change the return value
	bindingCtx.bindClassFunction("", "Target", "OnHit", InvokeTime::After, [](const MethodInvocationContext& ctx, ThisPtr ths, internal::Il2CppObject* gun, int32_t attackType, float aim, Vector2 targetHitPos, Vector3 intersectionPoint, float meleeVelocity, bool forceSuccessful) -> std::optional<bool> {
		auto wasSuccessful = ctx.getReturn<bool>();
		gunData[gun].hitTarget = wasSuccessful ? ths : internal::Il2CppObject{ nullptr };
		return std::nullopt;
	});

	//We want to store extra data related to guns, so we after the function that sets them up and fill our map with the pointers.
	bindingCtx.bindClassFunction("", "KataConfig", "SetupGuns", InvokeTime::After, [](const MethodInvocationContext& ctx, ThisPtr kataConfig) -> void {
		auto getGun = kataConfig.method<void*(int32_t)>("GetGun");
		gunData[getGun(kataConfig, 2)] = {};
		gunData[getGun(kataConfig, 1)] = {};
	});

	//We use ScoreKeeper.CheckCuePass as our "Update" function. There might be better ones, but this was called every frame while in a song, which is what we want.
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

			//If we just re-grabbed the gun (trick state 0), and we aren't lerping back, set that up.
			if (data.returnPoint == std::nullopt && IsUsingPhysics(gun) && newState == 0) {
				data.returnPoint = std::chrono::high_resolution_clock::now();

				auto rigidBody = gun.field<internal::Il2CppObject>("rigidBody");
				if (rigidBody.get()) {
					data.returnPosStart = rigidBodyClass->field<Vector3>(rigidBody.get(), "position").get();
				}
			}

			//If we should be lerping back our hands, then do it
			if (data.returnPoint) {
				auto diff = std::chrono::high_resolution_clock::now() - data.returnPoint.value();
				const std::chrono::duration<double, std::milli> dtMs = diff;
				double dt = dtMs.count();
				dt /= 1000.0f;

				//We do a lerp over 0.1s, so that the gun snaps back much faster than normal
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
				//At the end of the lerp, force reset the throw trick
				else {
					EndThrowTrick(gun);
					data.returnPoint = std::nullopt;
				}

			}

			//We just started a throw trick
			if (data.lastTrickState == 0 && newState == 2) {
				data.hitTarget = internal::Il2CppObject{ nullptr };
				data.hitChain = false;

				//If we we're returning, just end the trick
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

					//Go through all active targets
					for (int32_t i = 0; i < activeTargetArraySize; ++i) {
						internal::Il2CppObject target = getTarget(activeTargets.get(), i);

						//If we can hit the target
						auto isValidTarget = targetClass->method<bool(internal::Il2CppObject, int32_t, bool)>("IsValidTarget");
						if (isValidTarget(target, gun, 0, false)) {
							Vector3 intersect;
							int32_t firePointHistory = 0;

							//And the gun can shoot the target
							if (FindBestIntersection(gun, target, 5.0f, true, &intersect, &firePointHistory)) {

								//Tell the gun to fire, because of the hooked Target::OnHit we will know if we hit
								Fire(gun, target, intersect, firePointHistory, false, false, internal::Il2CppObject{ nullptr });

								if (data.hitTarget.ptr != nullptr) {

									//If we did hit, mark down if it was a chain
									auto cue = targetClass->field<internal::Il2CppObject>(data.hitTarget, "mCue");
									int32_t behavior = cue.getClass()->field<int32_t>(cue, "behavior");
									data.hitChain = behavior == 4;

									//And then move the gun to the target's hit point
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