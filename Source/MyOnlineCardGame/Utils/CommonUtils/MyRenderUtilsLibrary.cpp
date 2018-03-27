// Fill out your copyright notice in the Description page of Project Settings.

#include "MyRenderUtilsLibrary.h"

#include "MyCommonUtils.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Console.h"
#include "Engine/SceneCapture2D.h"

#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Runtime/UMG/Public/Blueprint/WidgetLayoutLibrary.h"

#include "Runtime/UMG/Public/Components/CanvasPanelSlot.h"
#include "Widgets/Input/SButton.h"
#include "Components/ButtonSlot.h"

//#include "Classes/PaperSprite.h"

#if WITH_EDITOR
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "PackageTools.h"
#endif

#if WITH_EDITOR

void UMyFlipImage::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyFlipImage, m_pSettingsAsset))
    {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty 1, this %p."), this);

        UTexture2D* pTexture = NULL;
        if (m_pSettingsAsset) {
            if (m_pSettingsAsset->m_cSettings.m_aFlipImageElems.Num() > 0) {
                pTexture = m_pSettingsAsset->m_cSettings.m_aFlipImageElems[0].m_pTexture;
                if (pTexture == NULL) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_aFlipImageElems[0].m_pTexture is NULL."));
                }
            }
            else {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT(".m_aFlipImageElems.Num() is 0."));
            }
        }
        else {
        }

        this->SetBrushFromTexture(pTexture);
    }

    Super::PostEditChangeProperty(e);
}

#endif

void UMyFlipImage::startImageFlip(float loopTime, int32 loopNum, bool matchSize)
{
    if (m_pSettingsAsset == NULL) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("settings not specified!"));
        return;
    }

    m_cSettingUsing = m_pSettingsAsset->m_cSettings;
    m_iIdxOfImageShowing = -1;
    m_iFrameOfImageShowing = 0;
    m_iLoopShowing = 0;

    int32 totalFrameNum = m_cSettingUsing.getTotalFrameNum();
    if (totalFrameNum <= 0) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not enough of frames specified: %d."), totalFrameNum);
        return;
    }

    //the time interval model is same as UE4's 3d flipbook
    if (FMath::IsNearlyEqual(loopTime, MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT)) {
    
    }
    else {
        m_cSettingUsing.m_fFlipTime = loopTime / totalFrameNum;
    }

    if (m_cSettingUsing.m_fFlipTime < MY_FLOAT_TIME_MIN_VALUE_TO_TAKE_EFFECT) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("flip time is too small: %f!"), m_cSettingUsing.m_fFlipTime);
    }

    m_cSettingUsing.m_iLoopNum = loopNum;
    m_cSettingUsing.m_bMatchSize = matchSize;

    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        world->GetTimerManager().SetTimer(m_cLoopTimerHandle, this, &UMyFlipImage::loop, m_cSettingUsing.m_fFlipTime, true);

        tryNextFrame(0, 0);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
    }

    //world->GetTimerManager().ClearTimer(m_cToCoreFullLoopTimerHandle);
    //world->GetTimerManager().SetTimer(m_cToCoreFullLoopTimerHandle, this, &AMyMJGameCoreDataSourceCpp::loop, ((float)MY_MJ_GAME_CORE_FULL_MAIN_THREAD_LOOP_TIME_MS) / (float)1000, true);
}

void UMyFlipImage::stopImageFlip()
{
    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
    }
}

bool UMyFlipImage::isImageFlipping() const
{
    UWorld *world = GetWorld();
    if (IsValid(world)) {
        return world->GetTimerManager().IsTimerActive(m_cLoopTimerHandle);

    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("world is invalid! Check settings!"));
        return false;
    }
};

