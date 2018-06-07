// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class MyOnlineCardGameEditorTarget : TargetRules
{
	public MyOnlineCardGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "MyOnlineCardGame", "MyOnlineCardGameCore", "MyOnlineCardGameClient" } );

        //bFasterWithoutUnity = true;

        bEnforceIWYU = true;

        bUseUnityBuild = false;
        bForceUnityBuild = false;
        bUseAdaptiveUnityBuild = false;
        //MinGameModuleSourceFilesForUnityBuild = 10;
    }
}
