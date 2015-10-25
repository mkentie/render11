#pragma once
#include "RenDevBackend.h"
#include "GlobalShaderConstants.h"

#include "TileRenderer.h" //Can't forward declare as DECLARE_CLASS implements destructor
#include "GouraudRenderer.h"
#include "ComplexSurfaceRenderer.h"
#include "DeviceState.h"
#include "TextureCache.h"

class UD3D11RenderDevice : public URenderDevice
{
#pragma warning(push, 1)
    DECLARE_CLASS(UD3D11RenderDevice, URenderDevice, CLASS_Config)
#pragma warning(pop)

public:
    explicit UD3D11RenderDevice();
    UD3D11RenderDevice(const UD3D11RenderDevice&) = delete;
    UD3D11RenderDevice& operator=(const UD3D11RenderDevice&) = delete;

    void StaticConstructor();

protected:
    void Render();

    //Convenience function so don't need to pass Viewport->...; template to pass varargs

    template<class... Args>
    void PrintFunc(Args... args)
    {
        assert(Viewport);
        assert(Viewport->Canvas);
        Viewport->Canvas->WrappedPrintf(Viewport->Canvas->SmallFont, 0, args...);
    }

    RenDevBackend m_Backend;
    std::unique_ptr<GlobalShaderConstants> m_pGlobalShaderConstants;
    std::unique_ptr<DeviceState> m_pDeviceState;
    std::unique_ptr<TileRenderer> m_pTileRenderer;
    std::unique_ptr<GouraudRenderer> m_pGouraudRenderer;
    std::unique_ptr<ComplexSurfaceRenderer> m_pComplexSurfaceRenderer;
    std::unique_ptr<TextureCache> m_pTextureCache;

    bool m_bNoTilesDrawnYet;

//From URenderDevice
public:
    virtual UBOOL Init(UViewport* const pInViewport, const INT iNewX, const INT iNewY, const INT iNewColorBytes, const UBOOL bFullscreen) override;
    virtual UBOOL SetRes(const INT iNewX, const INT iNewY, const INT iNewColorBytes, const UBOOL bFullscreen) override;
    virtual void Exit() override;

    virtual void Flush(const UBOOL bAllowPrecache) override;
    virtual void Lock(const FPlane FlashScale, const FPlane FlashFog, const FPlane ScreenClear, const DWORD RenderLockFlags, BYTE* const pHitData, INT* const pHitSize) override;
    virtual void Unlock(const UBOOL bBlit) override;
    virtual void DrawComplexSurface(FSceneNode* const pFrame, FSurfaceInfo& Surface, FSurfaceFacet& Facet) override;
    virtual void DrawGouraudPolygon(FSceneNode* const pFrame, FTextureInfo& Info, FTransTexture** const ppPts, const int NumPts, const DWORD PolyFlags, FSpanBuffer* const pSpan) override;
    virtual void DrawTile(FSceneNode* const pFrame, FTextureInfo& Info, const FLOAT fX, const FLOAT fY, const FLOAT fXL, const FLOAT fYL, const FLOAT fU, const FLOAT fV, const FLOAT fUL, const FLOAT fVL, FSpanBuffer* const pSpan, const FLOAT fZ, const FPlane Color, const FPlane Fog, const DWORD PolyFlags) override;
    virtual void Draw2DLine(FSceneNode* const pFrame, const FPlane Color, const DWORD LineFlags, const FVector P1, const FVector P2) override;
    virtual void Draw2DPoint(FSceneNode* const pFrame, const FPlane Color, const DWORD LineFlags, const FLOAT fX1, const FLOAT fY1, const FLOAT fX2, const FLOAT fY2, const FLOAT fZ) override;
    virtual void ClearZ(FSceneNode* const pFrame) override;
    virtual void PushHit(const BYTE* const pData, const INT iCount) override;
    virtual void PopHit(const INT iCount, const UBOOL bForce) override;

    /**
    pResult is a 128 character string.
    */
    virtual void GetStats(TCHAR* const pResult) override;

    virtual void ReadPixels(FColor* const pPixels) override;

    //virtual void EndFlash() override
    //virtual void DrawStats(FSceneNode* const pFrame) override;

    /**
    This optional function can be used to set the frustum and viewport parameters per scene change instead of per drawXXXX() call.
    \param Frame Contains various information with which to build frustum and viewport.
    \note Standard Z parameters: near 1, far 32760.
    */
    virtual void SetSceneNode(FSceneNode* const pFrame) override;
    //virtual void PrecacheTexture(FTextureInfo& Info, const DWORD PolyFlags) override {}

    virtual UBOOL Exec(const TCHAR* const Cmd, FOutputDevice& Ar = *GLog) override;
};
