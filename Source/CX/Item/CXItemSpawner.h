#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXItemSpawner.generated.h"

UCLASS()
class CX_API ACXItemSpawner : public AActor
{
	GENERATED_BODY()
	
protected:
	//이 스포너가 생성할 아이템 블루프린트 클래스
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<AActor> ItemClass;

	//현재 이 스포너가 생성해 둔 아이템 Actor
	UPROPERTY(VisibleInstanceOnly, Category = "Spawner")
	TObjectPtr<AActor> SpawnedItem;

	//아이템이 파괴된 뒤 다시 생성되기까지의 시간
	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
	float RespawnDelay = 2.0f;

	//아이템 리스폰을 예약하는 타이머 핸들
	FTimerHandle RespawnTimerHandle;

public:	
	ACXItemSpawner();

protected:
	virtual void BeginPlay() override;

	//ItemClass를 현재 스포너 위치에 생성한다.
	void SpawnItem();

	//생성한 아이템이 파괴되었을 때 호출된다.
	UFUNCTION()
	void OnSpawnedItemDestroyed(AActor* DestroyedActor);
};
