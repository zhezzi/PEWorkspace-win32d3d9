#include "CameraSceneNode.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/DebugRenderer.h"

#define Z_ONLY_CAM_BIAS 0.0f
namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(CameraSceneNode, SceneNode);

CameraSceneNode::CameraSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself) : SceneNode(context, arena, hMyself)
{
	m_near = 0.05f;
	m_far = 2000.0f;
}
void CameraSceneNode::addDefaultComponents()
{
	Component::addDefaultComponents();
	PE_REGISTER_EVENT_HANDLER(Events::Event_CALCULATE_TRANSFORMATIONS, CameraSceneNode::do_CALCULATE_TRANSFORMATIONS);
}

void CameraSceneNode::do_CALCULATE_TRANSFORMATIONS(Events::Event *pEvt)
{
	Handle hParentSN = getFirstParentByType<SceneNode>();
	if (hParentSN.isValid())
	{
		Matrix4x4 parentTransform = hParentSN.getObject<PE::Components::SceneNode>()->m_worldTransform;
		m_worldTransform = parentTransform * m_base;
	}
	
	Matrix4x4 &mref_worldTransform = m_worldTransform;

	Vector3 pos = Vector3(mref_worldTransform.m[0][3], mref_worldTransform.m[1][3], mref_worldTransform.m[2][3]);
	Vector3 n = Vector3(mref_worldTransform.m[0][2], mref_worldTransform.m[1][2], mref_worldTransform.m[2][2]);
	Vector3 target = pos + n;
	Vector3 up = Vector3(mref_worldTransform.m[0][1], mref_worldTransform.m[1][1], mref_worldTransform.m[2][1]);

	m_worldToViewTransform = CameraOps::CreateViewMatrix(pos, target, up);

	m_worldTransform2 = mref_worldTransform;

	m_worldTransform2.moveForward(Z_ONLY_CAM_BIAS);

	Vector3 pos2 = Vector3(m_worldTransform2.m[0][3], m_worldTransform2.m[1][3], m_worldTransform2.m[2][3]);
	Vector3 n2 = Vector3(m_worldTransform2.m[0][2], m_worldTransform2.m[1][2], m_worldTransform2.m[2][2]);
	Vector3 target2 = pos2 + n2;
	Vector3 up2 = Vector3(m_worldTransform2.m[0][1], m_worldTransform2.m[1][1], m_worldTransform2.m[2][1]);

	m_worldToViewTransform2 = CameraOps::CreateViewMatrix(pos2, target2, up2);
    
    PrimitiveTypes::Float32 aspect = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getWidth()) / (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getHeight());
    
    PrimitiveTypes::Float32 verticalFov = 0.33f * PrimitiveTypes::Constants::c_Pi_F32;
    if (aspect < 1.0f)
    {
        //ios portrait view
        static PrimitiveTypes::Float32 factor = 0.5f;
        verticalFov *= factor;
    }

	m_viewToProjectedTransform = CameraOps::CreateProjectionMatrix(verticalFov, 
		aspect,
		m_near, m_far);

    // Calculate model matrix = view * projection
    Matrix4x4 vpMatrix = m_viewToProjectedTransform * m_worldToViewTransform;
    m_cameraWorld = m_worldToViewTransform.inverse();
   

    // Left plane (line 0)
    m_frustumPlanesMatrix[0].m[0][0] = vpMatrix.m[0][3] + vpMatrix.m[0][0];
    m_frustumPlanesMatrix[0].m[0][1] = vpMatrix.m[1][3] + vpMatrix.m[1][0];
    m_frustumPlanesMatrix[0].m[0][2] = vpMatrix.m[2][3] + vpMatrix.m[2][0];
    m_frustumPlanesMatrix[0].m[0][3] = vpMatrix.m[3][3] + vpMatrix.m[3][0];

    // Right plane (line 1)
    m_frustumPlanesMatrix[1].m[0][0] = vpMatrix.m[0][3] - vpMatrix.m[0][0];
    m_frustumPlanesMatrix[1].m[0][1] = vpMatrix.m[1][3] - vpMatrix.m[1][0];
    m_frustumPlanesMatrix[1].m[0][2] = vpMatrix.m[2][3] - vpMatrix.m[2][0];
    m_frustumPlanesMatrix[1].m[0][3] = vpMatrix.m[3][3] - vpMatrix.m[3][0];

    // Bottom plane (line 2 )
    m_frustumPlanesMatrix[2].m[0][0] = vpMatrix.m[0][3] + vpMatrix.m[0][1];
    m_frustumPlanesMatrix[2].m[0][1] = vpMatrix.m[1][3] + vpMatrix.m[1][1];
    m_frustumPlanesMatrix[2].m[0][2] = vpMatrix.m[2][3] + vpMatrix.m[2][1];
    m_frustumPlanesMatrix[2].m[0][3] = vpMatrix.m[3][3] + vpMatrix.m[3][1];

    // Top plane (line 3)
    m_frustumPlanesMatrix[3].m[0][0] = vpMatrix.m[0][3] - vpMatrix.m[0][1];
    m_frustumPlanesMatrix[3].m[0][1] = vpMatrix.m[1][3] - vpMatrix.m[1][1];
    m_frustumPlanesMatrix[3].m[0][2] = vpMatrix.m[2][3] - vpMatrix.m[2][1];
    m_frustumPlanesMatrix[3].m[0][3] = vpMatrix.m[3][3] - vpMatrix.m[3][1];

    // Near plane (line 4)
    m_frustumPlanesMatrix[4].m[0][0] = vpMatrix.m[0][3] + vpMatrix.m[0][2];
    m_frustumPlanesMatrix[4].m[0][1] = vpMatrix.m[1][3] + vpMatrix.m[1][2];
    m_frustumPlanesMatrix[4].m[0][2] = vpMatrix.m[2][3] + vpMatrix.m[2][2];
    m_frustumPlanesMatrix[4].m[0][3] = vpMatrix.m[3][3] + vpMatrix.m[3][2];

    // Far plane (line 5)
    m_frustumPlanesMatrix[5].m[0][0] = vpMatrix.m[0][3] - vpMatrix.m[0][2];
    m_frustumPlanesMatrix[5].m[0][1] = vpMatrix.m[1][3] - vpMatrix.m[1][2];
    m_frustumPlanesMatrix[5].m[0][2] = vpMatrix.m[2][3] - vpMatrix.m[2][2];
    m_frustumPlanesMatrix[5].m[0][3] = vpMatrix.m[3][3] - vpMatrix.m[3][2];

    // Normalize
    for (int i = 0; i < 6; i++) {
        float length = sqrtf(m_frustumPlanesMatrix[i].m[0][0] * m_frustumPlanesMatrix[i].m[0][0] +
                             m_frustumPlanesMatrix[i].m[0][1] * m_frustumPlanesMatrix[i].m[0][1] +
                             m_frustumPlanesMatrix[i].m[0][2] * m_frustumPlanesMatrix[i].m[0][2]);

        m_frustumPlanesMatrix[i].m[0][0] /= length;
        m_frustumPlanesMatrix[i].m[0][1] /= length;
        m_frustumPlanesMatrix[i].m[0][2] /= length;
        m_frustumPlanesMatrix[i].m[0][3] /= length;
    }

  
	// Call the parent class's transformation logic
	SceneNode::do_CALCULATE_TRANSFORMATIONS(pEvt);

}


}; // namespace Components
}; // namespace PE
