// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "MeshManager.h"
// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"

#include "PrimeEngine/Geometry/SkeletonCPU/SkeletonCPU.h"

#include "PrimeEngine/Scene/RootSceneNode.h"

#include "PrimeEngine/Scene/Mesh.h" // 确保包含 Mesh 类定义的头文件


#include "Light.h"

// Sibling/Children includes

#include "MeshInstance.h"
#include "Skeleton.h"
#include "SceneNode.h"
#include "DrawList.h"
#include "SH_DRAW.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"

namespace PE {
namespace Components{

PE_IMPLEMENT_CLASS1(MeshManager, Component);
MeshManager::MeshManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: Component(context, arena, hMyself)
	, m_assets(context, arena, 256)
{
}

PE::Handle MeshManager::getAsset(const char *asset, const char *package, int &threadOwnershipMask)
{
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "%s/%s", package, asset);
	
	int index = m_assets.findIndex(key);
	if (index != -1)
	{
		return m_assets.m_pairs[index].m_value;
	}
	Handle h;

	if (StringOps::endswith(asset, "skela"))
	{
		PE::Handle hSkeleton("Skeleton", sizeof(Skeleton));
		Skeleton *pSkeleton = new(hSkeleton) Skeleton(*m_pContext, m_arena, hSkeleton);
		pSkeleton->addDefaultComponents();

		pSkeleton->initFromFiles(asset, package, threadOwnershipMask);
		h = hSkeleton;
	}
	else if (StringOps::endswith(asset, "mesha"))
	{
		MeshCPU mcpu(*m_pContext, m_arena);
		mcpu.ReadMesh(asset, package, "");

		PE::Handle hMesh("Mesh", sizeof(Mesh));
		Mesh *pMesh = new(hMesh) Mesh(*m_pContext, m_arena, hMesh);
		pMesh->addDefaultComponents();
		pMesh->loadFromMeshCPU_needsRC(mcpu, threadOwnershipMask);


		//PEINFO("......Location:MeshManager: mcpu.min_vertex%f\n", mcpu.min_vertex);
		//PEINFO("......Location:MeshManager: mcpu.max_vertex%f\n", mcpu.max_vertex);
		// Calculate AABB once per mesh
		calculateAABB(mcpu, *pMesh);
		PEINFO("......Location:MeshManager: pMesh->aabbVertices%f\n", pMesh->aabbVertices);


#if PE_API_IS_D3D11
		// todo: work out how lods will work
		//scpu.buildLod();
#endif
        // generate collision volume here. or you could generate it in MeshCPU::ReadMesh()
        pMesh->m_performBoundingVolumeCulling = true; // will now perform tests for this mesh

		h = hMesh;
	}


	PEASSERT(h.isValid(), "Something must need to be loaded here");

	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
	return h;
}

void MeshManager::registerAsset(const PE::Handle &h)
{
	static int uniqueId = 0;
	++uniqueId;
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "__generated_%d", uniqueId);
	
	int index = m_assets.findIndex(key);
	PEASSERT(index == -1, "Generated meshes have to be unique");
	
	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
}

void MeshManager::calculateAABB(MeshCPU& mcpu, Mesh& mesh)
{
	// 获取最小和最大顶点坐标
	Vector3 minVertex = mcpu.min_vertex;
	Vector3 maxVertex = mcpu.max_vertex;
	// Printout min_vertex value and max_vertex value

	// 生成包围盒的 8 个顶点
	Vector3 aabbVertices[8];
	aabbVertices[0] = Vector3(minVertex.m_x, minVertex.m_y, minVertex.m_z); // 最小点
	aabbVertices[1] = Vector3(maxVertex.m_x, minVertex.m_y, minVertex.m_z);
	aabbVertices[2] = Vector3(minVertex.m_x, maxVertex.m_y, minVertex.m_z);
	aabbVertices[3] = Vector3(maxVertex.m_x, maxVertex.m_y, minVertex.m_z);
	aabbVertices[4] = Vector3(minVertex.m_x, minVertex.m_y, maxVertex.m_z);
	aabbVertices[5] = Vector3(maxVertex.m_x, minVertex.m_y, maxVertex.m_z);
	aabbVertices[6] = Vector3(minVertex.m_x, maxVertex.m_y, maxVertex.m_z);
	aabbVertices[7] = Vector3(maxVertex.m_x, maxVertex.m_y, maxVertex.m_z); // 最大点

	// 你可以选择在这里保存 AABB 信息到 mesh 的属性中
	for (int i = 0; i < 8; i++) {
		mesh.aabbVertices[i] = aabbVertices[i];
	}
}

}; // namespace Components
}; // namespace PE
