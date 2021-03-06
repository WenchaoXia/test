// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"

#include "Kismet/BlueprintFunctionLibrary.h"

//#include "Utils/CommonUtils/MyCommonUtils.h"
#include "MJ/Utils/MyMJUtils.h"
#include "MJ/MyMJCardPack.h"

#include "MJ/MyMJGameData.h"

#include "MyMJUtilsLocalCS.generated.h"



UCLASS()
class UMyMJUtilsLocalCSLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintPure)
    static void genDefaultCfg(FMyMJGameCfgCpp &outCfg);

    //HaiDi card in Changsha, have its own rules
    static int32 getHaiDiCard(const FMyMJCoreDataHelperCpp& helper)
    {
        if (helper.m_aIdHelperLastCardsTakenInWholeGame.Num() > 0 && helper.m_eHelperGameStateJustBeforeLastCardsTakenInWholeGame != MyMJGameStateCpp::WeavedGangSpBuZhangLocalCS) {
            return helper.m_aIdHelperLastCardsTakenInWholeGame.Top();
        }

        return -1;
    };

    //return whether can Hu
    static bool checkHuLocalCSBorn(const FMyMJGameSubLocalCSCfgCpp &localCSCfg,
                                   const FMyMJValueIdMapCpp &handCardMap,
                                   TArray<FMyMJHuScoreResultItemCpp> &outHuScoreCalcResultItems,
                                   TArray<int32> &outShowOutCards);

    //@handCardsMapIncludeTriggerCard assume trigger card have been pushed in
    //return whether ting
    static bool checkTingWithFixedCardTypeLocalCSInGame(const FMyMJHuCfgCpp &huCfg,
                                                         const FMyMJGameSubLocalCSCfgCpp &localCSCfg,
                                                         const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                                         const FMyWeavesShowedOutStatisCpp &weavesShowedOutStatis,
                                                         const FMyMJValueIdMapCpp &handCardsMapIncludeTriggerCard,
                                                         FMyMJHuScoreResultTingCpp &outResultTing);

    //return whether can Hu, call this after verifying card type is OK(whether trigger card matches the ting card value, or no trigger card but whole cards matches hu card type condition)
    //This function will also check whether @inHuActionAttr matches the hu requirement, and return false if not
    //@outScoreResultFinal will be completed filled
    static bool calcScoreFinalLocalCSInGame(const FMyMJHuCfgCpp &huCfg,
                                            const FMyWeavesShowedOutStatisCpp &weavesShowedOutStatis,
                                            const FMyMJHuScoreResultTingCpp &inResultTing,
                                            const FMyMJHuActionAttrBaseCpp &inHuActionAttrBase,
                                            const FMyTriggerDataCpp *pTriggerData,
                                            FMyMJHuScoreResultFinalCpp &outScoreResultFinal);

    //return whether ting, it simply consider card type, the caller may check extra conditions such as either card left in game, and set it in @outResultTingGroup later
    //@handCardsMapExcludeTriggerCard must not include trigger card
    static bool checkTingsLocalCSInGame(const FMyMJCardValuePackCpp &inValuePack,
                                        const FMyMJGameCfgCpp &gameCfg,
                                        const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                        const FMyMJValueIdMapCpp &handCardsMapExcludeTriggerCard,
                                        FMyMJHuScoreResultTingGroupCpp &outResultTingGroup);

    //assume all cards in, used to check like the case TianHu
    static bool checkHuWithOutTriggerCard(const FMyMJCardValuePackCpp &inValuePack,
                                          const FMyMJHuCfgCpp &huCfg,
                                          const FMyMJGameSubLocalCSCfgCpp &localCSCfg,
                                          const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                          const FMyMJValueIdMapCpp &handCardsMap,
                                          const FMyMJHuActionAttrBaseCpp &inHuActionAttrBase,
                                          FMyMJHuScoreResultFinalGroupCpp &outScoreResultFinalGroup);


    static bool checkHuWithTriggerCard(const FMyMJCardValuePackCpp &inValuePack,
                                        const FMyMJHuCfgCpp &huCfg,
                                        const FMyMJGameSubLocalCSCfgCpp &localCSCfg,
                                        const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                        const FMyMJValueIdMapCpp &handCardsMap,
                                        const FMyMJHuActionAttrCpp &huActionAttr,
                                        const FMyMJHuScoreResultTingGroupCpp &tingGroup,
                                        FMyMJHuScoreResultFinalGroupCpp &outScoreResultFinalGroup);

    static void setHuScoreResultFinalGroupMetaData(const TArray<FMyMJWeaveCpp> &weavesShowedOut,
                                                    const FMyMJValueIdMapCpp &handCardsMap,
                                                    const FMyMJHuActionAttrBaseCpp &huActionAttrBase,
                                                    FMyMJHuScoreResultFinalGroupCpp &outScoreResultFinalGroup);

protected:


};