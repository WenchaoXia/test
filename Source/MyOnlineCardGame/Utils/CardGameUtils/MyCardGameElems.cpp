// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCardGameElems.h"

#define MyCardGameAssetCommonPartialNamePrefix_Value_Normal (TEXT("v%02d"))
#define MyCardGameAssetCommonPartialNamePrefix_Value_Unknown (TEXT("vUnknown"))


#define MyCardGameAssetCardWidgetPartialNameSuffix_Value_MainTexture (TEXT("_mainTexture"))


MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::updateValueShowing(int32 newValueShowing, int32 animationTimeMs)
{
    //Currently only support set at instance

    m_iValueShowing = newValueShowing;

    return updateWithValue(false);
}

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::getValueShowing(int32& valueShowing) const
{
    valueShowing = m_iValueShowing;
    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::setResPath(const FDirectoryPath& newResPath)
{
    m_cResPath = newResPath;
    updateWithValue(true);
    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::getResPath(FDirectoryPath& resPath) const
{
    resPath = m_cResPath;  
    return MyErrorCodeCommonPartCpp::NoError;
}

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::updateWithValue(bool bForce)
{
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;

    if (m_iValueUpdatedBefore == m_iValueShowing && bForce == false) {
        return MyErrorCodeCommonPartCpp::NoError;
    }

    m_iValueUpdatedBefore = m_iValueShowing;

    FString vPrefix;
    if (!m_cResPath.Path.IsEmpty()) {

        if (m_iValueShowing > MyCardGameValueUnknown) {
            vPrefix = FString::Printf(MyCardGameAssetCommonPartialNamePrefix_Value_Normal, m_iValueShowing);
        }
        else if (m_iValueShowing == MyCardGameValueUnknown) {
            vPrefix = MyCardGameAssetCommonPartialNamePrefix_Value_Unknown;
        }

    }

    m_pCardMainTexture = NULL;
    if (!vPrefix.IsEmpty()) {
        MyErrorCodeCommonPartCpp retT = helperTryLoadCardRes(m_cResPath.Path, vPrefix, &m_pCardMainTexture);
        MyErrorCodePartJoin(ret, retT);
    }

    UMyButton* pCB = getCenterButton(false);
    if (pCB) {
        //assume modify inplace is OK, and target texuture can be NULL here, on purpose or unexpected
        pCB->WidgetStyle.Normal.SetResourceObject(m_pCardMainTexture);
        pCB->SetStyle(pCB->WidgetStyle);
    }
    else {
        MyErrorCodePartJoin(ret, MyErrorCodeCommonPartCpp::UObjectNotExist);
    }

    return ret;
}


#if WITH_EDITOR

void UMyCardGameCardWidgetBaseCpp::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("PostEditChangeProperty, %s"), *m_cResPath.Path);
    FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyCardGameCardWidgetBaseCpp, m_iValueShowing))
    {

        updateWithValue(false);
        //updateWithValue(m_iValueShowing);
    }
    else {
        PropertyName = (e.MemberProperty != NULL) ? e.MemberProperty->GetFName() : NAME_None;

        if (PropertyName == GET_MEMBER_NAME_CHECKED(UMyCardGameCardWidgetBaseCpp, m_cResPath))
        {
            updateWithValue(true);
        }
    }

    Super::PostEditChangeProperty(e);
}

#endif

MyErrorCodeCommonPartCpp UMyCardGameCardWidgetBaseCpp::helperTryLoadCardRes(const FString &modelAssetPath, const FString &valuePrefix, class UTexture** ppOutCardValueMainTexture)
{
    MyErrorCodeCommonPartCpp ret = MyErrorCodeCommonPartCpp::NoError;

    if (ppOutCardValueMainTexture) {
        FString baseColorFullPathName = modelAssetPath + TEXT("/") + valuePrefix + MyCardGameAssetCardWidgetPartialNameSuffix_Value_MainTexture;
        UTexture* pTBaseColor = UMyCommonUtilsLibrary::helperTryFindAndLoadAsset<UTexture>(nullptr, baseColorFullPathName);
        if (IsValid(pTBaseColor)) {
            *ppOutCardValueMainTexture = pTBaseColor;
        }
        else {
            *ppOutCardValueMainTexture = NULL;
            MyErrorCodePartJoin(ret, MyErrorCodeCommonPartCpp::AssetLoadFail);
        }
    }

    return ret;
};
