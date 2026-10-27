// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/CXGameState.h"
#include "CXUI.generated.h"

/**
 *  Simple Vehicle HUD class
 *  Displays the current speed and gear.
 *  Widget setup is handled in a Blueprint subclass.
 */
UCLASS(abstract)
class CX_API UCXUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	/** Controls the display of speed in Km/h or MPH */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	bool bIsMPH = false;

public:

	/** Called to update the speed display */
	void UpdateSpeed(float NewSpeed);

	/** Called to update the gear display */
	void UpdateGear(int32 NewGear);
	/** Called to update the coins display */
	void UpdateCoins(int32 NewCoins);
	/** 추가 최대 속도 표시를 갱신한다.*/
	void UpdateBonusMaxSpeed(float NewBonusMaxSpeed);
	//현재 게임 단계를 UI에 전달한다.
	void UpdateGamePhase(EGamePhase NewGamePhase);
	// 남은 시간을 UI에 전달한다.
	void UpdateRemainingTime(int32 NewRemainingTime);
	//게임 종료 결과 문구를 UI에 전달한다.
	void UpdateMatchResult(const FText& NewMatchResult);

protected:

	/** Implemented in Blueprint to display the new speed */
	UFUNCTION(BlueprintImplementableEvent, Category = Vehicle)
	void OnSpeedUpdate(float NewSpeed);

	/** Implemented in Blueprint to display the new gear */
	UFUNCTION(BlueprintImplementableEvent, Category = Vehicle)
	void OnGearUpdate(int32 NewGear);

	/** Implemented in Blueprint to display the new coins amount */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
	void OnCoinsUpdate(int32 NewCoins);

	/** 블루프린트에서 추가 최대 속도 표시를 갱신한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
	void OnBonusMaxSpeedUpdate(float NewBonusMaxSpeed);

	// 블루프린트에서 현재 게임 단계 문구를 갱신한다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void OnGamePhaseUpdate(EGamePhase NewGamePhase);

	//블루프린트에서 남은 시간 문구를 갱신한다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void OnRemainingTimeUpdate(int32 NewRemainingTime);

	//블루프린트에서 게임 종료 결과 Text를 갱신한다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void OnMatchResultUpdate(const FText& NewMatchResult);
};
