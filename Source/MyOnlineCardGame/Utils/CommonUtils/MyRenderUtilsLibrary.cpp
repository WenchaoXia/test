// Fill out your copyright notice in the Description page of Project Settings.

#include "MyRenderUtilsLibrary.h"

#include "MyCommonUtils.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Console.h"

#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
//#include "Classes/PaperSprite.h"

#if WITH_EDITOR
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "PackageTools.h"
#endif

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