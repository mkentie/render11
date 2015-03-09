#include "stdafx.h"
#include "D3D11Drv.h"
#include "Helpers.h"

#pragma warning(push, 1)
IMPLEMENT_CLASS(UD3D11RenderDevice);
#pragma warning(pop)

UD3D11RenderDevice::UD3D11RenderDevice()
{
    URenderDevice::SpanBased = 0;
    URenderDevice::FullscreenOnly = 0;
    URenderDevice::SupportsFogMaps = 1;
    URenderDevice::SupportsTC = 1;
    URenderDevice::SupportsDistanceFog = 0;
    URenderDevice::SupportsLazyTextures = 0;
}

void UD3D11RenderDevice::StaticConstructor()
{

}

UBOOL UD3D11RenderDevice::Init(UViewport* const pInViewport, const INT iNewX, const INT iNewY, const INT iNewColorBytes, const UBOOL bFullscreen)
{
    assert(pInViewport);

#ifdef _DEBUG
    pInViewport->Exec(L"ShowLog");
#endif

    LOGMESSAGE(L"Initializing Direct3D 11 Renderer.");

    try
    {
        if (!m_Backend.Init(static_cast<HWND>(pInViewport->GetWindow())))
        {
            LOGWARNING(L"Failed to initialize Direct3D 11 Renderer.");
            return false;
        }

        URenderDevice::Viewport = pInViewport;
        if (!SetRes(iNewX, iNewY, iNewColorBytes, bFullscreen))
        {
            LOGWARNING(L"Failed to set resolution during Init().");
            return false;
        }

        m_pGlobalShaderConstants = std::make_unique<GlobalShaderConstants>(m_Backend.GetDevice(), m_Backend.GetDeviceContext());
        m_pDeviceState = std::make_unique<DeviceState>(m_Backend.GetDevice(), m_Backend.GetDeviceContext());
        m_pTextureCache = std::make_unique<TextureCache>(m_Backend.GetDevice(), m_Backend.GetDeviceContext());
        m_pTileRenderer = std::make_unique<TileRenderer>(m_Backend.GetDevice(), m_Backend.GetDeviceContext());
        m_pGouraudRenderer = std::make_unique<GouraudRenderer>(m_Backend.GetDevice(), m_Backend.GetDeviceContext());
    }
    catch (const WException& ex)
    {
        LOGWARNINGF(L"Exception: %s", ex.wwhat());
        return false;
    }

    m_pDeviceState->BindSamplerStates();

    return true;
}

UBOOL UD3D11RenderDevice::SetRes(const INT iNewX, const INT iNewY, const INT iNewColorBytes, const UBOOL /*bFullscreen*/)
{
    assert(URenderDevice::Viewport);

    //Without BLIT_HardwarePaint, game doesn't trigger us when resizing
    //Without BLIT_Direct3D renderer only ever gets one draw call, and SetRes() isn't called on window resize
    if (!URenderDevice::Viewport->ResizeViewport(EViewportBlitFlags::BLIT_HardwarePaint | EViewportBlitFlags::BLIT_Direct3D, iNewX, iNewY, iNewColorBytes))
    {
        LOGWARNINGF(L"Viewport resize failed (%d x %d).", iNewX, iNewY);
        return false;
    }

    try
    {
        m_Backend.SetRes(iNewX, iNewY);
    }
    catch (const WException& ex)
    {
        LOGWARNINGF(L"Exception: %s", ex.wwhat());
        return false;
    }

    return true;
}

void UD3D11RenderDevice::Exit()
{

}

void UD3D11RenderDevice::Flush(const UBOOL /*bAllowPrecache*/)
{

}

void UD3D11RenderDevice::SetSceneNode(FSceneNode* const pFrame)
{
    assert(pFrame);
    m_Backend.SetViewport(*pFrame);
    m_pGlobalShaderConstants->SetSceneNode(*pFrame);
}


void UD3D11RenderDevice::Lock(const FPlane /*FlashScale*/, const FPlane /*FlashFog*/, const FPlane /*ScreenClear*/, const DWORD /*RenderLockFlags*/, BYTE* const /*pHitData*/, INT* const /*pHitSize*/)
{
    m_bNoTilesDrawnYet = true;
    m_Backend.NewFrame();
    m_pTileRenderer->NewFrame();
    m_pGouraudRenderer->NewFrame();
}

