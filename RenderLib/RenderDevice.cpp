#include "RenderPch.h"
#include "RenderDevice.h"

namespace Render
{
	RenderDevice::RenderDevice()
	{

	}
	RenderDevice::~RenderDevice()
	{
	}

	bool RenderDevice::Initialize()
	{
		UINT32		Flag = 0;

	#ifdef _DEBUG
		Flag = D3D11_CREATE_DEVICE_DEBUG;
	#endif
		D3D_FEATURE_LEVEL			FeatureLV;

		if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, Flag, nullptr, 0, D3D11_SDK_VERSION, m_device.GetAddressOf(), &FeatureLV, m_context.GetAddressOf())))
			return false;

		return true;
	}
}
