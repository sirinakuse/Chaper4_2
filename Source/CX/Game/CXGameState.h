#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXGameState.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Waiting,
	Playing,
	Ended
};

class ACXPlayerState;

UCLASS()
class CX_API ACXGameState : public AGameStateBase
{
	GENERATED_BODY()
	
protected:
	//모든 클라이언트에 복제할 현재 게임 단계다.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	EGamePhase GamePhase = EGamePhase::Waiting;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	int32 RemaingingTime = 0;

	//게임 종료 시 승리한 플레이어의 PlayerState다.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	TObjectPtr<ACXPlayerState> WinnerPlayerState;

	//최고 코인 플레이어가 둘 이상이면 동점 여부를 true로 저장한다.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
	bool bIsDraw = false;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	EGamePhase GetGamePhase() const { return GamePhase; }
	int32 GetRemaingingTime() const { return RemaingingTime; }
	ACXPlayerState* GetWinnerPlayerState() const { return WinnerPlayerState; }
	bool IsDraw() const { return bIsDraw; }

	//서버 GameMode가 게임 단계를 변경한다.
	void SetGamePhase(EGamePhase NewGamePhase);
	//서버 GameMode가 남은 시간을 변경한다.
	void SetRemainingTime(int32 NewRemainingTime);
	//서버 GameMode가 승자를 저장한다.
	void SetWinnerPlayerState(ACXPlayerState* NewWinnerPlayerState);
	//서버 GameMode가 동점 여부를 저장한다.
	void SetIsDraw(bool bNewIsDraw);

	//서버가 받은 채팅을 모든 클라이언트에 전달한다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastChatMessage(const FString& SenderName, const FString& Message);
};
