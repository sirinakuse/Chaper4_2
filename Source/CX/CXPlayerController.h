// CXPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CXPlayerController.generated.h"

class UInputMappingContext;
class ACXPawn;
class UCXUI;
class UChatUI;

/**
 *  Vehicle Player Controller class
 *  Handles input mapping and user interface
 */
UCLASS(abstract)
class CX_API ACXPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	/** If true, the optional steering wheel input mapping context will be registered */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	bool bUseSteeringWheelControls = false;

	/** Optional Input Mapping Context to be used for steering wheel input.
	 *  This is added alongside the default Input Mapping Context and does not block other forms of input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta=(EditCondition="bUseSteeringWheelControls"))
	UInputMappingContext* SteeringWheelInputMappingContext;

	/** Pointer to the controlled vehicle pawn */
	UPROPERTY(Replicated)
	TObjectPtr<ACXPawn> VehiclePawn;

	/** Type of the UI to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<UCXUI> VehicleUIClass;

	/** Pointer to the UI widget */
	TObjectPtr<UCXUI> VehicleUI;

	//게임 종료 처리를 한 번만 수행하기 위한 플래그다.
	bool bHasHandleGameEnd = false;

	//에디터에서 지정할 채팅 UI 위젯 클래스다.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UChatUI> CUIClass;

	//현재 이 클라이언트 화면에 생성된 채팅 UI다.
	UPROPERTY()
	TObjectPtr<UChatUI> CUI;

	// Begin Actor interface
protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

public:

	virtual void Tick(float Delta) override;

	//로컬 채팅 UI가 서버에 메시지 전송을 요청한다.
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Chat")
	void ServerSendChatMessage(const FString& Message);

	//GameState가 전달한 채팅 메시지를 로컬 UI에 표시한다.
	void ReceiveChatMessage(const FText& NewMessage);
	//로컬 채팅 UI의 입력칸에 키보드 포커스를 준다.
	void FocusChatInput();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void CloseChatInput();

	//Alt를 누르는 동안 마우스를 표시하고 UI를 클릭할 수 있게 한다.
	void ShowCursorMdoe();
	//Alt를 놓으면 마우스를 숨기고 차량 조작으로 돌아간다.
	void HideCursorMdoe();

protected:

	virtual void OnPossess(APawn* InPawn) override;
};
