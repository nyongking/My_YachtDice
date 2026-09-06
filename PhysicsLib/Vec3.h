#pragma once

struct Vec3
{
	union
	{
		struct { float x, y, z; };
		float v[3];
	};

	constexpr Vec3() noexcept : x(0.f), y(0.f), z(0.f) {}
	constexpr Vec3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
	constexpr Vec3(const Vec3& rhs) noexcept : x(rhs.x), y(rhs.y), z(rhs.z) {}

	Vec3 operator+(const Vec3& rhs) const {	return Vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
	Vec3 operator-(const Vec3& rhs) const { return Vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
	Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
	Vec3 operator/(float scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }

	Vec3& operator+=(const Vec3& rhs) 
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
	Vec3& operator-=(const Vec3& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}
	Vec3& operator*=(float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
	Vec3& operator/=(float scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}
	Vec3 operator-() const { return Vec3(-x, -y, -z); }

	void MakeZero() { x = y = z = 0.f; }
};