void UD3D11RenderDevice::Unlock(const UBOOL bBlit)
{
    Render();

    if (bBlit)
    {
        m_Backend.Present();
    }
}

void UD3D11RenderDevice::Render()
{
    //Check if something to render
    if (!m_pTileRenderer->IsMapped() && !m_pGouraudRenderer->IsMapped())
    {
        return;
    }

    m_pGlobalShaderConstants->Bind();
    m_pDeviceState->Bind();
    m_pTextureCache->BindTextures();

    if (m_pTileRenderer->IsMapped())
    {
        m_pTileRenderer->Unmap();
        m_pTileRenderer->Bind();
        m_pTileRenderer->Draw();
    }

    if (m_pGouraudRenderer->IsMapped())
    {
        m_pGouraudRenderer->Unmap();
        m_pGouraudRenderer->Bind();
        m_pGouraudRenderer->Draw();
    }
}

void UD3D11RenderDevice::DrawComplexSurface(FSceneNode* const /*pFrame*/, FSurfaceInfo& /*Surface*/, FSurfaceFacet& /*Facet*/)
{
    assert(m_bNoTilesDrawnYet); //Want to be sure that tiles are the last thing to be drawn
}

void UD3D11RenderDevice::DrawGouraudPolygon(FSceneNode* const /*pFrame*/, FTextureInfo& Info, FTransTexture** const ppPts, const int NumPts, const DWORD PolyFlags, FSpanBuffer* const /*pSpan*/)
{
    //assert(m_bNoTilesDrawnYet); //Want to be sure that tiles are the last thing to be drawn -> doesn't hold for gouraud

    if (NumPts < 3) //Degenerate triangle
    {
        return;
    }

    const auto& BlendState = m_pDeviceState->GetBlendStateForPolyFlags(PolyFlags);
    const auto& DepthStencilState = m_pDeviceState->GetDepthStencilStateForPolyFlags(PolyFlags);
    if (!m_pDeviceState->IsBlendStatePrepared(BlendState) || !m_pDeviceState->IsDepthStencilStatePrepared(DepthStencilState) || !m_pTextureCache->IsPrepared(Info, 0) || m_pTileRenderer->IsMapped())
    {
        Render();
    }

    m_pDeviceState->PrepareDepthStencilState(DepthStencilState);
    m_pDeviceState->PrepareBlendState(BlendState);
    const TextureConverter::TextureData& texDiffuse = m_pTextureCache->FindOrInsertAndPrepare(Info, 0);

    if (!m_pGouraudRenderer->IsMapped())
    {
        m_pGouraudRenderer->Map();
    }

    GouraudRenderer::Vertex* const v = m_pGouraudRenderer->GetTriangleFan(NumPts);
    for (int i = 0; i < NumPts; i++) //Set fan verts
    {
        static_assert(sizeof(ppPts[i]->Point) >= sizeof(v[i].Pos), "Sizes differ, can't use reinterpret_cast");
        v[i].Pos = reinterpret_cast<decltype(v[i].Pos)&>(ppPts[i]->Point);

        static_assert(sizeof(ppPts[i]->Light) >= sizeof(v[i].Color), "Sizes differ, can't use reinterpret_cast");
        v[i].Color = reinterpret_cast<decltype(v[i].Color)&>(ppPts[i]->Light);

        v[i].TexCoords.x = ppPts[i]->U * texDiffuse.fMultU;
        v[i].TexCoords.y = ppPts[i]->V * texDiffuse.fMultV;

        v[i].PolyFlags = PolyFlags;

      //  v->Fog = *(Vec4*)&Pts[i]->Fog.X;


    }

}

