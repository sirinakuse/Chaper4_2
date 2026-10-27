#include "Player/CXPlayerState.h"

//DOREPLIFETIME_CONDITION 매크로를 사용하기 위해 포함한다.
#include "Net/UnrealNetwork.h"

void ACXPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//부모 APlayerState가 등록한 기본 복제 속성을 먼저 유지한다.
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//ThisClass: 지금 함수가 속한 ACXPlayerState
	//Coins: 복제할 멤버 변수
	//COND_OwnerOnly: 이 PlayerState를 소유한 클라이언트에게만 전송
	//Coins는 이 PlayerState의 소유자에게만 복제한다.
	DOREPLIFETIME_CONDITION(ThisClass, Coins, COND_OwnerOnly);
	//BonusMaxSpeed도 이 PlayerState의 소유자에게만 복제한다.
	DOREPLIFETIME_CONDITION(ThisClass, BonusMaxSpeed, COND_OwnerOnly);
}

void ACXPlayerState::AddCoins(int32 Amount)
{
	//Coins는 서버에서만 변경할 수 있다.
	if (HasAuthority() == false)
	{
		return;
	}
	//0 이하의 값으로 Coins가 변경되는 것을 막는다.
	if (Amount <= 0)
	{
		return;
	}
	
	//유효한 금액만큼 Coins를 증가시킨다.
	Coins += Amount;
}

bool ACXPlayerState::RemoveCoins(int32 Amount)
{
	//Coins는 서버에서만 변경할 수 있다.
	if (HasAuthority() == false)
	{
		return false;
	}
	// 0이하의 값은 차감할 수 없다.
	if (Amount <= 0)
	{
		return false;
	}

	//보유 Coins가 부족하면 차감하지 않는다.
	if (Coins < Amount)
	{
		return false;
	}
	//유효한 금액만큼 Coins를 차감한다.
	Coins -= Amount;

	return true;
}

void ACXPlayerState::AddBonusMaxSpeed(float Amount)
{
	//BonusMaxSpeed는 서버에서만 변경할 수 있다.
	//서버 권한 검사
	if (HasAuthority() == false)
	{
		return;
	}
	//0 이하의 값으로 BonusMaxSpeed가 변경되는 것을 막는다.
	if (Amount <= 0.0f)
	{
		return;
	}

	//유효한 값만큼 BonusMaxSpeed를 증가시킨다.
	BonusMaxSpeed += Amount;
}
