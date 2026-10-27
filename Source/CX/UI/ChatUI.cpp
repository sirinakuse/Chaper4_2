#include "UI/ChatUI.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"

void UChatUI::AddChatMessage(const FText& NewMessage)
{
	//위젯 BP에서 ScrollBox를 찾지 못했으면 처리하지 않는다.
	if (IsValid(ChatScrollBox) == false)
	{
		return;
	}

	//메시지를 표시할 TextBlock을 동적으로 만든다.
	UTextBlock* MessageText = NewObject<UTextBlock>(this);
	if (IsValid(MessageText) == false)
	{
		return;
	}

	MessageText->SetText(NewMessage);
	
	//새 메시지를 채팅 목록 마지막에 추가한다.
	ChatScrollBox->AddChild(MessageText);
	//가장 최근 메시지가 보이도록 맨 아래로 이동한다.
	ChatScrollBox->ScrollToEnd();
}

void UChatUI::FocusChatInput()
{
	//입력칸이 존재할 때만 키보드 포커스를 이동한다.
	if (IsValid(ChatInputText))
	{
		ChatInputText->SetKeyboardFocus();
	}

	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (IsValid(OwningPlayerController))
	{
		ChatInputText->SetUserFocus(OwningPlayerController);
	}
}
