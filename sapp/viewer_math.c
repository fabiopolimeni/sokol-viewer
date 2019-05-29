#include "viewer_math.h"

#include "math.h"

#if defined(__cplusplus)
extern "C" {
#endif

vec3f_t squat_axis(quat_t rotation) {
    return svec3_normalize(svec3(
        rotation.x,
        rotation.y,
        rotation.z
    ));
}

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

// https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
vec3f_t __squat_rotate_vec3(quat_t r, vec3f_t v) {
    vec3f_t u = svec3(r.x, r.y, r.z);
    mfloat_t s = r.w;

    vec3f_t a = svec3_multiply_f(u, 2.0f * svec3_dot(u, v));
    vec3f_t b = svec3_multiply_f(v, s * s - svec3_dot(u, u));
    vec3f_t c = svec3_multiply_f(svec3_cross(u, v), 2.0f * s);

    return svec3_add(svec3_add(a, b), c);
}

// alternative from GLM implementation
// https://github.com/g-truc/glm/blob/fce2abd01ce21063bd25ba67c9318be83bf48813/glm/detail/type_quat.inl#L314
vec3f_t squat_rotate_vec3(quat_t r, vec3f_t v) {
    vec3f_t u = svec3(r.x, r.y, r.z);
    vec3f_t uv = svec3_cross(u, v);
    vec3f_t uuv = svec3_cross(u, uv);

    // v + ((cross(u, v) * s) + cross(u, cross(u, v)) * 2.0f
    return svec3_add(v,
        svec3_multiply_f(
            svec3_add(
                svec3_multiply_f(uv, r.w),
                uuv),
            2.0f)
        );
}

mat4f_t transform_to_mat4(transform_t transform) {
    return smat4_translate(smat4_multiply(
        smat4_rotation_quat(
            squat_normalize(transform.rotation)),
        smat4_scaling(
            smat4_identity(), transform.scale)
    ), transform.position);
}

vec3f_t plane_project_point(plane_t plane, vec3f_t point) {
    // p' = p - n * (n.p + d)
    return svec3_subtract(point, svec3_multiply_f(
        plane.normal, svec3_dot(
            plane.normal, point) + plane.distance)
    );
}

#if defined(__cplusplus)
} // extern "C"
#endif