int32 UMyFlipImage::getImageSize(FVector2D& size) const
{
    if (m_pSettingsAsset && m_pSettingsAsset->m_cSettings.m_aFlipImageElems.Num() > 0) {
        UTexture2D* pT = m_pSettingsAsset->m_cSettings.m_aFlipImageElems[0].m_pTexture;
        if (pT) {
            size.X = pT->GetSizeX();
            size.Y = pT->GetSizeY();
            return 0;
        }
    }

    size = FVector2D::ZeroVector;
    return -1;
}

void UMyFlipImage::BeginDestroy()
{
    Super::BeginDestroy();

    UWorld *world = GetWorld();
    if (IsValid(world)) {
        world->GetTimerManager().ClearTimer(m_cLoopTimerHandle);
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("BeginDestroy: debug, timer clear OK."));
    }
    else {
        if (this != GetClass()->GetDefaultObject()) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("BeginDestroy: world is invalid and object is not CDO! Check settings!"));
        }
    }
}

void UMyFlipImage::loop()
{
    //just go to next frame
    tryNextFrame(m_iIdxOfImageShowing, m_iFrameOfImageShowing + 1);
}

void UMyFlipImage::tryNextFrame(int32 idxOfImage, int32 frameOfImage)
{
    int32 elemL = m_cSettingUsing.m_aFlipImageElems.Num();
    MY_VERIFY(elemL > 0);

    const FMyFlipImageElemCpp* pElem = NULL;
    while (1)
    {
        if (idxOfImage >= 0 && idxOfImage < elemL) {
        }
        else {
            //out of range
            m_iLoopShowing++;

            bool isLastLoop = m_cSettingUsing.m_iLoopNum > 0 && m_iLoopShowing >= m_cSettingUsing.m_iLoopNum;
            OnFlipEnd.ExecuteIfBound(m_iLoopShowing, isLastLoop);

            if (isLastLoop) {
                stopImageFlip();
                if (m_bHideWhenFlipAllOver) {
                    this->SetVisibility(ESlateVisibility::Hidden);
                }
                return;
            }

            idxOfImage = 0;
            frameOfImage = 0;
        }

        pElem = &m_cSettingUsing.m_aFlipImageElems[idxOfImage];
        MY_VERIFY(pElem->m_iFrameOccupy > 0 || m_cSettingUsing.m_iLoopNum > 0); //avoid dead loop
        if (frameOfImage >= pElem->m_iFrameOccupy) {
            idxOfImage++;
            frameOfImage = 0;
            continue;
        }

        break;
    }

    MY_VERIFY(pElem);

    if (frameOfImage == 0) {
        //First frame
        if (pElem->m_pTexture) {
            SetBrushFromTexture(pElem->m_pTexture, m_cSettingUsing.m_bMatchSize);
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("found a flip image elem with NULL texture, idx %d."), idxOfImage);
        }

        if (m_bShowWhenFlipStart) {
            this->SetVisibility(ESlateVisibility::Visible);
        }
    }

    m_iIdxOfImageShowing = idxOfImage;
    m_iFrameOfImageShowing = frameOfImage;

    //pElem->m_pTexture->GetSizeX();
}


UMyButton::UMyButton(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    m_bEnableWidgetTransformPressed = false;
    m_cWidgetTransformPressed.Scale.X = m_cWidgetTransformPressed.Scale.Y = 0.8;
}

void UMyButton::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);

    FName PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyButton, RenderTransform))
    {
        m_cWidgetTransformNormal = RenderTransform;
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyButton, m_cWidgetTransformNormal))
    {
        RenderTransform = m_cWidgetTransformNormal;
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyButton, WidgetStyle))
    {
        if (WidgetStyle.Normal.HasUObject()) {
            //helper set unused styles
            if (!WidgetStyle.Pressed.HasUObject()) {
                WidgetStyle.Pressed = WidgetStyle.Normal;
            }
            if (!WidgetStyle.Hovered.HasUObject()) {
                WidgetStyle.Hovered = WidgetStyle.Normal;
            }
            if (!WidgetStyle.Disabled.HasUObject()) {
                WidgetStyle.Disabled = WidgetStyle.Normal;
                WidgetStyle.Disabled.SetResourceObject(NULL);
            }
        }
        RenderTransform = m_cWidgetTransformNormal;
    }


    Super::PostEditChangeProperty(e);
}


