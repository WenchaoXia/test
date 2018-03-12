// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyCommonUtilsLibrary.h"
#include "MyRenderInterface.h"

#include "RenderUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "Blueprint/UserWidget.h"

#include "MyRenderUtilsLibrary.generated.h"


USTRUCT()
struct FMyFlipImageElemCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyFlipImageElemCpp()
    {
        m_pTexture = NULL;
        m_iFrameOccupy = 1;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "texture"))
    UTexture2D* m_pTexture;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "frame occupy", ClampMin = 1))
    int32 m_iFrameOccupy;
};

USTRUCT()
struct FMyFlipImageSettingsCpp
{
    GENERATED_USTRUCT_BODY()

public:

    FMyFlipImageSettingsCpp()
    {
        reset();
    };

    inline void reset()
    {
        m_aFlipImageElems.Reset();
        m_fFlipTime = 1 / 30;
        m_bMatchSize = false;
        m_iLoopNum = 1;
    };

    inline int32 getTotalFrameNum() const
    {
        int32 ret = 0;
        int32 l = m_aFlipImageElems.Num();
        for (int32 i = 0; i < l; i++) {
            ret += m_aFlipImageElems[i].m_iFrameOccupy;
            MY_VERIFY(m_aFlipImageElems[i].m_iFrameOccupy > 0);
            if (m_aFlipImageElems[i].m_pTexture == NULL) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("found a flip image elem with NULL texture, idx %d."), i);
            }
        }

        return ret;
    };

    UPROPERTY(EditAnywhere, meta = (DisplayName = "flip image elems"))
    TArray<FMyFlipImageElemCpp> m_aFlipImageElems;

    //the time one frame occupy
    UPROPERTY(EditAnywhere, meta = (DisplayName = "flip time"))
        float m_fFlipTime;

    UPROPERTY(EditAnywhere, meta = (DisplayName = "match size"))
        bool m_bMatchSize;

    //if <= 0, means loop for ever
    UPROPERTY(EditAnywhere, meta = (DisplayName = "loop num"))
        int32 m_iLoopNum;
};

UCLASS(BlueprintType)
class MYONLINECARDGAME_API UMyFlipImageSettingsAssetCpp : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, meta = (DisplayName = "settings"))
    FMyFlipImageSettingsCpp m_cSettings;
};


/**
* We can use UE4's flipbook as data containor, but that would increase package size about 20kb per image, which is bad for mobile game.
* So we use custom data struct.
* * No Children
*/
UCLASS()
class MYONLINECARDGAME_API UMyFlipImage : public UImage
{
    GENERATED_BODY()


public:

    UMyFlipImage() : Super()
    {
        m_pSettingsAsset = NULL;

        m_iIdxOfImageShowing = -1;
        m_iFrameOfImageShowing = 0;
        m_iLoopShowing = 0;

        m_bShowWhenFlipStart = false;
        m_bHideWhenFlipAllOver = false;
    };

    //DECLARE_DYNAMIC_DELEGATE_RetVal(int32, FOnFlipEnd);
    //@loopFinished means how many loops done, it should never <= 0 unless overspin
    DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnFlipEnd, int32, loopFinished, bool, isLastLoop);

    //when calling, it will ensure a image will be set immedietlly if expected one is not set.
    //if @loopTime is 0, it will use preset interval, otherwise interval will be calculated to match it
    //if @loopNum is 0, it means loop for ever
    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    void startImageFlip(float loopTime, int32 loopNum = 1, bool matchSize = false);

    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    void stopImageFlip();

    UFUNCTION(BlueprintPure, meta = (UnsafeDuringActorConstruction = "true"))
    bool isImageFlipping() const;

    UFUNCTION(BlueprintGetter)
    UMyFlipImageSettingsAssetCpp* getSettingsAsset() const
    {
        return m_pSettingsAsset;
    }

    //return true if new path is OK
    UFUNCTION(BlueprintSetter)
    void setSettingsAsset(UMyFlipImageSettingsAssetCpp* pSettingsAsset)
    {
        m_pSettingsAsset = pSettingsAsset;
    }

    //return errcode, 0 means no errro. @size will return the first flip image's size, or zero if settings is empty or invalid
    UFUNCTION(BlueprintCallable)
    int32 getImageSize(FVector2D& size) const;

    UPROPERTY(EditAnywhere, BlueprintSetter = setSettingsAsset, BlueprintGetter = getSettingsAsset, meta = (DisplayName = "settings Asset"))
    UMyFlipImageSettingsAssetCpp* m_pSettingsAsset;

    //return value is ignored, called on every step end
    UPROPERTY(EditAnywhere, meta = (IsBindableEvent = "True", DisplayName = "on flip end"))
    FOnFlipEnd OnFlipEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "show when FlipStart"))
    bool m_bShowWhenFlipStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "hide when FlipAllOver"))
    bool m_bHideWhenFlipAllOver;

