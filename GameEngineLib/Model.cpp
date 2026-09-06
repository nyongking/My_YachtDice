#include "GameEnginePch.h"
#include "Model.h"
#include "MaterialManager.h"
#include "GBufferMaterial.h"

#include <fstream>

namespace GameEngine
{
	bool Model::Load(const std::string& path, ID3D11Device* pDevice)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file)
			return false;

		// 헤더 검증
		MeshFileHeader header = {};
		file.read(reinterpret_cast<char*>(&header), sizeof(header));

		if (strncmp(header.magic, "MYMESH", 6) != 0)
			return false;
		if (header.version != 1)
			return false;

		m_sections.resize(header.sectionCount);
		m_materials.resize(header.sectionCount);

		for (uint32_t i = 0; i < header.sectionCount; ++i)
		{
			MeshSectionHeader secHeader = {};
			file.read(reinterpret_cast<char*>(&secHeader), sizeof(secHeader));

			// 정점/인덱스 읽기
			std::vector<Render::VTXPOSNORMTANUV> vertices(secHeader.vertexCount);
			std::vector<uint32_t>                indices(secHeader.indexCount);

			file.read(reinterpret_cast<char*>(vertices.data()),
					  sizeof(Render::VTXPOSNORMTANUV) * secHeader.vertexCount);
			file.read(reinterpret_cast<char*>(indices.data()),
					  sizeof(uint32_t) * secHeader.indexCount);

			if (!file)
				return false;

			// GPU 버퍼 생성
			if (!m_sections[i].geometry.CreateBuffers(pDevice, vertices, indices))
				return false;

			// 경량 메타데이터 저장 (vertices/indices는 스코프 종료 시 자동 해제)
			m_sections[i].matKey      = secHeader.matKey;
			m_sections[i].vertexCount = secHeader.vertexCount;
			m_sections[i].indexCount  = secHeader.indexCount;

			// MaterialManager에서 클론 가져오기; 미등록이면 GBufferMaterial 자동 등록
			m_materials[i] = MaterialManager::GetInstance()->Get(secHeader.matKey);
			if (!m_materials[i])
			{
				MaterialManager::GetInstance()->LoadSync(
					secHeader.matKey,
					std::make_unique<Render::GBufferMaterial>(),
					"GBuffer");
				m_materials[i] = MaterialManager::GetInstance()->Get(secHeader.matKey);
			}
			m_sections[i].material = m_materials[i].get();
		}

		return true;
	}
}