//Warn:: check the code is accomplish with parent's function when UE4 upgrade
TSharedRef<SWidget> UMyButton::RebuildWidget()
{
    MyButton = SNew(SButton)
        .OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClickedMy))
        .OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressedMy))
        .OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleasedMy))
        .OnHovered_UObject(this, &ThisClass::SlateHandleHoveredMy)
        .OnUnhovered_UObject(this, &ThisClass::SlateHandleUnhoveredMy)
        .ButtonStyle(&WidgetStyle)
        .ClickMethod(ClickMethod)
        .TouchMethod(TouchMethod)
        .IsFocusable(IsFocusable)
        ;

    if (GetChildrenCount() > 0)
    {
        Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
    }

    return MyButton.ToSharedRef();
}

void UMyButton::SlateHandlePressedMy()
{
    if (m_bEnableWidgetTransformPressed) {
        SetRenderTransform(m_cWidgetTransformPressed);
    }

    SlateHandlePressed();
}

void UMyButton::SlateHandleReleasedMy()
{
    if (m_bEnableWidgetTransformPressed) {
        SetRenderTransform(m_cWidgetTransformNormal);
    }

    SlateHandleReleased();
}

/*
void UMyButton::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    OnPressed.Clear();
    OnPressed.AddDynamic(this, &UMyButton::onPressedInner);

    OnReleased.Clear();
    OnReleased.AddDynamic(this, &UMyButton::onReleasedInner);
}

void UMyButton::onPressedInner()
{
    SetRenderTransform(m_cWidgetTransformPressed);
    OnPressedOverride.Broadcast();
}

void UMyButton::onReleasedInner()
{
    SetRenderTransform(m_cWidgetTransformNormal);
    OnReleased.Broadcast();
}
*/


void UMyUserWidgetWithCurveUpdaterCpp::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (m_bUpdaterNeedTick) {
        m_cUpdater.tick(InDeltaTime);
    }
}

void UMyUserWidgetWithCurveUpdaterCpp::updaterOnCommonUpdate(const FMyWithCurveUpdateStepDataBasicCpp& data, const FVector& vector)
{
    const FMyWithCurveUpdateStepDataWidgetBasicCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataWidgetBasicCpp*>(&data);
    MY_VERIFY(pData);

    FWidgetTransform wtNow;
    wtNow.Translation = FMath::Vector2DInterpTo(pData->m_cWidgetTransformStart.Translation, pData->m_cWidgetTransformEnd.Translation, vector.X, 1);
    
    wtNow.Angle = FMath::FInterpTo(pData->m_cWidgetTransformStart.Angle, pData->m_cWidgetTransformEnd.Angle, vector.Y, 1);
    wtNow.Shear = FMath::Vector2DInterpTo(pData->m_cWidgetTransformStart.Shear, pData->m_cWidgetTransformEnd.Shear, vector.Y, 1);

    wtNow.Scale = FMath::Vector2DInterpTo(pData->m_cWidgetTransformStart.Scale, pData->m_cWidgetTransformEnd.Scale, vector.Z, 1);

    SetRenderTransform(wtNow);
}

void UMyUserWidgetWithCurveUpdaterCpp::updaterOnCommonFinish(const FMyWithCurveUpdateStepDataBasicCpp& data)
{
    const FMyWithCurveUpdateStepDataWidgetBasicCpp* pData = StaticCast<const FMyWithCurveUpdateStepDataWidgetBasicCpp*>(&data);
    MY_VERIFY(pData);

    SetRenderTransform(pData->m_cWidgetTransformEnd);
}

