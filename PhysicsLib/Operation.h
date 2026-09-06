#pragma once

#include "Vec3.h"
#include "Quat.h"
#include "Mat3.h"
#include "MyNumbers.h"

#include <cmath>
#include <cassert>


// Vec3 ---------------------------------------------------------

inline float Vec3Dot(const Vec3& a, const Vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3	Vec3Cross(const Vec3& a, const Vec3& b)
{
	return Vec3(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

inline float	Vec3LengthSq(const Vec3& v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline float	Vec3Length(const Vec3& v)
{
	return sqrtf(Vec3LengthSq(v));
}

inline Vec3	Vec3Normalize(const Vec3& v)
{
	float len = Vec3Length(v);
	if (0.f == len)
		return Vec3(0.f, 0.f, 0.f);
	float inv = 1.f / len;

	return v * inv;
}

inline Vec3	Vec3Lerp(const Vec3& a, const Vec3& b, float scalar)
{
	return a + (b - a) * scalar;
}

inline Vec3 Vec3Zero()
{
	return Vec3(0.f, 0.f, 0.f);
}

// Quat ---------------------------------------------------------

inline Quat QuatFromAxisAngle(const Vec3& axis, float angle)
{
	float half = angle * 0.5f;
	float sin = sinf(half);
	return Quat(axis.x * sin, axis.y * sin, axis.z * sin, cosf(half));
}

inline void QuatToAxisAngle(const Quat q, Vec3& outAxis, float& outAngle)
{
	float w = q.w;
	if (w > 1.f) w = 1.f;
	if (w < -1.f) w = -1.f;

	outAngle = 2.f * acosf(w);

	float halfSin = sqrtf(1.f - w * w);

	if (0.0001f < halfSin)
	{
		float inv = 1.f / halfSin;
		outAxis = Vec3(q.x * inv, q.y * inv, q.z * inv);
	}
	else
	{
		outAxis = Vec3(1.f, 0.f, 0.f);
		outAngle = 0.f;
	}
}

inline Quat QuatConjugate(const Quat& q)
{
	return Quat(-q.x, -q.y, -q.z, q.w);
}

inline Quat QuatMultiply(const Quat& q1, const Quat& q2)
{
	return q1 * q2;
}

inline Vec3 QuatRotateVec3(const Quat& q, const Vec3& v)
{
	Quat t(v.x, v.y, v.z, 0.f);
	Quat result = q * t * QuatConjugate(q);
	return Vec3(result.x, result.y, result.z);
}

inline Mat3 QuatToMat3(const Quat& q)
{
	float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
	float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
	float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

	Mat3 ret;
	ret.m[0][0] = 1 - 2 * (yy + zz);
	ret.m[0][1] = 2 * (xy + wz);
	ret.m[0][2] = 2 * (xz - wy);

	ret.m[1][0] = 2 * (xy - wz);
	ret.m[1][1] = 1 - 2 * (xx + zz);
	ret.m[1][2] = 2 * (yz + wx);

	ret.m[2][0] = 2 * (xz + wy);
	ret.m[2][1] = 2 * (yz - wx);
	ret.m[2][2] = 1 - 2 * (xx + yy);

	return ret;
}

inline Quat QuatNormalize(const Quat& q)
{
	float len = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	if (len < 1e-8f)
		return Quat(0.f, 0.f, 0.f, 1.f);
	float invlen = 1.f / len;

	return Quat(q.x * invlen, q.y * invlen, q.z * invlen, q.w * invlen);
}

inline Quat QuatSlerp(Quat a, Quat b, float t)
{
	float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

	if (dot < 0.f)
	{
		b = Quat(-b.x, -b.y, -b.z, -b.w);
		dot = -dot;
	}

	if (dot > 0.999f)
	{
		Quat ret(
			a.x + t * (b.x - a.x),
			a.y + t * (b.y - a.y),
			a.z + t * (b.z - a.z),
			a.w + t * (b.w - a.w)
		);

		return QuatNormalize(ret);
	}

	float theta = acosf(dot);
	float sintheta = sinf(theta);

	float ra = sinf((1 - t) * theta) / sintheta;
	float rb = sinf(t * theta) / sintheta;

	return Quat(ra * a.x + rb * b.x,
		ra * a.y + rb * b.y,
		ra * a.z + rb * b.z,
		ra * a.w + rb * b.w
	);
}

// YXZ order (legacy)
inline Quat QuatFromEuler(float pitchDeg, float yawDeg, float rollDeg)
{
	float px = pitchDeg * TO__RADIAN;
	float py = yawDeg * TO__RADIAN;
	float pz = rollDeg * TO__RADIAN;

	Quat qx = QuatFromAxisAngle(AXIS__X, px);
	Quat qy = QuatFromAxisAngle(AXIS__Y, py);
	Quat qz = QuatFromAxisAngle(AXIS__Z, pz);

	return qy * qx * qz;
}

// Matches DirectXMath row-major: v' = v * Rx * Ry * Rz
// Row-major matrix multiply order → quaternion reverse order: qz * qy * qx
// Equivalent to GameEngineLib's EulerToPhysicsQuat()
inline Quat QuatFromEulerXYZ(float pitchDeg, float yawDeg, float rollDeg)
{
	float px = pitchDeg * TO__RADIAN;
	float py = yawDeg * TO__RADIAN;
	float pz = rollDeg * TO__RADIAN;

	Quat qx = QuatFromAxisAngle(AXIS__X, px);
	Quat qy = QuatFromAxisAngle(AXIS__Y, py);
	Quat qz = QuatFromAxisAngle(AXIS__Z, pz);

	return qz * qy * qx;
}

// Mat3 ---------------------------------------------------------

inline Vec3 Mat3MultiplyVec3(const Mat3& m, const Vec3& v)
{
	return m * v;
}

inline Mat3 Mat3MultiplyMat3(const Mat3& m1, const Mat3& m2)
{
	return m1 * m2;
}

inline Mat3 Mat3Transpose(const Mat3& m)
{
	Mat3 matT;

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			matT.m[i][j] = m.m[j][i];

	return matT;
}

inline Mat3 Mat3Inverse(const Mat3& m)
{
	float c00 = m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1];
	float c01 = m.m[1][2] * m.m[2][0] - m.m[1][0] * m.m[2][2];
	float c02 = m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0];

	float c10 = m.m[0][2] * m.m[2][1] - m.m[0][1] * m.m[2][2];
	float c11 = m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0];
	float c12 = m.m[0][1] * m.m[2][0] - m.m[0][0] * m.m[2][1];

	float c20 = m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1];
	float c21 = m.m[0][2] * m.m[1][0] - m.m[0][0] * m.m[1][2];
	float c22 = m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];

	float det = m.m[0][0] * c00 + m.m[0][1] * c01 + m.m[0][2] * c02;
	assert(fabsf(det) > 1e-6f && "No InverseMat");

	float invDet = 1.f / det;

	Mat3 ret;
	ret.m[0][0] = c00 * invDet;
	ret.m[0][1] = c10 * invDet;
	ret.m[0][2] = c20 * invDet;

	ret.m[1][0] = c01 * invDet;
	ret.m[1][1] = c11 * invDet;
	ret.m[1][2] = c21 * invDet;

	ret.m[2][0] = c02 * invDet;
	ret.m[2][1] = c12 * invDet;
	ret.m[2][2] = c22 * invDet;

	return ret;
}

inline Mat3 Mat3Zero()
{
	Mat3 ret;

	ret.MakeZero();

	return ret;
}

inline Mat3 Mat3Identity()
{
	Mat3 ret;
	ret.MakeIdentity();

	return ret;
}