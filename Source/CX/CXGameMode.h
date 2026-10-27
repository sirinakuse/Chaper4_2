// CXGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CXGameMode.generated.h"

UCLASS(MinimalAPI)
class ACXGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACXGameMode();

protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	//새 플레이어가 서버에 접속한 뒤 호출한다.
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void BeginPlay() override;
	//대기 단계를 시작한다.
	void StartWaitingPhase();
	//게임 진행 단계를 시작한다.
	void StartPlayingPhase();
	//게임을 종료하고 승패를 판정한다.
	void EndGame();
	//진행 단계에서 남은 시간을 1초 감소시킨다.
	void UpdateCountdown();

private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;

private:
	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

protected:
	//대기 단계가 유지될 시간(초)다.
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 WaitingDuration = 5;
	//실제 게임이 진행될 시간(초)이다.
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 GameDuration = 60;

	//게임 시작에 필요한 최소 접속 인원이다.
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 RequiredPlayerCount = 2;
	//이미 대기 카운트다운을 시작했는지 저장한다.
	bool bHasCountdownStarted = false;

	//매초 카운트다운을 호출할 타이머 핸들이다.
	FTimerHandle CountdownTimerHandle;
};