void UMyUserWidgetWithCurveUpdaterCpp::updaterActivateTick(bool activate, FString debugString)
{
    m_bUpdaterNeedTick = activate;

    if (activate) {
        if (m_bShowWhenActivated) {
            this->SetVisibility(ESlateVisibility::Visible);
        }
    }
    else {
        if (m_bHideWhenInactivated) {
            this->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}



bool AMyTextureGenSuitBaseCpp::checkSettings() const
{
    if (!IsValid(m_pSceneCapture2D))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pSceneCapture2D invalid: %p."), m_pSceneCapture2D);
        return false;
    }

    if (m_aTargetActors.Num() <= 0)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_aTargetActors not specified."));
        return false;
    }

    if (!IsValid(m_pRenderTarget))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRenderTarget invalid: %p."), m_pRenderTarget);
        return false;
    }

    if (!IsValid(m_pRenderMaterial))
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pRenderMaterial invalid: %p."), m_pRenderMaterial);
        return false;
    }

    if (m_sRenderMaterialTextureParamName.IsNone())
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_sRenderMaterialTextureParamName not specified."));
        return false;
    };

    return true;
}

int32 AMyTextureGenSuitBaseCpp::genPrepare()
{
    if (!AMyTextureGenSuitBaseCpp::checkSettings()) {
        return -1;
    }

    //prepare the capture excutor
    USceneCaptureComponent2D* pCC = m_pSceneCapture2D->GetCaptureComponent2D();

    pCC->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
    pCC->CompositeMode = ESceneCaptureCompositeMode::SCCM_Overwrite;
    pCC->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    pCC->bCaptureEveryFrame = false;
    pCC->bCaptureOnMovement = false;
    pCC->ClearShowOnlyComponents(NULL);
    int32 l = m_aTargetActors.Num();
    for (int32 i = 0; i < l; i++)
    {
        pCC->ShowOnlyActorComponents(m_aTargetActors[i]);
    }

    if (IsValid(m_pTempRenderTarget)) {
        m_pTempRenderTarget->MarkPendingKill();
        m_pTempRenderTarget = NULL;
    }
    m_pTempRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, m_pRenderTarget->SizeX, m_pRenderTarget->SizeY, m_pRenderTarget->RenderTargetFormat);
    MY_VERIFY(IsValid(m_pTempRenderTarget));

    if (IsValid(m_pTempRenderMaterialInstance)) {
        m_pTempRenderMaterialInstance->MarkPendingKill();
        m_pTempRenderMaterialInstance = NULL;
    }

    m_pTempRenderMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, m_pRenderMaterial);
    MY_VERIFY(IsValid(m_pTempRenderMaterialInstance));

    return 0;
}

int32 AMyTextureGenSuitBaseCpp::genDo(FString newTextureName)
{
    if (newTextureName.IsEmpty()) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("newTextureName not specified."));
        return -1;
    }

    if (!AMyTextureGenSuitBaseCpp::checkSettings()) 
    {
        return -2;
    }

    if (!IsValid(m_pTempRenderTarget)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pTempRenderTarget invalid: %p."), m_pTempRenderTarget);
        return -11;
    }

    if (!IsValid(m_pTempRenderMaterialInstance)) {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("m_pTempRenderMaterialInstance invalid: %p."), m_pTempRenderMaterialInstance);
        return -12;
    }

    UTextureRenderTarget2D *pTarget0 = m_pTempRenderTarget, *pTarget1 = m_pRenderTarget;

    USceneCaptureComponent2D* pCC = m_pSceneCapture2D->GetCaptureComponent2D();
    pCC->TextureTarget = pTarget0;

    pCC->CaptureScene();

    m_pTempRenderMaterialInstance->SetTextureParameterValue(m_sRenderMaterialTextureParamName, pTarget0);

    UKismetRenderingLibrary::ClearRenderTarget2D(this, pTarget1);
    UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, pTarget1, m_pTempRenderMaterialInstance);


    UTexture2D* pNew = UMyRenderUtilsLibrary::RenderTargetCreateStaticTexture2DTryBest(pTarget1, newTextureName, TextureCompressionSettings::TC_Default, TMGS_NoMipmaps);

    if (IsValid(pNew))
    {
        return 0;
    }
    else {
        return - 100;
    }
}

