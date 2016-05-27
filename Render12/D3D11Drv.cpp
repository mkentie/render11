#include "stdafx.h"
#include "D3D11Drv.h"
#include "Helpers.h"

#pragma warning(push, 1)
IMPLEMENT_CLASS(UD3D12RenderDevice);
#pragma warning(pop)

UD3D12RenderDevice::UD3D12RenderDevice()
{

    URenderDevice::SpanBased = 0;
    URenderDevice::FullscreenOnly = 0;
    URenderDevice::SupportsFogMaps = 1;
    URenderDevice::SupportsTC = 1;
    URenderDevice::SupportsDistanceFog = 0;
    URenderDevice::SupportsLazyTextures = 0;
}

void UD3D12RenderDevice::StaticConstructor()
{

}

UBOOL UD3D12RenderDevice::Init(UViewport* const pInViewport, const INT iNewX, const INT iNewY, const INT iNewColorBytes, const UBOOL bFullscreen)
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

        auto& Device = m_Backend.GetDevice();
        auto& CommandList = m_Backend.GetCommandList();

        m_pGlobalShaderConstants = std::make_unique<GlobalShaderConstants>(Device, CommandList);
       // m_pDeviceState = std::make_unique<DeviceState>(Device, DeviceContext);
       // m_pTextureCache = std::make_unique<TextureCache>(Device, DeviceContext);
        m_pTileRenderer = std::make_unique<TileRenderer>(Device, m_pGlobalShaderConstants->GetRootSignature(), CommandList);
       // m_pGouraudRenderer = std::make_unique<GouraudRenderer>(Device, DeviceContext);
       // m_pComplexSurfaceRenderer = std::make_unique<ComplexSurfaceRenderer>(Device, DeviceContext);
    }
    catch (const WException& ex)
    {
        LOGWARNINGF(L"Exception: %s", ex.wwhat());
        OutputDebugString(ex.wwhat());
        return false;
    }

    //m_pDeviceState->BindSamplerStates();

    return true;
}

UBOOL UD3D12RenderDevice::SetRes(const INT iNewX, const INT iNewY, const INT iNewColorBytes, const UBOOL /*bFullscreen*/)
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

void UD3D12RenderDevice::Exit()
{

}

void UD3D12RenderDevice::Flush(const UBOOL /*bAllowPrecache*/)
{
	//m_pTextureCache->Flush();
}


void UD3D12RenderDevice::SetSceneNode(FSceneNode* const pFrame)
{
    assert(pFrame);
    m_Backend.SetViewport(*pFrame);
    m_pGlobalShaderConstants->SetSceneNode(*pFrame);
}


void UD3D12RenderDevice::Lock(const FPlane /*FlashScale*/, const FPlane /*FlashFog*/, const FPlane /*ScreenClear*/, const DWORD /*RenderLockFlags*/, BYTE* const /*pHitData*/, INT* const /*pHitSize*/)
{
    m_bNoTilesDrawnYet = true;
    const size_t iFrameIndex = m_Backend.NewFrame();
    m_pTileRenderer->NewFrame(iFrameIndex);
    //m_pGouraudRenderer->NewFrame();
    //m_pComplexSurfaceRenderer->NewFrame();
}

void UD3D12RenderDevice::Unlock(const UBOOL bBlit)
{
    
    Render();

    if (bBlit)
    {
        m_Backend.Present();
    }
}

void UD3D12RenderDevice::Render()
{
    //Check if something to render
//    if (!m_pTileRenderer->IsMapped() )
        //&& !m_pGouraudRenderer->IsMapped() && !m_pComplexSurfaceRenderer->IsMapped())
    {
     //   return;
    }

    m_pGlobalShaderConstants->Bind();
    //m_pDeviceState->Bind();
    //m_pTextureCache->BindTextures();

    if (m_pTileRenderer->InBatch())
    {
        m_pTileRenderer->StopBatch();
        m_pTileRenderer->Bind();
        m_pTileRenderer->Draw();
    }


    //if (m_pGouraudRenderer->IsMapped())
    //{
    //    m_pGouraudRenderer->Unmap();
    //    m_pGouraudRenderer->Bind();
    //    m_pGouraudRenderer->Draw();
    //}

    //if (m_pComplexSurfaceRenderer->IsMapped())
    //{
    //    m_pComplexSurfaceRenderer->Unmap();
    //    m_pComplexSurfaceRenderer->Bind();
    //    m_pComplexSurfaceRenderer->Draw();
    //}
}

