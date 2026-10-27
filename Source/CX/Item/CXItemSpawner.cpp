#include "Item/CXItemSpawner.h"

//월드에 아이템 Actor를 생성하기 위해 포함
#include "Engine/World.h"
//아이템 리스폰 타이머를 사용하기 위해 포함
#include "TimerManager.h"

ACXItemSpawner::ACXItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ACXItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() == false)
	{
		return;
	}

	//게임 싲가 시 첫 번째 아이템을 생성한다.
	SpawnItem();
}

void ACXItemSpawner::SpawnItem()
{
	//에디터에서 생성할 아이템 클래스를 지정하지 않았다면 생성하지 않는다.
	if (ItemClass == nullptr)
	{
		return;
	}

	//이미 이 스포너가 만든 아이템이 남아 있다면 중복 생성하지 않는다.
	if (IsValid(SpawnedItem))
	{
		return;
	}

	//유효한 월드를 가져오지 못했다면 생성할 수 없다.
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	//아이템을 아직 BeginPlay 하지 않은 상태로 생성한다.
	SpawnedItem = World->SpawnActorDeferred<AActor>(
		ItemClass,
		GetActorTransform(),
		this
	);

	//생성에 성공했을 때만 파괴 이벤트를 먼저 구독한다.
	if (IsValid(SpawnedItem))
	{
		//생성한 아이템이 파괴되면 스포너가 리스폰을 처리한다.
		SpawnedItem->OnDestroyed.AddDynamic(
			this,
			&ACXItemSpawner::OnSpawnedItemDestroyed
		);

		//델리게이트 연결을 마친 뒤 아이템 생성을 완료한다.
		SpawnedItem->FinishSpawning(GetActorTransform());
	}
}

void ACXItemSpawner::OnSpawnedItemDestroyed(AActor* DestroyedActor)
{
	//이 스포너가 현재 관리하는 아이템이 아니라면 무시한다.
	if (DestroyedActor != SpawnedItem)
	{
		return;
	}

	//파괴된 아이템 참조를 비운다.
	SpawnedItem = nullptr;

	//RespawnDelay(기본2초) 뒤에 SpawnItem을 한 번 호출한다.
	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&ACXItemSpawner::SpawnItem,
		RespawnDelay,
		false
	);
}


