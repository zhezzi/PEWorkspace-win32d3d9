#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/Scene/Skeleton.h"
#include "DefaultAnimationSM.h"
#include "Light.h"

#include "PrimeEngine/GameObjectModel/Camera.h"

// Sibling/Children includes
#include "MeshInstance.h"
#include "MeshManager.h"
#include "SceneNode.h"
#include "CameraManager.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
namespace PE {
namespace Components{

PE_IMPLEMENT_CLASS1(MeshInstance, Component);

MeshInstance::MeshInstance(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
: Component(context, arena, hMyself)
, m_culledOut(false)
{
}

void MeshInstance::addDefaultComponents()
{
	Component::addDefaultComponents();
}

void MeshInstance::initFromFile(const char *assetName, const char *assetPackage,
		int &threadOwnershipMask)
{
	Handle h = m_pContext->getMeshManager()->getAsset(assetName, assetPackage, threadOwnershipMask);
	initFromRegisteredAsset(h);

	
	// Get aabb from mesh
	Mesh* pMesh = m_hAsset.getObject<Mesh>();
	if (pMesh)
	{
		// 调试信息，输出 Mesh 的 AABB 顶点信息
		for (int i = 0; i < 8; ++i)
		{
			//PEINFO("...Location: MeshInstance: pMesh->aabbVertices[%d]: %f %f %f\n", i, pMesh->aabbVertices[i].m_x, pMesh->aabbVertices[i].m_y, pMesh->aabbVertices[i].m_z);
		}
		//PEINFO("...Location: MeshInstance: aabbVertices..%f\n", pMesh->aabbVertices);
		getAABB(*pMesh);
	}
	else
	{
		//PEERROR("MeshInstance: Failed to get Mesh object from asset!");
	}
	
}

void MeshInstance::getAABB(Mesh& mesh) {
	for (int i = 0; i < 8; i++) {
		this->aabbVertices[i] = mesh.aabbVertices[i];
		//PEINFO("...Location: MeshInstance: aabbVertices..%f\n", aabbVertices);
	}

}

bool MeshInstance::hasSkinWeights()
{
	Mesh *pMesh = m_hAsset.getObject<Mesh>();
	return pMesh->m_hSkinWeightsCPU.isValid();
}

void MeshInstance::initFromRegisteredAsset(const PE::Handle &h)
{
	m_hAsset = h;
	//add this instance as child to Mesh so that skin knows what to draw
	static int allowedEvts[] = {0};
	m_hAsset.getObject<Component>()->addComponent(m_hMyself, &allowedEvts[0]);
}


}; // namespace Components
}; // namespace PE
