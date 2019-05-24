#include "viewer_math.h"

#include "math.h"

#if defined(__cplusplus)
extern "C" {
#endif

//https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
quat_t squat_from_euler(vec3f_t euler) {
    mfloat_t cy = MCOS(euler.z * 0.5f);
    mfloat_t sy = MSIN(euler.z * 0.5f);
    mfloat_t cp = MCOS(euler.y * 0.5f);
    mfloat_t sp = MSIN(euler.y * 0.5f);
    mfloat_t cr = MCOS(euler.x * 0.5f);
    mfloat_t sr = MSIN(euler.x * 0.5f);

    return (quat_t){
        .w = cy * cp * cr + sy * sp * sr,
        .x = cy * cp * sr - sy * sp * cr,
        .y = sy * cp * sr + cy * sp * cr,
        .z = sy * cp * cr - cy * sp * sr
    };
}

vec3f_t squat_to_euler(quat_t q) {
    // roll (x-axis rotation)
	mfloat_t sinr_cosp = +2.0f * (q.w * q.x + q.y * q.z);
	mfloat_t cosr_cosp = +1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    mfloat_t roll = MATAN2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
	mfloat_t sinp = +2.0f * (q.w * q.y - q.z * q.x);
	mfloat_t pitch = (MFABS(sinp) >= 1.f)
		? copysignf(MPI / 2, sinp) // use 90 degrees if out of range
		: MASIN(sinp);

	// yaw (z-axis rotation)
	mfloat_t siny_cosp = +2.0f * (q.w * q.z + q.x * q.y);
	mfloat_t cosy_cosp = +1.0f - 2.0f * (q.y* q.y + q.z * q.z);  
	mfloat_t yaw = MATAN2(siny_cosp, cosy_cosp);

    return (vec3f_t){.x = roll, .y = pitch, .z = yaw};
}

mat4f_t transform_to_mat4(transform_t transform) {
    return smat4_translate(smat4_multiply(
        smat4_rotation_quat(
            squat_normalize(transform.rotation)),
        smat4_scaling(
            smat4_identity(), transform.scale)
    ), transform.position);
}

#if defined(__cplusplus)
} // extern "C"
#endif
