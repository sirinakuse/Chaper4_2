// CXPlayerController.cpp


#include "CXPlayerController.h"
#include "CXPawn.h"
#include "CXUI.h"
#include "Game/CXGameState.h"
#include "Player/CXPlayerState.h"
#include "UI/ChatUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Net/UnrealNetwork.h"

void ACXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}
	
	// spawn the UI widget and add it to the viewport
	VehicleUI = CreateWidget<UCXUI>(this, VehicleUIClass);

	check(VehicleUI);

	VehicleUI->AddToViewport();

	if (IsLocalController() && IsValid(CUIClass))
	{
		CUI = CreateWidget<UChatUI>(this, CUIClass);

		if (IsValid(CUI))
		{
			CUI->AddToViewport();
		}
	}
}

void ACXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);

		// optionally add the steering wheel context
		if (bUseSteeringWheelControls && SteeringWheelInputMappingContext)
		{
			Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
		}
	}
}

void ACXPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, VehiclePawn);
}

void ACXPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}
	if(IsValid(VehicleUI))
	{
		if (const ACXPlayerState* CXPS = GetPlayerState<ACXPlayerState>())
		{
			//소유한 PlayerState에서 Coins를 읽어 UI에 전달한다.
			VehicleUI->UpdateCoins(CXPS->GetCoins());
			//소유한 PlayerState에서 추가 최대 속도를 읽어 UI에 전달한다.
			VehicleUI->UpdateBonusMaxSpeed(CXPS->GetBonusMaxSpeed());
		}

		//모든 플레이어가 공유하는 GameState를 읽어 UI에 전달한다.
		if (const ACXGameState* CXGameState = GetWorld()->GetGameState<ACXGameState>())
		{
			VehicleUI->UpdateGamePhase(CXGameState->GetGamePhase());
			VehicleUI->UpdateRemainingTime(CXGameState->GetRemaingingTime());

			//게임이 끝난 경우에만 내 화면의 승패 결과를 표시한다.
			if (CXGameState->GetGamePhase() == EGamePhase::Ended)
			{
				FText MatchResult;
				
				//최고 코인이 동점이면 모든 플레이어에게 동점 결과를 표시한다.
				if (CXGameState->IsDraw())
				{
					MatchResult = FText::FromString(TEXT("Draw"));
				}
				//내 PlayerState가 승자 PlayerState와 같으면 승리다.
				else if (CXGameState->GetWinnerPlayerState() == GetPlayerState<ACXPlayerState>())
				{
					MatchResult = FText::FromString(TEXT("Victory"));
				}
				//동점도 승리도 아니면 패배다.
				else
				{
					MatchResult = FText::FromString(TEXT("Defeat"));
				}

				VehicleUI->UpdateMatchResult(MatchResult);

				//게임 종료 직후 한 번만 차량 입력을 끄고 마우스를 표시한다.
				if (bHasHandleGameEnd == false)
				{
					bHasHandleGameEnd = true;

					//내 차량의 입력을 비활성화한다.
					if (IsValid(VehiclePawn))
					{
						VehiclePawn->DisableInput(this);
					}

					//결과 화면을 조작할 수 있도록 마우스를 표시한다.
					bShowMouseCursor = true;

					FInputModeGameAndUI InputMode;
					InputMode.SetHideCursorDuringCapture(false);
					SetInputMode(InputMode);
				}
			}
		}
	}
}

void ACXPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	//앞뒤 공백을 제거한 메시지를 만든다.
	FString TrimmedMessage = Message;
	TrimmedMessage.TrimStartAndEndInline();
	//서버 검증 위치
	if (TrimmedMessage.IsEmpty() || TrimmedMessage.Len() > 128)
	{
		return;
	}

	ACXPlayerState* CXPlayerState = GetPlayerState<ACXPlayerState>();
	ACXGameState* CXGameState = GetWorld()->GetGameState<ACXGameState>();

	if (IsValid(CXPlayerState) == false || IsValid(CXGameState) == false)
	{
		return;
	}

	//서버가 발신자 이름과 검증된 메시지를 전원에게 전달한다.
	CXGameState->MulticastChatMessage(CXPlayerState->GetPlayerName(), TrimmedMessage);
}

void ACXPlayerController::ReceiveChatMessage(const FText& NewMessage)
{
	//이 클라이언트의 채팅 UI가 존재할 때만 메시지를 추가한다.
	if (IsValid(CUI))
	{
		CUI->AddChatMessage(NewMessage);
	}
}

void ACXPlayerController::FocusChatInput()
{
	if (IsValid(CUI) == false)
	{
		return;
	}

	//키보드 입력을 게임과 UJI가 함께 처리하도록 전환한다.
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(true);
	SetInputMode(InputMode);

	//실제 글자를 입력할 EditableText에 포커스를 준다.
	CUI->FocusChatInput();
}

void ACXPlayerController::CloseChatInput()
{
	//키보드 입력을 다시 게임 전용으로 돌린다.
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	//채팅 종료 후에는 마우스 커서를 숨긴다.
	bShowMouseCursor = false;
}

void ACXPlayerController::ShowCursorMdoe()
{
	//Alt를 누르는 동안 마우스 커서를 표시한다.
	bShowMouseCursor = true;

	//게임 입력도 유지하면서 UI 클릭도 가능하게 한다.
	FInputModeGameAndUI InputMode;
	//마우스 클릭 중에도 커서를 숨기지 않는다.
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void ACXPlayerController::HideCursorMdoe()
{
	//Alt를 놓으면 마우스 커서를 다시 숨긴다.
	bShowMouseCursor = false;

	//UI 포커스를 해제하고 게임 입력만 받도록 전환하다.
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ACXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<ACXPawn>(InPawn);
}