bool UMyRenderUtilsLibrary::RenderTargetIsSizePowerOfTwo(class UTextureRenderTarget2D* inTextureRenderTarget)
{
    if (inTextureRenderTarget == NULL)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("inTextureRenderTarget is NULL."));
        return false;
    }

    const bool bIsValidSize = (inTextureRenderTarget->SizeX != 0 && !(inTextureRenderTarget->SizeX & (inTextureRenderTarget->SizeX - 1)) &&
        inTextureRenderTarget->SizeY != 0 && !(inTextureRenderTarget->SizeY & (inTextureRenderTarget->SizeY - 1)));

    return bIsValidSize;
}

UTexture2D* UMyRenderUtilsLibrary::RenderTargetConstructTexture2DIgnoreSizeLimit(class UTextureRenderTarget2D* inTextureRenderTarget, UObject* Outer, const FString& NewTexName, EObjectFlags InObjectFlags, uint32 Flags, TArray<uint8>* AlphaOverride)
{
    if (inTextureRenderTarget == NULL)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("inTextureRenderTarget is NULL."));
        return NULL;
    }

    Flags &= (~CTF_AllowMips); //we assume the size is not power of two, and forbid the mip feature
    UTexture2D* Result = NULL;

#if WITH_EDITOR

    // The r2t resource will be needed to read its surface contents
    FRenderTarget* RenderTarget = inTextureRenderTarget->GameThread_GetRenderTargetResource();

    const EPixelFormat PixelFormat = inTextureRenderTarget->GetFormat();
    ETextureSourceFormat TextureFormat = TSF_Invalid;
    TextureCompressionSettings CompressionSettingsForTexture = TC_Default;
    switch (PixelFormat)
    {
    case PF_B8G8R8A8:
        TextureFormat = TSF_BGRA8;
        break;
    case PF_FloatRGBA:
        TextureFormat = TSF_RGBA16F;
        CompressionSettingsForTexture = TC_HDR;
        break;
    }

    // exit if source is not compatible.
    if (TextureFormat == TSF_Invalid)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("TextureFormat is not supported in renderTarget."));
        return NULL;
    };


    // create the 2d texture
    Result = NewObject<UTexture2D>(Outer, FName(*NewTexName), InObjectFlags);
    // init to the same size as the 2d texture
    Result->Source.Init(inTextureRenderTarget->SizeX, inTextureRenderTarget->SizeY, 1, 1, TextureFormat);

    uint32* TextureData = (uint32*)Result->Source.LockMip(0);
    const int32 TextureDataSize = Result->Source.CalcMipSize(0);

    // read the 2d surface
    if (TextureFormat == TSF_BGRA8)
    {
        TArray<FColor> SurfData;
        RenderTarget->ReadPixels(SurfData);
        // override the alpha if desired
        if (AlphaOverride)
        {
            check(SurfData.Num() == AlphaOverride->Num());
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = (*AlphaOverride)[Pixel];
            }
        }
        else if (Flags & CTF_RemapAlphaAsMasked)
        {
            // if the target was rendered with a masked texture, then the depth will probably have been written instead of 0/255 for the
            // alpha, and the depth when unwritten will be 255, so remap 255 to 0 (masked out area) and anything else as 255 (written to area)
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = (SurfData[Pixel].A == 255) ? 0 : 255;
            }
        }
        else if (Flags & CTF_ForceOpaque)
        {
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = 255;
            }
        }
        // copy the 2d surface data to the first mip of the static 2d texture
        check(TextureDataSize == SurfData.Num() * sizeof(FColor));
        FMemory::Memcpy(TextureData, SurfData.GetData(), TextureDataSize);
    }
    else if (TextureFormat == TSF_RGBA16F)
    {
        TArray<FFloat16Color> SurfData;
        RenderTarget->ReadFloat16Pixels(SurfData);
        // override the alpha if desired
        if (AlphaOverride)
        {
            check(SurfData.Num() == AlphaOverride->Num());
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = ((float)(*AlphaOverride)[Pixel]) / 255.0f;
            }
        }
        else if (Flags & CTF_RemapAlphaAsMasked)
        {
            // if the target was rendered with a masked texture, then the depth will probably have been written instead of 0/255 for the
            // alpha, and the depth when unwritten will be 255, so remap 255 to 0 (masked out area) and anything else as 1 (written to area)
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = (SurfData[Pixel].A == 255) ? 0.0f : 1.0f;
            }
        }
        else if (Flags & CTF_ForceOpaque)
        {
            for (int32 Pixel = 0; Pixel < SurfData.Num(); Pixel++)
            {
                SurfData[Pixel].A = 1.0f;
            }
        }
        // copy the 2d surface data to the first mip of the static 2d texture
        check(TextureDataSize == SurfData.Num() * sizeof(FFloat16Color));
        FMemory::Memcpy(TextureData, SurfData.GetData(), TextureDataSize);
    }
    Result->Source.UnlockMip(0);
    // if render target gamma used was 1.0 then disable SRGB for the static texture
    if (FMath::Abs(RenderTarget->GetDisplayGamma() - 1.0f) < KINDA_SMALL_NUMBER)
    {
        Flags &= ~CTF_SRGB;
    }

    Result->SRGB = (Flags & CTF_SRGB) != 0;
    Result->MipGenSettings = TMGS_FromTextureGroup;

    if ((Flags & CTF_AllowMips) == 0)
    {
        Result->MipGenSettings = TMGS_NoMipmaps;
    }

    Result->CompressionSettings = CompressionSettingsForTexture;
    if (Flags & CTF_Compress)
    {
        // Set compression options.
        Result->DeferCompression = (Flags & CTF_DeferCompression) ? true : false;
    }
    else
    {
        // Disable compression
        Result->CompressionNone = true;
        Result->DeferCompression = false;
    }
    Result->PostEditChange();

