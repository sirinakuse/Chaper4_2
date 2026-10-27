#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CXPlayerState.generated.h"

UCLASS()
class CX_API ACXPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	//Coins를 복제 대상으로 표시한다.
	//실제 복제 조건은 GetLifetimeReplicatedProps에서 등록한다.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	int32 Coins = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	float BonusMaxSpeed = 0.0f;
	
public:
	//현재 보유 Coins를 반환한다.
	int32 GetCoins() const { return Coins; }
	//현재 추가 최대 속도를 반환한다.
	float GetBonusMaxSpeed() const { return BonusMaxSpeed; }

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//서버에서 Coins를 증가시킨다.
	void AddCoins(int32 Amount);
	//서버에서 Coins를 차감하고, 차감 성공 여부를 반환한다.
	bool RemoveCoins(int32 Amount);
	//서버에서 BonusMaxSpeed를 증가시킨다.
	void AddBonusMaxSpeed(float Amount);
};
