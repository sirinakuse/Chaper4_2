#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatUI.generated.h"

class UScrollBox;
class UEditableText;

UCLASS()
class CX_API UChatUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	//WBP_ChatUI의 ChatScrollBox와 이름으로 연결된다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatScrollBox;

	//WBP_ChatUI의 ChatInputText와 이름으로 연결된다.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEditableText> ChatInputText;

public:
	//채팅창 맨 아래에 메시지 한 줄을 추가한다.
	void AddChatMessage(const FText& NewMessage);

	//키보드 입력 대상이 채팅 입력칸이 되도록 포커스를 준다.
	void FocusChatInput();
};
