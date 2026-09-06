#pragma once
#include "Vec3.h"
#include <cstring>

struct Mat3
{
	union
	{
		struct { Vec3 row0, row1, row2; };
		Vec3 rows[3];
		float m[3][3];
	};

	Mat3() { memset(m, 0, sizeof(m)); }
	Mat3(const Mat3& rhs) { memcpy(m, rhs.m, sizeof(m)); }
	Mat3& operator=(const Mat3& rhs) { memcpy(m, rhs.m, sizeof(m)); return *this; }

	Mat3 operator*(const Mat3& rhs) const
	{
		Mat3 ret;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				ret.m[i][j] = 0;
				for (int k = 0; k < 3; ++k)
					ret.m[i][j] += m[i][k] * rhs.m[k][j];
			}
		}
		return ret;
	}

	Vec3 operator*(const Vec3& v) const
	{
		return Vec3(v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0],
			v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1],
			v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2]);
	}

	void MakeZero()
	{
		m[0][0] = 0.f; m[0][1] = 0.f; m[0][2] = 0.f;
		m[1][0] = 0.f; m[1][1] = 0.f; m[1][2] = 0.f;
		m[2][0] = 0.f; m[2][1] = 0.f; m[2][2] = 0.f;
	}

	void MakeIdentity()
	{
		m[0][0] = 1.f; m[0][1] = 0.f; m[0][2] = 0.f;
		m[1][0] = 0.f; m[1][1] = 1.f; m[1][2] = 0.f;
		m[2][0] = 0.f; m[2][1] = 0.f; m[2][2] = 1.f;
	}
};

