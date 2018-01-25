// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RenderUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "MyRenderUtilsLibrary.generated.h"

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
};