#else

    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("create static mesh only valid in editor env now!"));

#endif

    return Result;
};

UTexture2D* UMyRenderUtilsLibrary::RenderTargetCreateStaticTexture2DTryBest(UTextureRenderTarget2D* inTextureRenderTarget, FString InName, enum TextureCompressionSettings CompressionSettings, enum TextureMipGenSettings MipSettings)
{
#if WITH_EDITOR

    if (inTextureRenderTarget == NULL)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("inTextureRenderTarget is NULL."));
        return nullptr;
    }

    else if (!inTextureRenderTarget->Resource)
    {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("RenderTarget has been released."));
        return nullptr;
    }
    else
    {
        FString Name;
        FString PackageName;
        IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

        //Use asset name only if directories are specified, otherwise full path
        if (!InName.Contains(TEXT("/")))
        {
            FString AssetName = inTextureRenderTarget->GetOutermost()->GetName();
            const FString SanitizedBasePackageName = PackageTools::SanitizePackageName(AssetName);
            const FString PackagePath = FPackageName::GetLongPackagePath(SanitizedBasePackageName) + TEXT("/");
            AssetTools.CreateUniqueAssetName(PackagePath, InName, PackageName, Name);
        }
        else
        {
            InName.RemoveFromStart(TEXT("/"));
            InName.RemoveFromStart(TEXT("Content/"));
            InName.StartsWith(TEXT("Game/")) == true ? InName.InsertAt(0, TEXT("/")) : InName.InsertAt(0, TEXT("/Game/"));
            AssetTools.CreateUniqueAssetName(InName, TEXT(""), PackageName, Name);
        }

        UObject* NewObj = nullptr;
        UPackage* pPackage = CreatePackage(NULL, *PackageName);
        bool bIsSizePowerOfTwo = RenderTargetIsSizePowerOfTwo(inTextureRenderTarget);

        // create a static 2d texture
        if (bIsSizePowerOfTwo)
        {
            NewObj = inTextureRenderTarget->ConstructTexture2D(pPackage, Name, inTextureRenderTarget->GetMaskedFlags(), CTF_Default | CTF_AllowMips, NULL);
        }
        else {
            NewObj = RenderTargetConstructTexture2DIgnoreSizeLimit(inTextureRenderTarget, pPackage, Name, inTextureRenderTarget->GetMaskedFlags(), CTF_Default | CTF_AllowMips, NULL);
        }


        UTexture2D* NewTex = Cast<UTexture2D>(NewObj);

        if (NewTex != nullptr)
        {
            // package needs saving
            NewObj->MarkPackageDirty();

            // Notify the asset registry
            FAssetRegistryModule::AssetCreated(NewObj);

            // Update Compression and Mip settings
            NewTex->CompressionSettings = CompressionSettings;
            NewTex->MipGenSettings = MipSettings;
            NewTex->PostEditChange();

            return NewTex;
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Failed to create a new texture, bIsSizePowerOfTwo %d."), bIsSizePowerOfTwo);
            return nullptr;
        }
    }