void UD3D12RenderDevice::DrawComplexSurface(FSceneNode* const /*pFrame*/, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
    //assert(m_bNoTilesDrawnYet); //Want to be sure that tiles are the last thing to be drawn

    //const DWORD PolyFlags = Surface.PolyFlags;
    //const auto& BlendState = m_pDeviceState->GetBlendStateForPolyFlags(PolyFlags);
    //const auto& DepthStencilState = m_pDeviceState->GetDepthStencilStateForPolyFlags(PolyFlags);
    //if (!m_pDeviceState->IsBlendStatePrepared(BlendState) || !m_pDeviceState->IsDepthStencilStatePrepared(DepthStencilState) || m_pTileRenderer->IsMapped() || m_pGouraudRenderer->IsMapped())
    //{
    //    Render();
    //}

    //unsigned int TexFlags = 0;

    //const TextureConverter::TextureData* pTexDiffuse = nullptr;
    //if (Surface.Texture)
    //{
    //    if (!m_pTextureCache->IsPrepared(*Surface.Texture, 0))
    //    {
    //        Render();
    //    }
    //    pTexDiffuse = &m_pTextureCache->FindOrInsertAndPrepare(*Surface.Texture, 0);
    //    TexFlags |= 0x00000001;
    //}

    //const TextureConverter::TextureData* pTexLight = nullptr;
    //if (Surface.LightMap)
    //{

    //    if (Surface.LightMap->bRealtimeChanged)
    //    {
    //        m_LightMaps.push_back(Surface.LightMap->CacheID);
    //    }

    //    if (!m_pTextureCache->IsPrepared(*Surface.LightMap, 1))
    //    {
    //        Render();
    //    }
    //    pTexLight = &m_pTextureCache->FindOrInsertAndPrepare(*Surface.LightMap, 1);
    //    TexFlags |= 0x00000002;
    //}

    //m_pDeviceState->PrepareDepthStencilState(DepthStencilState);
    //m_pDeviceState->PrepareBlendState(BlendState);

    //if (!m_pComplexSurfaceRenderer->IsMapped())
    //{
    //    m_pComplexSurfaceRenderer->Map();
    //}

    ////Code from OpenGL renderer to calculate texture coordinates
    //const float UDot = Facet.MapCoords.XAxis | Facet.MapCoords.Origin;
    //const float VDot = Facet.MapCoords.YAxis | Facet.MapCoords.Origin;

    ////Draw each polygon
    //for (const FSavedPoly* pPoly = Facet.Polys; pPoly; pPoly = pPoly->Next)
    //{
    //    assert(pPoly);
    //    const FSavedPoly& Poly = *pPoly;
    //    if (Poly.NumPts < 3) //Skip invalid polygons
    //    {
    //        continue;
    //    }

    //    ComplexSurfaceRenderer::Vertex* const pVerts = m_pComplexSurfaceRenderer->GetTriangleFan(Poly.NumPts); //Reserve space and generate indices for fan		
    //    for (int i = 0; i < Poly.NumPts; i++)
    //    {
    //        ComplexSurfaceRenderer::Vertex& v = pVerts[i];

    //        //Code from OpenGL renderer to calculate texture coordinates
    //        const float U = Facet.MapCoords.XAxis | Poly.Pts[i]->Point;
    //        const float V = Facet.MapCoords.YAxis | Poly.Pts[i]->Point;
    //        const float UCoord = U - UDot;
    //        const float VCoord = V - VDot;

    //        //Diffuse texture coordinates
    //        v.TexCoords.x = (UCoord - Surface.Texture->Pan.X)*pTexDiffuse->fMultU;
    //        v.TexCoords.y = (VCoord - Surface.Texture->Pan.Y)*pTexDiffuse->fMultV;

    //        if (Surface.LightMap)
    //        {
    //            //Lightmaps require pan correction of -.5
    //            v.TexCoords1.x = (UCoord - (Surface.LightMap->Pan.X - 0.5f*Surface.LightMap->UScale))*pTexLight->fMultU;
    //            v.TexCoords1.y = (VCoord - (Surface.LightMap->Pan.Y - 0.5f*Surface.LightMap->VScale))*pTexLight->fMultV;
    //        }
    //        //if (Surface.DetailTexture)
    //        //{
    //        //    v->TexCoord[2].x = (UCoord - Surface.DetailTexture->Pan.X)*detail->multU;
    //        //    v->TexCoord[2].y = (VCoord - Surface.DetailTexture->Pan.Y)*detail->multV;
    //        //}
    //        //if (Surface.FogMap)
    //        //{
    //        //    //Fogmaps require pan correction of -.5
    //        //    v->TexCoord[3].x = (UCoord - (Surface.FogMap->Pan.X - 0.5f*Surface.FogMap->UScale))*fogMap->multU;
    //        //    v->TexCoord[3].y = (VCoord - (Surface.FogMap->Pan.Y - 0.5f*Surface.FogMap->VScale))*fogMap->multV;
    //        //}
    //        //if (Surface.MacroTexture)
    //        //{
    //        //    v->TexCoord[4].x = (UCoord - Surface.MacroTexture->Pan.X)*macro->multU;
    //        //    v->TexCoord[4].y = (VCoord - Surface.MacroTexture->Pan.Y)*macro->multV;
    //        //}

    //        static_assert(sizeof(Poly.Pts[i]->Point) >= sizeof(v.Pos), "Sizes differ, can't use reinterpret_cast");
    //        v.Pos = reinterpret_cast<decltype(v.Pos)&>(Poly.Pts[i]->Point);

    //        v.PolyFlags = PolyFlags;
    //        v.TexFlags = TexFlags;

    //    }

    //}
}

