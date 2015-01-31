#include "stdafx.h"
#include "GlobalShaderConstants.h"

GlobalShaderConstants::GlobalShaderConstants(ID3D11Device& Device, ID3D11DeviceContext& DeviceContext)
:m_CBufPerFrame(Device, DeviceContext)
,m_fFov(0.0)
,m_iViewPortX(0)
,m_iViewPortY(0)
{

}

void GlobalShaderConstants::SetSceneNode(const FSceneNode& SceneNode)
{
    assert(SceneNode.Viewport);
    assert(SceneNode.Viewport->Actor);
    assert(reinterpret_cast<uintptr_t>(&m_CBufPerFrame.m_Data.ProjectionMatrix) % 16 == 0);
    
    //TODO: we don't need any of the SceneNode precalculated values, so remove calculation from render.dll
    if (SceneNode.Viewport->Actor->FovAngle == m_fFov && SceneNode.X == m_iViewPortX && SceneNode.Y == m_iViewPortY)
    {
        return;
    }

    //Create projection matrix with swapped near/far for better accuracy
    static const float fZNear = 32760.0f;
    static const float fZFar = 1.0f;

    const float fAspect = SceneNode.FX / SceneNode.FY;
    const float fFovVert = SceneNode.Viewport->Actor->FovAngle / fAspect * static_cast<float>(PI) / 180.0f;

    m_CBufPerFrame.m_Data.fRes[0] = SceneNode.FX;
    m_CBufPerFrame.m_Data.fRes[1] = SceneNode.FY;
    m_CBufPerFrame.m_Data.ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fFovVert, fAspect, fZNear, fZFar);

    m_CBufPerFrame.MarkAsDirty();
    m_fFov = SceneNode.Viewport->Actor->FovAngle;
    m_iViewPortX = SceneNode.X;
    m_iViewPortY = SceneNode.Y;
}

void GlobalShaderConstants::Bind()
{
    if (m_CBufPerFrame.IsDirty())
    {
        m_CBufPerFrame.Update();
    }

    m_CBufPerFrame.Bind(0);
}


