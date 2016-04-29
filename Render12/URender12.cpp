
#include "stdafx.h"
#include "URender12.h"
#include "D3D11Drv.h"
#include "Helpers.h"


#pragma warning(push, 1)
IMPLEMENT_PACKAGE(Render12);
IMPLEMENT_CLASS(URender12);
#pragma warning(pop)

URender12::URender12()
{
    int a = 5; a++;
}

void URender12::StaticConstructor()
{

}
// 
// void URender12::Init( UEngine* InEngine )
// {
// 	URenderBase::Init(InEngine);
// }
// 
// void URender12::Destroy()
// {
// 	URenderBase::Destroy();
// }
// 
// UBOOL URender12::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
// {
// 	return true;
// }
// 
void URender12::PreRender( FSceneNode* Frame )
{
    assert(Frame);
    assert(Frame->Viewport);
    assert(Frame->Viewport->RenDev);
    //Check if we recognize the renderer
    if (Frame->Viewport->RenDev->GetClass() != UD3D12RenderDevice::StaticClass())
    {
        LOGWARNINGF(L"Incompatible renderer: %s.", Frame->Viewport->RenDev->GetName());
    }

    URender::PreRender(Frame);
}
// 
// void URender12::PostRender( FSceneNode* Frame )
// {
// 
// }

FSceneNode* URender12::CreateMasterFrame( UViewport* Viewport, FVector Location, FRotator Rotation, FScreenBounds* Bounds )
{
    // Set base info.
    FSceneNode* Frame = new(GSceneMem)FSceneNode;
    Frame->Viewport = Viewport;
    Frame->X = Viewport->SizeX;
    Frame->Y = Viewport->SizeY;
    Frame->XB = 0;
    Frame->YB = 0;
    Frame->Level = Viewport->Actor->GetLevel();
    Frame->Parent = NULL;
    Frame->Sibling = NULL;
    Frame->Child = NULL;
    Frame->iSurf = INDEX_NONE;
    Frame->Recursion = 0;
    Frame->Mirror = 1.0;
    Frame->Recursion = 0;
    Frame->NearClip = FPlane(0, 0, 0, 0);
    Frame->Draw[0] = NULL;
    Frame->Draw[1] = NULL;
    Frame->Draw[2] = NULL;
    Frame->Sprite = NULL;
    Frame->Span = new(GSceneMem)FSpanBuffer;
    Frame->Span->AllocIndexForScreen(Viewport->SizeX, Viewport->SizeY, &GSceneMem);
    
    // Compute coords.
    Frame->ComputeRenderCoords(Location, Rotation);

    // Compute zone.
    Frame->ZoneNumber = Viewport->Actor->GetLevel()->Model->PointRegion(Viewport->Actor->GetLevel()->GetLevelInfo(), Frame->Coords.Origin).ZoneNumber;

    return Frame;
}
//
// FSceneNode* URender12::CreateChildFrame( FSceneNode* Parent, FSpanBuffer* Span, ULevel* Level, INT iSurf, INT iZone, FLOAT Mirror, const FPlane& NearClip, const FCoords& Coords, FScreenBounds* Bounds )
// {
// 	return nullptr;
// }
// 
// void URender12::FinishMasterFrame()
// {
// 
// }
// 


void DoShit(FSceneNode* const pFrame)
{
    for (FSceneNode* F = pFrame->Child; F; F = F->Sibling)
    {
        DoShit(F);
    }
    pFrame->Child = nullptr;
    pFrame->Sprite = nullptr;
    int i = 0;
    for (FDynamicSprite* pS = pFrame->Sprite; pS; pS = pS->RenderNext)
    {
        i++;
    }
}

 void URender12::DrawWorld( FSceneNode* const pFrame )
{
    DoShit(pFrame);
    pFrame->Child = nullptr;
    pFrame->Sprite = nullptr;
    URender::DrawWorld(pFrame);
    

}
// 
// void URender12::DrawActor( FSceneNode* Frame, AActor* Actor )
// {
// 
// }
// 
// UBOOL URender12::Project( FSceneNode* Frame, const FVector &V, FLOAT &ScreenX, FLOAT &ScreenY, FLOAT* Scale )
// {
// 	return true;
// }
// 
// UBOOL URender12::Deproject( FSceneNode* Frame, INT ScreenX, INT ScreenY, FVector& V )
// {
// 	return true;
// }
// 
// UBOOL URender12::BoundVisible( FSceneNode* Frame, FBox* Bound, FSpanBuffer* SpanBuffer, FScreenBounds& Results )
// {
// 	return true;
// }
// 
// void URender12::GetVisibleSurfs( UViewport* Viewport, TArray<INT>& iSurfs )
// {
// 
// }
// 
// void URender12::GlobalLighting( UBOOL Realtime, AActor* Owner, FLOAT& Brightness, FPlane& Color )
// {
// 
// }
// 
// void URender12::Precache( UViewport* Viewport )
// {
// 
// }
// 
// void URender12::DrawCircle( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector& Location, FLOAT Radius )
// {
// 
// }
// 
// void URender12::DrawBox( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector Min, FVector Max )
// {
// 
// }
// 
// 