protected:

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& e) override;
#endif

    void BeginDestroy() override;

    void loop();
    void tryNextFrame(int32 idxOfImage, int32 frameOfImage);

    FMyFlipImageSettingsCpp m_cSettingUsing; //copy of asset, allow modifying
    int32 m_iIdxOfImageShowing;
    int32 m_iFrameOfImageShowing;
    int32 m_iLoopShowing;

    FTimerHandle m_cLoopTimerHandle;
};


USTRUCT(BlueprintType)
struct FMyWithCurveUpdateStepDataWidgetBasicCpp : public FMyWithCurveUpdateStepDataBasicCpp
{
    GENERATED_USTRUCT_BODY()

public:
    FMyWithCurveUpdateStepDataWidgetBasicCpp() : Super()
    {
        m_sClassName = TEXT("FMyWithCurveUpdateStepDataWidgetBasicCpp");
        reset(true);
    };

    virtual ~FMyWithCurveUpdateStepDataWidgetBasicCpp()
    {
    };

    inline void reset(bool resetSubClassDataonly = false)
    {
        if (!resetSubClassDataonly) {
            Super::reset();
        }

        FWidgetTransform t;
        m_cWidgetTransformStart = m_cWidgetTransformEnd = t;
    };

    virtual FMyWithCurveUpdateStepDataWidgetBasicCpp* createOnHeap() override
    {
        return new FMyWithCurveUpdateStepDataWidgetBasicCpp();
    };

    virtual void copyContentFrom(const FMyWithCurveUpdateStepDataBasicCpp& other) override
    {
        const FMyWithCurveUpdateStepDataWidgetBasicCpp* pOther = StaticCast<const FMyWithCurveUpdateStepDataWidgetBasicCpp *>(&other);
        *this = *pOther;
    };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "render transform start"))
    FWidgetTransform m_cWidgetTransformStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "render transform end"))
    FWidgetTransform m_cWidgetTransformEnd;
};


struct FMyWithCurveUpdaterWidgetCpp : public FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataWidgetBasicCpp>
{

public:
    FMyWithCurveUpdaterWidgetCpp() : FMyWithCurveUpdaterTemplateCpp<FMyWithCurveUpdateStepDataWidgetBasicCpp>()
    {

    };

    virtual ~FMyWithCurveUpdaterWidgetCpp()
    {

    };

    inline void reset()
    {
        FMyWithCurveUpdaterBasicCpp::reset();
    };
};


//a widget support animation setup at runtime, since default widget's animation is static
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class MYONLINECARDGAME_API UMyUserWidgetWithCurveUpdaterCpp : public UUserWidget
{
    GENERATED_BODY()

public:
    UMyUserWidgetWithCurveUpdaterCpp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer)
    {
        m_cUpdater.m_cCommonUpdateDelegate.BindUObject(this, &UMyUserWidgetWithCurveUpdaterCpp::updaterOnCommonUpdate);
        m_cUpdater.m_cCommonFinishDelegete.BindUObject(this, &UMyUserWidgetWithCurveUpdaterCpp::updaterOnCommonFinish);
        m_cUpdater.m_cActivateTickDelegate.BindUObject(this, &UMyUserWidgetWithCurveUpdaterCpp::updaterActivateTick);

        m_bUpdaterNeedTick = false;

        m_bShowWhenActivated = false;
        m_bHideWhenInactivated = false;
    };

    virtual ~UMyUserWidgetWithCurveUpdaterCpp()
    {

    };

    UFUNCTION(BlueprintCallable)
    int32 updaterAddStepToTail(const FMyWithCurveUpdateStepDataWidgetBasicCpp& data)
    {
        return m_cUpdater.addStepToTail(data);
    };

    UFUNCTION(BlueprintCallable)
    void updaterClearSteps()
    {
        m_cUpdater.clearSteps();
    }

    inline FMyWithCurveUpdaterWidgetCpp& getUpdaterRef()
    {
        return m_cUpdater;
    };

