// Math.cpp (Clippy95)
// --------------------
// Created: 1/15/2025

#include "../FileLogger.h"
#include "../Patcher/patch.h"
#include "../GameConfig.h"
#include "Math.h"
namespace Math
{
    void matrix_multiply_safe(matrix* result, const matrix* lhs, const matrix* rhs) {

        auto safe_dot = [](float a1, float a2, float a3, int step_base) -> float {
            bool nan1 = std::isnan(a1);
            bool nan2 = std::isnan(a2);
            bool nan3 = std::isnan(a3);

            if (nan1) Logger::TypedLog(CHN_DEBUG, "NaN detected at step %d: a1=%f \n", step_base + 1, a1);
            if (nan2) Logger::TypedLog(CHN_DEBUG, "NaN detected at step %d: a2=%f \n", step_base + 2, a2);
            if (nan3) Logger::TypedLog(CHN_DEBUG, "NaN detected at step %d: a3=%f \n", step_base + 3, a3);

            return (nan1 || nan2 || nan3) ? 0.0f : a1 + a2 + a3;
            };

        auto safe_mul = [](float a, float b, int step) -> float {
            bool nan_a = std::isnan(a);
            bool nan_b = std::isnan(b);

            if (nan_a) Logger::TypedLog(CHN_DEBUG, "NaN detected at step %d: input_a=%f \n", step, a);
            if (nan_b) Logger::TypedLog(CHN_DEBUG, "NaN detected at step %d: input_b=%f \n", step, b);

            return (nan_a || nan_b) ? 0.0f : a * b;
            };

        result->rvec.x = safe_dot(safe_mul(lhs->rvec.x, rhs->rvec.x, 10),
            safe_mul(lhs->rvec.y, rhs->uvec.x, 11),
            safe_mul(lhs->rvec.z, rhs->fvec.x, 12), 13);

        result->rvec.y = safe_dot(safe_mul(lhs->rvec.x, rhs->rvec.y, 20),
            safe_mul(lhs->rvec.y, rhs->uvec.y, 21),
            safe_mul(lhs->rvec.z, rhs->fvec.y, 22), 23);

        result->rvec.z = safe_dot(safe_mul(lhs->rvec.x, rhs->rvec.z, 30),
            safe_mul(lhs->rvec.y, rhs->uvec.z, 31),
            safe_mul(lhs->rvec.z, rhs->fvec.z, 32), 33);

        result->uvec.x = safe_dot(safe_mul(lhs->uvec.x, rhs->rvec.x, 40),
            safe_mul(lhs->uvec.y, rhs->uvec.x, 41),
            safe_mul(lhs->uvec.z, rhs->fvec.x, 42), 43);

        result->uvec.y = safe_dot(safe_mul(lhs->uvec.x, rhs->rvec.y, 50),
            safe_mul(lhs->uvec.y, rhs->uvec.y, 51),
            safe_mul(lhs->uvec.z, rhs->fvec.y, 52), 53);

        result->uvec.z = safe_dot(safe_mul(lhs->uvec.x, rhs->rvec.z, 60),
            safe_mul(lhs->uvec.y, rhs->uvec.z, 61),
            safe_mul(lhs->uvec.z, rhs->fvec.z, 62), 63);

        result->fvec.x = safe_dot(safe_mul(lhs->fvec.x, rhs->rvec.x, 70),
            safe_mul(lhs->fvec.y, rhs->uvec.x, 71),
            safe_mul(lhs->fvec.z, rhs->fvec.x, 72), 73);

        result->fvec.y = safe_dot(safe_mul(lhs->fvec.x, rhs->rvec.y, 80),
            safe_mul(lhs->fvec.y, rhs->uvec.y, 81),
            safe_mul(lhs->fvec.z, rhs->fvec.y, 82), 83);

        result->fvec.z = safe_dot(safe_mul(lhs->fvec.x, rhs->rvec.z, 90),
            safe_mul(lhs->fvec.y, rhs->uvec.z, 91),
            safe_mul(lhs->fvec.z, rhs->fvec.z, 92), 93);
    }
    bool has_nan_values(const matrix* m) {
        return std::isnan(m->rvec.x) || std::isnan(m->rvec.y) || std::isnan(m->rvec.z) ||
            std::isnan(m->uvec.x) || std::isnan(m->uvec.y) || std::isnan(m->uvec.z) ||
            std::isnan(m->fvec.x) || std::isnan(m->fvec.y) || std::isnan(m->fvec.z);
    }

	namespace Fixes {
		SafetyHookMid matrix_operator_multiplication_midhook{};
		void Init() {
			// Idea to fix issue #14 IK/Foot issue getting messed up, the actual call cause is at 0x0x00CE9600, but rn im doing this globally as it makes the most sense - Clippy95
            matrix_operator_multiplication_midhook = safetyhook::create_mid(0x00BE2F57, [](SafetyHookContext& ctx) {
                matrix* result = (matrix*)ctx.eax;
                matrix* thisa = (matrix*)ctx.edx;
                const matrix* m = (matrix*)ctx.ecx;

                bool lhs_has_nan = has_nan_values(thisa);
                bool rhs_has_nan = has_nan_values(m);

                if (lhs_has_nan || rhs_has_nan) {
                    if (lhs_has_nan) {
                        Logger::TypedLog(CHN_DEBUG, "NaN detected in LHS matrix at 0x%p", thisa);
                    }
                    if (rhs_has_nan) {
                        Logger::TypedLog(CHN_DEBUG, "NaN detected in RHS matrix at 0x%p", m);
                    }

                    matrix lhs_copy = *thisa;
                    matrix rhs_copy = *m;

                    matrix_multiply_safe(result, &lhs_copy, &rhs_copy);

                    ctx.eip = 0x00BE313F;
                }
                else {
                }
                });
		}
	}
	void Init() {
		if(GameConfig::GetValue("Debug","MathFixes",1))
		Math::Fixes::Init();
	}
}
