// CXGameMode.cpp

#include "CXGameMode.h"
#include "CXPlayerController.h"
#include "Game/CXGameState.h"
#include "GameFramework/PlayerState.h"
#include "Player/CXPlayerState.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"

ACXGameMode::ACXGameMode()
{
	PlayerControllerClass = ACXPlayerController::StaticClass();
	//플레이어 접속 시 ACXPlayerState를 생성하도록 지정한다.
	PlayerStateClass = ACXPlayerState::StaticClass();
	//접속한 모든 플레이어가 ACXGameState를 공유하도록 지정한다.
	GameStateClass = ACXGameState::StaticClass();
}

AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailablePlayerStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (IsPlayerStartAvailable(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	checkf(AvailablePlayerStarts.IsEmpty() == false, TEXT("There is no available Vehicle PlayerStart."));

	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());
	return Cast<AActor>(AvailablePlayerStarts[RandomIndex]);
}

void ACXGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//이미 카운트다운이 시작됐다면 다시 시작하지 않는다.
	if (bHasCountdownStarted)
	{
		return;
	}

	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (IsValid(CXGameState) == false)
	{
		return;
	}

	//필요한 인원이 모두 접속할 때까지 대기한다.
	if (CXGameState->PlayerArray.Num() < RequiredPlayerCount)
	{
		return;
	}

	//필요한 인원이 모두 접속했으므로 대기 카운트다운을 시작한다.
	bHasCountdownStarted = true;
	StartWaitingPhase();
}

void ACXGameMode::BeginPlay()
{
	Super::BeginPlay();

	//서버 시작 직후에는 접속 인원을 기다린다.
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (IsValid(CXGameState))
	{
		CXGameState->SetGamePhase(EGamePhase::Waiting);
		CXGameState->SetRemainingTime(0);
	}
}

void ACXGameMode::StartWaitingPhase()
{
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (IsValid(CXGameState) == false)
	{
		return;
	}

	//모든 클라이언트에 대기 상태와 대기 시간을 복제한다.
	CXGameState->SetGamePhase(EGamePhase::Waiting);
	CXGameState->SetRemainingTime(WaitingDuration);

	//1초마다 남은 시간을 갱신한다.
	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle,
		this,
		&ACXGameMode::UpdateCountdown,
		1.0f,
		true
	);
}

void ACXGameMode::StartPlayingPhase()
{
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (IsValid(CXGameState) == false)
	{
		return;
	}

	//게임 진행 상태와 실제 게임 시간을 모든 클라이언트에 복제한다.
	CXGameState->SetGamePhase(EGamePhase::Playing);
	CXGameState->SetRemainingTime(GameDuration);
}

void ACXGameMode::UpdateCountdown()
{
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (IsValid(CXGameState) == false)
	{
		return;
	}

	//현재 남은 시간을 1초 감소시킨다.
	const int32 NewRemainingTime = CXGameState->GetRemaingingTime() - 1;
	CXGameState->SetRemainingTime(NewRemainingTime);

	//대기시간이 끝났으면 게임을 시작한다.
	if (CXGameState->GetGamePhase() == EGamePhase::Waiting && NewRemainingTime <= 0)
	{
		StartPlayingPhase();
		return;
	}

	if (CXGameState->GetGamePhase() == EGamePhase::Playing && NewRemainingTime <= 0)
	{
		EndGame();
	}
}

void ACXGameMode::EndGame()
{
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (IsValid(CXGameState) == false)
	{
		return;
	}

	//더 이상 카운트다운이 진행되지 않게 타이머를 중지한다.
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

	//현재까지 가장 많은 코인을 가진 플레이어를 저장한다.
	ACXPlayerState* WinnerPlayerState = nullptr;
	int32 HighestCoins = -1;
	bool bIsDraw = false;

	//서버가 관리하는 모든 플레이어의 PlayerState를 순회한다.
	for (APlayerState* PlayerState : CXGameState->PlayerArray)
	{
		ACXPlayerState* CXPlayerState = Cast<ACXPlayerState>(PlayerState);
		if (IsValid(CXPlayerState) == false)
		{
			continue;
		}

		const int32 CurrentCoins = CXPlayerState->GetCoins();

		//더 높은 코인을 찾으면 새 승리 후보로 교체하고 동점을 해제한다.
		if (CXPlayerState->GetCoins() > HighestCoins)
		{
			HighestCoins = CurrentCoins;
			WinnerPlayerState = CXPlayerState;
			bIsDraw = false;
		}
		else if (CurrentCoins == HighestCoins)
		{
			bIsDraw = true;
		}
	}

	//동점이라면 특정 승자를 저장하지 않는다.
	CXGameState->SetWinnerPlayerState(bIsDraw ? nullptr : WinnerPlayerState);
	CXGameState->SetIsDraw(bIsDraw);
	

	//모든 클라이언트에 게임 종료 상태를 복제한다.
	CXGameState->SetGamePhase(EGamePhase::Ended);
	CXGameState->SetRemainingTime(0);
}

bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (IsValid(PlayerStart) == false || IsValid(GetWorld()) == false)
	{
		return false;
	}

	const FTransform PlayerStartTransform = PlayerStart->GetActorTransform();
	const FVector CheckLocation = PlayerStartTransform.TransformPosition(VehicleSpawnCheckOffset);
	const FQuat CheckRotation = PlayerStartTransform.GetRotation();
	const FCollisionShape CheckShape = FCollisionShape::MakeBox(VehicleSpawnCheckHalfExtent);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);

	const bool bVehicleExists =
		GetWorld()->OverlapAnyTestByObjectType(
			CheckLocation,
			CheckRotation,
			ObjectQueryParams,
			CheckShape,
			QueryParams
		);

	return !bVehicleExists;
}