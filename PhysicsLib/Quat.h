#pragma once
struct Quat
{
	float x, y, z, w;

	constexpr Quat() noexcept : x(0.f), y(0.f), z(0.f), w(1.f) {}
	constexpr Quat(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
	constexpr Quat(const Quat& rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {}

	Quat operator*(const Quat& rhs) const
	{
		return Quat(w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
			w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
			w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
			w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
	}

	Quat operator+(const Quat& rhs) const
	{
		return Quat(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	Quat operator*(float scalar) const
	{
		return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	Quat& operator+=(const Quat& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;

		return *this;
	}

	Quat& operator*=(float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;

		return *this;
	}
};