protected:

    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    void updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector);
    void updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data);
    void updaterActivateTick(bool activate, FString debugString);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "show when activated"))
    bool m_bShowWhenActivated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "hide when Inactivated"))
    bool m_bHideWhenInactivated;

    FMyWithCurveUpdaterWidgetCpp m_cUpdater;
    bool m_bUpdaterNeedTick;
};


UCLASS(Blueprintable, HideCategories = (Collision, Rendering))
class MYONLINECARDGAME_API AMyTextureGenSuitBaseCpp : public AActor
{
    GENERATED_BODY()

public:

    AMyTextureGenSuitBaseCpp() : Super()
    {
        reset();
    };

    virtual ~AMyTextureGenSuitBaseCpp()
    {

    };

    void reset()
    {
        m_pSceneCapture2D = NULL;
        m_aTargetActors.Reset();
        m_pRenderTarget = NULL;
        m_pRenderMaterial = NULL;
        m_sRenderMaterialTextureParamName = TEXT("InBaseColor");
        m_pTempRenderTarget = NULL;
        m_pTempRenderMaterialInstance = NULL;
    };

    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    virtual bool checkSettings() const;

    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    int32 genPrepare();

    //return error code, and by default it have no mips. Caller should clear target package before calling, otherwise a rename of new texture will happen, and still return 0.
    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"))
    int32 genDo(FString newTextureName);

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "scene capture 2d"))
    class ASceneCapture2D* m_pSceneCapture2D;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "target actors"))
    TArray<AActor*> m_aTargetActors;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "render target"))
    UTextureRenderTarget2D* m_pRenderTarget;

    //the material to handle the generated render target datas, we will have alpha inversed in that, and the material should handle the case
    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "render material"))
    UMaterialInterface* m_pRenderMaterial;

    //will use it to set texture parameter to "render material"
    UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "render material texture param name"))
    FName m_sRenderMaterialTextureParamName;

    UPROPERTY(BlueprintReadOnly, Transient, meta = (DisplayName = "temp render target"))
    UTextureRenderTarget2D* m_pTempRenderTarget;

    UPROPERTY(BlueprintReadOnly, Transient, meta = (DisplayName = "temp render material instance"))
    UMaterialInstanceDynamic* m_pTempRenderMaterialInstance;
};


//Warn: we don't support multiple player screen in one client now!
UCLASS()
class UMyRenderUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    //Some feature like mips can only work whe the size met the size requirement, this function check if it mets
    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"), Category = "UMyRenderUtilsLibrary")
    static bool RenderTargetIsSizePowerOfTwo(UTextureRenderTarget2D* inTextureRenderTarget);

    //assume size is not power of two, so feature like mips will be disabled
    static UTexture2D* RenderTargetConstructTexture2DIgnoreSizeLimit(UTextureRenderTarget2D* inTextureRenderTarget, UObject* Outer, const FString& NewTexName, EObjectFlags InObjectFlags, uint32 Flags = CTF_Default, TArray<uint8>* AlphaOverride = NULL);

    //will try create one even if the size is not power of two
    UFUNCTION(BlueprintCallable, meta = (UnsafeDuringActorConstruction = "true"), Category = "UMyRenderUtilsLibrary")
    static UTexture2D* RenderTargetCreateStaticTexture2DTryBest(UTextureRenderTarget2D* inTextureRenderTarget, FString InName = "Texture", enum TextureCompressionSettings CompressionSettings = TC_Default, enum TextureMipGenSettings MipSettings = TMGS_FromTextureGroup);

    //return error code.
    //before calling, make sure widget is anchored in one point, and its offset right and bottom reflect the width and height.
    //@canvasSize in most case can be get by UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry() if canvas fill the screen, or specified manually if canvas is a "small" widget
    //@positionInCanvas coordinate is right as x positive, down as y positive in canvas space, mostly like screen position in UWidgetLayoutLibrary API
    UFUNCTION(BlueprintCallable, Category = "UMyRenderUtilsLibrary")
    static int32 getCenterPointPositionForWidgetInCanvasWithPointAnchor(const FVector2D& canvasSize, const UWidget* widgetUnderCanvas, FVector2D& positionInCanvas);

};