void UD3D12RenderDevice::DrawGouraudPolygon(FSceneNode* const /*pFrame*/, FTextureInfo& Info, FTransTexture** const ppPts, const int NumPts, const DWORD PolyFlags, FSpanBuffer* const /*pSpan*/)
{
    ////assert(m_bNoTilesDrawnYet); //Want to be sure that tiles are the last thing to be drawn -> doesn't hold for gouraud

    //if (NumPts < 3) //Degenerate triangle
    //{
    //    return;
    //}

    //const auto& BlendState = m_pDeviceState->GetBlendStateForPolyFlags(PolyFlags);
    //const auto& DepthStencilState = m_pDeviceState->GetDepthStencilStateForPolyFlags(PolyFlags);
    //if (!m_pDeviceState->IsBlendStatePrepared(BlendState) || !m_pDeviceState->IsDepthStencilStatePrepared(DepthStencilState) || !m_pTextureCache->IsPrepared(Info, 0) || m_pTileRenderer->IsMapped() || m_pComplexSurfaceRenderer->IsMapped())
    //{
    //    Render();
    //}

    //m_pDeviceState->PrepareDepthStencilState(DepthStencilState);
    //m_pDeviceState->PrepareBlendState(BlendState);
    //const TextureConverter::TextureData& texDiffuse = m_pTextureCache->FindOrInsertAndPrepare(Info, 0);

    //if (!m_pGouraudRenderer->IsMapped())
    //{
    //    m_pGouraudRenderer->Map();
    //}

    //GouraudRenderer::Vertex* const pVerts = m_pGouraudRenderer->GetTriangleFan(NumPts);
    //for (int i = 0; i < NumPts; i++) //Set fan verts
    //{
    //    GouraudRenderer::Vertex& v = pVerts[i];

    //    static_assert(sizeof(ppPts[i]->Point) >= sizeof(v.Pos), "Sizes differ, can't use reinterpret_cast");
    //    v.Pos = reinterpret_cast<decltype(v.Pos)&>(ppPts[i]->Point);

    //    static_assert(sizeof(ppPts[i]->Light) >= sizeof(v.Color), "Sizes differ, can't use reinterpret_cast");
    //    v.Color = reinterpret_cast<decltype(v.Color)&>(ppPts[i]->Light);

    //    v.TexCoords.x = ppPts[i]->U * texDiffuse.fMultU;
    //    v.TexCoords.y = ppPts[i]->V * texDiffuse.fMultV;

    //    v.PolyFlags = PolyFlags;

    //  //  v->Fog = *(Vec4*)&Pts[i]->Fog.X;


    //}

}

