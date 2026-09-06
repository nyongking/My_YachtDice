#include "RenderPch.h"
#include "RenderGlobal.h"

#include "RenderDevice.h"
#include "Renderer.h"
//#include "RenderResourceManager.h"
//#include "ConstantBufferManager.h"
//#include "ViewProjManager.h"
//#include "RenderDefaultRegistry.h"
#include "RenderPipeline.h"

namespace Render
{
	bool InitRender(bool window, UINT sizeX, UINT sizeY, HWND hwnd)
	{
		if (false == RenderDevice::GetInstance().Initialize())
			return false;

		if (false == Renderer::GetInstance().Initialize(window, sizeX, sizeY, hwnd))
			return false;

		//if (false == ConstantBufferManager::GetInstance().Initialize())
		//	return false;

		//if (false == RenderResourceManager::GetInstance().Initialize())
		//	return false;

	/*	if (false == RenderDefaultRegistry::GetInstance().RegisterDefaultRenderItems(
			RenderDevice::GetInstance().GetDevice()))
			return false;*/

		if (false == RenderPipeline::GetInstance().Initialize(sizeX, sizeY))
			return false;

		return true;
	}

}
