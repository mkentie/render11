ThisClass
ThisClass
#include "stdafx.h"
#include "URender11.h"
#include "D3D11Drv.h"
#include "Helpers.h"


#pragma warning(push, 1)
IMPLEMENT_PACKAGE(Render11);
IMPLEMENT_CLASS(URender11);
#pragma warning(pop)

URender11::URender11()
{
    int a = 5; a++;
}

void URender11::StaticConstructor()
{

}
// 
// void URender11::Init( UEngine* InEngine )
// {
// 	URenderBase::Init(InEngine);
// }
// 
// void URender11::Destroy()
// {
// 	URenderBase::Destroy();
// }
// 
// UBOOL URender11::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
// {
// 	return true;
// }
// 
void URender11::PreRender( FSceneNode* Frame )
{
    assert(Frame);
    assert(Frame->Viewport);
    assert(Frame->Viewport->RenDev);
    //Check if we recognize the renderer
    if (Frame->Viewport->RenDev->GetClass() != UD3D11RenderDevice::StaticClass())
    {
        LOGWARNINGF(L"Incompatible renderer: %s.", Frame->Viewport->RenDev->GetName());
    }

    URender::PreRender(Frame);
}
// 
// void URender11::PostRender( FSceneNode* Frame )
// {
// 
// }
// 
// FSceneNode* URender11::CreateMasterFrame( UViewport* Viewport, FVector Location, FRotator Rotation, FScreenBounds* Bounds )
// {
// 	// Set base info.
// 	FSceneNode* pFrame	= new FSceneNode;
// 	pFrame->Viewport	= Viewport;
// 	pFrame->Level		= Viewport->Actor->XLevel;
// 	pFrame->Parent		= nullptr;
// 	pFrame->Sibling		= nullptr;
// 	pFrame->iSurf		= INDEX_NONE;
// 	pFrame->Recursion	= 0;
// 	pFrame->Mirror		= 1.0;
// 	pFrame->NearClip	= FPlane(0,0,0,0);
// 	pFrame->Span		= nullptr;
// 	pFrame->Draw[0]		= nullptr;
// 	pFrame->Draw[1]		= nullptr;
// 	pFrame->Draw[2]		= nullptr;
// 	pFrame->Sprite		= nullptr;
// 	pFrame->X			= Viewport->SizeX;
// 	pFrame->Y			= Viewport->SizeY;
// 	pFrame->XB			= 0;
// 	pFrame->YB			= 0;
// 
// 	// Fills in precomputed members
// 	pFrame->ComputeRenderCoords( Location, Rotation );
// 
// 	pFrame->ZoneNumber = Viewport->Actor->XLevel->Model->PointRegion( Viewport->Actor->XLevel->GetLevelInfo(), pFrame->Coords.Origin ).ZoneNumber;
// 
// 	return pFrame;
// }
// 
// FSceneNode* URender11::CreateChildFrame( FSceneNode* Parent, FSpanBuffer* Span, ULevel* Level, INT iSurf, INT iZone, FLOAT Mirror, const FPlane& NearClip, const FCoords& Coords, FScreenBounds* Bounds )
// {
// 	return nullptr;
// }
// 
// void URender11::FinishMasterFrame()
// {
// 
// }
// 
// void URender11::DrawWorld( FSceneNode* const pFrame )
// {
//  
// }
// 
// void URender11::DrawActor( FSceneNode* Frame, AActor* Actor )
// {
// 
// }
// 
// UBOOL URender11::Project( FSceneNode* Frame, const FVector &V, FLOAT &ScreenX, FLOAT &ScreenY, FLOAT* Scale )
// {
// 	return true;
// }
// 
// UBOOL URender11::Deproject( FSceneNode* Frame, INT ScreenX, INT ScreenY, FVector& V )
// {
// 	return true;
// }
// 
// UBOOL URender11::BoundVisible( FSceneNode* Frame, FBox* Bound, FSpanBuffer* SpanBuffer, FScreenBounds& Results )
// {
// 	return true;
// }
// 
// void URender11::GetVisibleSurfs( UViewport* Viewport, TArray<INT>& iSurfs )
// {
// 
// }
// 
// void URender11::GlobalLighting( UBOOL Realtime, AActor* Owner, FLOAT& Brightness, FPlane& Color )
// {
// 
// }
// 
// void URender11::Precache( UViewport* Viewport )
// {
// 
// }
// 
// void URender11::DrawCircle( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector& Location, FLOAT Radius )
// {
// 
// }
// 
// void URender11::DrawBox( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector Min, FVector Max )
// {
// 
// }
// 
// 