#else

    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Create static mesh from render target is Only supported in editor now."));
    return nullptr;

#endif

}


int32 UMyRenderUtilsLibrary::getCenterPointPositionForWidgetInCanvasWithPointAnchor(const FVector2D& canvasSize, const UWidget* widgetUnderCanvas, FVector2D& positionInCanvas)
{
    const UCanvasPanelSlot* pSlot = NULL;
    if (widgetUnderCanvas)
    {
        pSlot = Cast<const UCanvasPanelSlot>(widgetUnderCanvas->Slot);
        if (pSlot == NULL) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("widget is not under canvas, slot class name: %s."), *widgetUnderCanvas->Slot->GetClass()->GetName());
            return -10;
        }
    }
    else {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("widgetUnderCanvas is NULL."), widgetUnderCanvas);
        return -1;
    }

    FAnchorData layoutData = pSlot->GetLayout();

    bool bIsAnchorPoint = (!layoutData.Anchors.IsStretchedHorizontal()) && (!layoutData.Anchors.IsStretchedVertical());
    if (!bIsAnchorPoint) {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("widget is not anchored one point: min %s, max %s."), *layoutData.Anchors.Minimum.ToString(), *layoutData.Anchors.Maximum.ToString());
        return -20;
    }

    FVector2D anchorEndPosi = canvasSize * layoutData.Anchors.Minimum;
    FVector2D anchorStartPosi = anchorEndPosi + FVector2D(layoutData.Offsets.Left, layoutData.Offsets.Top);
    FVector2D widgetSize = FVector2D(layoutData.Offsets.Right, layoutData.Offsets.Bottom);
    FVector2D widgetCenterOffsetFromAnchorStartPosi = (FVector2D(0.5, 0.5) - layoutData.Alignment) * widgetSize;

    positionInCanvas = anchorStartPosi + widgetCenterOffsetFromAnchorStartPosi;

    return 0;
}


void UMyRenderUtilsLibrary::myElemAndGroupDynamicArrangeCalcTotalSize(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& groupCount, const FIntVector& elemCount, FVector& totalSize)
{
    FVector distanceIgnorePadding;
    myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(meta, groupCount, elemCount, distanceIgnorePadding);

    totalSize = meta.m_cPaddingPercent.GetDesiredSize3D() * meta.m_cElemSize + distanceIgnorePadding;
}

