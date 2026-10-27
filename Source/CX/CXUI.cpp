#include "CXUI.h"
#include "Game/CXGameState.h"

void UCXUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
}

void UCXUI::UpdateGear(int32 NewGear)
{
	// call the Blueprint handler
	OnGearUpdate(NewGear);
}

void UCXUI::UpdateCoins(int32 NewCoins)
{
	// 블루프린트에서 Coins Text를 갱신하도록 이벤트를 호출한다.
	OnCoinsUpdate(NewCoins);
}

void UCXUI::UpdateBonusMaxSpeed(float NewBonusMaxSpeed)
{
	// 블루프린트에서 BonusSpeed Text를 갱신하도록 이벤트를 호출한다.
	OnBonusMaxSpeedUpdate(NewBonusMaxSpeed);
}

void UCXUI::UpdateGamePhase(EGamePhase NewGamePhase)
{
	//블루프린트에서 게임 단계 Text를 갱신하도록 이벤트를 호출한다.
	OnGamePhaseUpdate(NewGamePhase);
}

void UCXUI::UpdateRemainingTime(int32 NewRemainingTime)
{
	//블루프린트에서 남은 시간 Text를 갱신하도록 이벤트를 호출한다.
	OnRemainingTimeUpdate(NewRemainingTime);
}

void UCXUI::UpdateMatchResult(const FText& NewMatchResult)
{
	//블루프린트에서 결과 Text를 갱신하도록 이벤트를 호출한다.
	OnMatchResultUpdate(NewMatchResult);
}


