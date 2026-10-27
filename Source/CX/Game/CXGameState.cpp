#include "Game/CXGameState.h"

#include "CXPlayerController.h"
//DoREPLIFETIME 매크로를 사용하기 위해 포함
#include "Net/UnrealNetwork.h"

void ACXGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//부모 GameStateBase가 등록한 기본 복제를 유지한다.
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//현재 게임 단계는 모든 클라이언트에 복제한다.
	DOREPLIFETIME(ThisClass, GamePhase);
	//남은 시간도 모든 클라이언트에 복제한다.
	DOREPLIFETIME(ThisClass, RemaingingTime);
	//승리한 플레이어 정보도 모든 클라이언트에 복제한다.
	DOREPLIFETIME(ThisClass, WinnerPlayerState);
	DOREPLIFETIME(ThisClass, bIsDraw);
}

void ACXGameState::SetGamePhase(EGamePhase NewGamePhase)
{
	if (HasAuthority() == false)
	{
		return;
	}

	GamePhase = NewGamePhase;
}

void ACXGameState::SetRemainingTime(int32 NewRemainingTime)
{
	if (HasAuthority() == false)
	{
		return;
	}
	//남은 시간이 음수가 되지 않도록 제한한다.
	RemaingingTime = FMath::Max(NewRemainingTime, 0);
}

void ACXGameState::SetWinnerPlayerState(ACXPlayerState* NewWinnerPlayerState)
{
	//승리 정보는 서버만 변경할 수 있다.
	if (HasAuthority() == false)
	{
		return;
	}

	WinnerPlayerState = NewWinnerPlayerState;
}

void ACXGameState::SetIsDraw(bool bNewIsDraw)
{
	if (HasAuthority() == false)
	{
		return;
	}

	bIsDraw = bNewIsDraw;
}

void ACXGameState::MulticastChatMessage_Implementation(const FString& SenderName, const FString& Message)
{
	//표시할 "보낸사람: 메시지" 형태의 Text를 만든다.
	const FText ChatLine = FText::FromString(FString::Printf(TEXT("%s: %s"), *SenderName, *Message));

	//각 실행 중인 월드에서 로컬 PlayerController를 찾아 UI 갱신을 요청한다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(It->Get());

		if (IsValid(CXPlayerController))
		{
			CXPlayerController->ReceiveChatMessage(ChatLine);
		}
	}
}