void UD3D11RenderDevice::DrawTile(FSceneNode* const /*pFrame*/, FTextureInfo& Info, const FLOAT fX, const FLOAT fY, const FLOAT fXL, const FLOAT fYL, const FLOAT fU, const FLOAT fV, const FLOAT fUL, const FLOAT fVL, FSpanBuffer* const /*pSpan*/, const FLOAT fZ, const FPlane Color, const FPlane /*Fog*/, const DWORD PolyFlags)
{
    assert(m_pTileRenderer);
    assert(m_pTextureCache);
    assert(m_pDeviceState);
    m_bNoTilesDrawnYet = false;

    const DWORD PolyFlagsCorrected = (PolyFlags & (PF_Translucent | PF_Masked)) != PF_Masked ? PolyFlags ^ PF_Masked : PolyFlags; //Translucent has precedence over masked

    const auto& BlendState = m_pDeviceState->GetBlendStateForPolyFlags(PolyFlagsCorrected);

    //Flush state
    if (!m_pDeviceState->IsBlendStatePrepared(BlendState) || !m_pTextureCache->IsPrepared(Info, 0) || m_pGouraudRenderer->IsMapped())
    {
        Render();
    }

    m_pDeviceState->PrepareBlendState(BlendState);
    m_pTextureCache->FindOrInsertAndPrepare(Info, 0);

    if (!m_pTileRenderer->IsMapped())
    {
        m_pTileRenderer->Map();
    }

    TileRenderer::Tile& t = m_pTileRenderer->GetTile();

    t.XYPos.x = fX;
    t.XYPos.y = fX + fXL;
    t.XYPos.z = fY;
    t.XYPos.w = fY + fYL;

    t.TexCoord.x = fU / Info.Texture->USize;
    t.TexCoord.y = (fU + fUL) / Info.Texture->USize;
    t.TexCoord.z = fV / Info.Texture->VSize;
    t.TexCoord.w = (fV + fVL) / Info.Texture->VSize;

    //t.Z = fZ;
    static_assert(sizeof(Color) >= sizeof(t.Color), "Sizes differ, can't use reinterpret_cast");
    t.Color = reinterpret_cast<const decltype(t.Color)&>(Color);

    t.PolyFlags = PolyFlagsCorrected;
}

void UD3D11RenderDevice::Draw2DLine(FSceneNode* const /*pFrame*/, const FPlane /*Color*/, const DWORD /*LineFlags*/, const FVector /*P1*/, const FVector /*P2*/)
{

}

void UD3D11RenderDevice::Draw2DPoint(FSceneNode* const /*pFrame*/, const FPlane /*Color*/, const DWORD /*LineFlags*/, const FLOAT /*fX1*/, const FLOAT /*fY1*/, const FLOAT /*fX2*/, const FLOAT /*fY2*/, const FLOAT /*fZ*/)
{

}

void UD3D11RenderDevice::ClearZ(FSceneNode* const /*pFrame*/)
{

}

void UD3D11RenderDevice::PushHit(const BYTE* const /*pData*/, const INT /*iCount*/)
{

}

void UD3D11RenderDevice::PopHit(const INT /*iCount*/, const UBOOL /*bForce*/)
{

}



void UD3D11RenderDevice::GetStats(TCHAR* const pResult)
{
    //Buffer is only 128 chars, so we do our own thing
    assert(Viewport);
    assert(Viewport->Canvas);

    PrintFunc(L"Tiles | Buffer fill: %Iu/%Iu. Draw calls: %Iu.", m_pTileRenderer->GetNumTiles(), m_pTileRenderer->GetMaxTiles(), m_pTileRenderer->GetNumDraws());
    PrintFunc(L"Gouraud | Buffer fill: %Iu/%Iu. Draw calls: %Iu.", m_pGouraudRenderer->GetNumIndices(), m_pGouraudRenderer->GetMaxIndices(), m_pGouraudRenderer->GetNumDraws());
    PrintFunc(L"TexCache | Num: %Iu.", m_pTextureCache->GetNumTextures());

    m_pTextureCache->PrintSizeHistogram(*Viewport->Canvas);
}

void UD3D11RenderDevice::ReadPixels(FColor* const /*pPixels*/)
{

}

UBOOL UD3D11RenderDevice::Exec(const TCHAR* const Cmd, FOutputDevice& Ar /*= *GLog*/)
{
//     OutputDebugString(Cmd);
//     OutputDebugString(L"\n");

    if (wcscmp(Cmd, L"texsizehist") == 0)
    {
        assert(m_pTextureCache);
        //m_pTextureCache->PrintSizeHistogram();
    }

    return URenderDevice::Exec(Cmd, Ar);
}
