#pragma once
#include "Geometry.h"
#include "Material.h"
#include "BufferStruct.h"

namespace GameEngine
{
	// .mymesh 바이너리 포맷 헤더
	struct MeshFileHeader
	{
		char     magic[8];       // "MYMESH\0\0"
		uint32_t version;        // 1
		uint32_t sectionCount;
	};

	struct MeshSectionHeader
	{
		uint32_t vertexCount;
		uint32_t indexCount;
		char     matKey[128];    // MaterialManager 키
		float    minBound[3];    // AABB 최솟값
		float    maxBound[3];    // AABB 최댓값
	};

	class Model
	{
	public:
		struct MeshSection
		{
			Render::Geometry  geometry;
			Render::Material* material    = nullptr;  // m_materials 원소를 가리킴
			std::string       matKey;
			uint32_t          vertexCount = 0;
			uint32_t          indexCount  = 0;
		};

	public:
		Model()  = default;
		~Model() = default;

		Model(const Model&)            = delete;
		Model& operator=(const Model&) = delete;

		bool               Load(const std::string& path, ID3D11Device* pDevice);

		size_t             GetSectionCount() const        { return m_sections.size(); }
		MeshSection&       GetSection(size_t index)       { return m_sections[index]; }
		const MeshSection& GetSection(size_t index) const { return m_sections[index]; }

	private:
		std::vector<MeshSection>                       m_sections;
		std::vector<std::unique_ptr<Render::Material>> m_materials;  // 클론 수명 관리
	};
}