void UMyRenderUtilsLibrary::myElemAndGroupDynamicArrangeGetElemCenterPositionArrangeDirectionAllPositive(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& idxGroup, const FIntVector& idxElem, FVector& centerPosition)
{
    FVector distanceIgnorePadding;
    myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(meta, idxGroup, idxElem, distanceIgnorePadding);

    FVector distanceCenterIgnorePadding = distanceIgnorePadding + meta.m_cElemSize / 2;

    //direction is from negtive to positve
    FVector sourcePaddingPecent(meta.m_cPaddingPercent.Left, meta.m_cPaddingPercent.Top, meta.m_cPaddingPercent.ZNegative);

    centerPosition = distanceCenterIgnorePadding + sourcePaddingPecent * meta.m_cElemSize;
}

void UMyRenderUtilsLibrary::myElemAndGroupCalcDelimiterNumber(const FIntVector &groupWalked, const FIntVector &elemWalked, FIntVector &groupDelimiterNumber, FIntVector &elemDelimiterNumber)
{
    MY_VERIFY(groupWalked.X >= 0 && groupWalked.Y >= 0 && groupWalked.Z >= 0);
    MY_VERIFY(elemWalked.X >= 0 && elemWalked.Y >= 0 && elemWalked.Z >= 0);

    groupDelimiterNumber.X = groupWalked.X > 1 ? groupWalked.X - 1 : 0;
    groupDelimiterNumber.Y = groupWalked.Y > 1 ? groupWalked.Y - 1 : 0;
    groupDelimiterNumber.Z = groupWalked.Z > 1 ? groupWalked.Z - 1 : 0;

    elemDelimiterNumber.X = elemWalked.X - 1 - groupDelimiterNumber.X;
    if (elemDelimiterNumber.X < 0) elemDelimiterNumber.X = 0;

    elemDelimiterNumber.Y = elemWalked.Y - 1 - groupDelimiterNumber.Y;
    if (elemDelimiterNumber.Y < 0) elemDelimiterNumber.Y = 0;

    elemDelimiterNumber.Z = elemWalked.Z - 1 - groupDelimiterNumber.Z;
    if (elemDelimiterNumber.Z < 0) elemDelimiterNumber.Z = 0;
}

void UMyRenderUtilsLibrary::myElemAndGroupDynamicArrangeCalcDistanceWalkedBeforeIgnorePadding(const FMyElemAndGroupDynamicArrangeMetaCpp& meta, const FIntVector& groupWalked, const FIntVector& elemWalked, FVector& distanceIgnorePadding)
{
    FIntVector groupDelimiterNumber, elemDelimiterNumber;
    myElemAndGroupCalcDelimiterNumber(groupWalked, elemWalked, groupDelimiterNumber, elemDelimiterNumber);

    distanceIgnorePadding = 
        FVector(groupDelimiterNumber) * meta.m_cGroupSpacingPercent * meta.m_cElemSize +
        FVector(elemDelimiterNumber)  * meta.m_cElemSpacingPercent * meta.m_cElemSize +
        FVector(elemWalked) * meta.m_cElemSize;
}

/*
void UMyRenderUtilsLibrary::myElem2DDynamicArrangeCalcTotalSize(const FMyElem2DDynamicArrangeMetaCpp& meta, FMyIntVector2D groupCount, FMyIntVector2D elemCount, FVector2D& totalSize)
{


    FMyIntVector2D groupDelim, elemDelim;



    totalSize = 
    meta.m_cPaddingPercent.GetDesiredSize() * meta.m_cElemSize +
        groupDelim.ToVector2D() * meta.m_cGroupSpacingPercent * meta.m_cElemSize +
        elemDelim.ToVector2D()  * meta.m_cElemSpacingPercent * meta.m_cElemSize +
        elemCount.ToVector2D() * meta.m_cElemSize;
}

void UMyRenderUtilsLibrary::myElem2DDynamicArrangeGetElemCenterPosition(const FMyElem2DDynamicArrangeMetaCpp& meta, FMyIntVector2D idxGroup, FMyIntVector2D idxElem, FMyIntVector2D coordinateNegativeFlag, FVector2D& centerPosition)
{
}
*/