void UD3D12RenderDevice::DrawTile(FSceneNode* const /*pFrame*/, FTextureInfo& Info, const FLOAT fX, const FLOAT fY, const FLOAT fXL, const FLOAT fYL, const FLOAT fU, const FLOAT fV, const FLOAT fUL, const FLOAT fVL, FSpanBuffer* const /*pSpan*/, const FLOAT fZ, const FPlane Color, const FPlane /*Fog*/, const DWORD PolyFlags)
{
    assert(m_pTileRenderer);
    //assert(m_pTextureCache);
  //  assert(m_pDeviceState);
    m_bNoTilesDrawnYet = false;

    const DWORD PolyFlagsCorrected = (PolyFlags & (PF_Translucent | PF_Masked)) != PF_Masked ? PolyFlags ^ PF_Masked : PolyFlags; //Translucent has precedence over masked

    const auto& BlendState = m_pDeviceState->GetBlendStateForPolyFlags(PolyFlagsCorrected);

    //Flush state
    //if (!m_pDeviceState->IsBlendStatePrepared(BlendState) || !m_pTextureCache->IsPrepared(Info, 0) || m_pGouraudRenderer->IsMapped() || m_pComplexSurfaceRenderer->IsMapped())
    //{
    //    Render();
    //}

    //m_pDeviceState->PrepareBlendState(BlendState);
    //m_pTextureCache->FindOrInsertAndPrepare(Info, 0);

    if (!m_pTileRenderer->InBatch())
    {
        m_pTileRenderer->StartBatch();
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

void UD3D12RenderDevice::Draw2DLine(FSceneNode* const /*pFrame*/, const FPlane /*Color*/, const DWORD /*LineFlags*/, const FVector /*P1*/, const FVector /*P2*/)
{

}

void UD3D12RenderDevice::Draw2DPoint(FSceneNode* const /*pFrame*/, const FPlane /*Color*/, const DWORD /*LineFlags*/, const FLOAT /*fX1*/, const FLOAT /*fY1*/, const FLOAT /*fX2*/, const FLOAT /*fY2*/, const FLOAT /*fZ*/)
{

}

void UD3D12RenderDevice::ClearZ(FSceneNode* const /*pFrame*/)
{

}

void UD3D12RenderDevice::PushHit(const BYTE* const /*pData*/, const INT /*iCount*/)
{

}

void UD3D12RenderDevice::PopHit(const INT /*iCount*/, const UBOOL /*bForce*/)
{

}



void UD3D12RenderDevice::GetStats(TCHAR* const /*pResult*/)
{
    //Buffer is only 128 chars, so we do our own thing
    assert(Viewport);
    assert(Viewport->Canvas);

    PrintFunc(L"Tiles | Buffer fill: %Iu/%Iu. Draw calls: %Iu.", m_pTileRenderer->GetNumTiles(), m_pTileRenderer->GetMaxTiles(), m_pTileRenderer->GetNumDraws());
    PrintFunc(L"Gouraud | Buffer fill: %Iu/%Iu. Draw calls: %Iu.", m_pGouraudRenderer->GetNumIndices(), m_pGouraudRenderer->GetMaxIndices(), m_pGouraudRenderer->GetNumDraws());
    PrintFunc(L"Complex | Buffer fill: %Iu/%Iu. Draw calls: %Iu.", m_pComplexSurfaceRenderer->GetNumIndices(), m_pComplexSurfaceRenderer->GetMaxIndices(), m_pComplexSurfaceRenderer->GetNumDraws());
    PrintFunc(L"TexCache | Num: %Iu.", m_pTextureCache->GetNumTextures());

    m_pTextureCache->PrintSizeHistogram(*Viewport->Canvas);
}

void UD3D12RenderDevice::ReadPixels(FColor* const /*pPixels*/)
{

}

UBOOL UD3D12RenderDevice::Exec(const TCHAR* const Cmd, FOutputDevice& Ar /*= *GLog*/)
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
