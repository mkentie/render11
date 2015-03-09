class URender11 : public URender
{
#pragma warning(push, 1)
DECLARE_CLASS(URender11, URender, CLASS_Config)
#pragma warning(pop)

public:
    explicit URender11();
    URender11(const URender11&) = delete;
    URender11& operator=(const URender11&) = delete;

    void StaticConstructor();

//From URenderBase
public:
    //virtual void Init(UEngine* InEngine) override;
// 	virtual void Destroy() override;
// 	UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar) override;
    virtual void PreRender(FSceneNode* Frame) override;
// 	virtual void PostRender(FSceneNode* Frame) override;
    virtual FSceneNode* CreateMasterFrame(UViewport* Viewport, FVector Location, FRotator Rotation, FScreenBounds* Bounds) override;
// 	virtual FSceneNode* CreateChildFrame(FSceneNode* Parent, FSpanBuffer* Span, ULevel* Level, INT iSurf, INT iZone, FLOAT Mirror, const FPlane& NearClip, const FCoords& Coords, FScreenBounds* Bounds) override;
// 	virtual void FinishMasterFrame() override;
//virtual void DrawWorld(FSceneNode* Frame) override;
// 	virtual void DrawActor(FSceneNode* Frame, AActor* Actor) override;
// 	virtual UBOOL Project(FSceneNode* Frame, const FVector &V, FLOAT &ScreenX, FLOAT &ScreenY, FLOAT* Scale) override;
// 	virtual UBOOL Deproject(FSceneNode* Frame, INT ScreenX, INT ScreenY, FVector& V) override;
// 	virtual UBOOL BoundVisible(FSceneNode* Frame, FBox* Bound, FSpanBuffer* SpanBuffer, FScreenBounds& Results) override;
// 	virtual void GetVisibleSurfs(UViewport* Viewport, TArray<INT>& iSurfs) override;
// 	virtual void GlobalLighting(UBOOL Realtime, AActor* Owner, FLOAT& Brightness, FPlane& Color) override;
// 	virtual void Precache(UViewport* Viewport) override;
// 	virtual void DrawCircle(FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector& Location, FLOAT Radius) override;
// 	virtual void DrawBox(FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector Min, FVector Max) override;
};