#include "Item/CXSpeedBuff.h"
//오버랩한 대상이 차량 Pawn인지 확인하기 위해 포함
#include "CXPawn.h"
//획득한 플레이어의 BonusMaxSpeed를 증가시키기 위해 포함
#include "Player/CXPlayerState.h"
//차량 Pawn에서 Controller를 가져오기 위해 포함
#include "GameFramework/Controller.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ACXSpeedBuff::ACXSpeedBuff()
{
	//스피드 버프는 매 프레임 동작할 필요가 없다.
	PrimaryActorTick.bCanEverTick = false;

	//서버에서 생성,파괴한 스피드 버프를 모든 클라이언트에 동기화한다.
	bReplicates = true;

	//스피드 버프 Actor의 네트워크 갱신 빈도를 1로 설정한다.
	NetUpdateFrequency = 1.0f;

	//차량과의 오버랩을 감지할 구체 충돌 컴포넌트를 생성한다.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	//충돌 컴포넌트를 이 액터의 루트 컴포넌트로 지정한다.
	SetRootComponent(CollisionComponent);
	//차량이 가까이 왔을 때 획득하도록 충돌 반지름을 설정한다.
	CollisionComponent->SetSphereRadius(100.0f);

	//물리 충돌 없이 오버랩 질의(Query)만 사용한다.
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//기본적으로 모든 충돌 채널을 무시한다.
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	//Vehicle 채널과만 오버랩 이벤트를 발생시킨다.
	CollisionComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	//오버랩 이벤트 생성을 활성화한다.
	CollisionComponent->SetGenerateOverlapEvents(true);

	//스피드 버프의 외형을 표시할 메시 컴포넌트를 생성한다.
	BuffMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuffMesh"));
	//메시를 충돌 컴포넌트에 부착해 함께 이동하도록 한다.
	BuffMesh->SetupAttachment(CollisionComponent);
	//메시 자체는 충돌을 처리하지 않고, CollisionComponent만 사용한다.
	BuffMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ACXSpeedBuff::BeginPlay()
{
	Super::BeginPlay();

	//스피드 버프 획득 판정은 서버에서만 수행한다.
	if (HasAuthority() == false)
	{
		return;
	}
	//차량이 스피드 버프 영역에 들어오면 OnSpeedBuffOverlap 함수를 호출하도록 연결한다.
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ACXSpeedBuff::OnSpeedBuffOverlap);
}

void ACXSpeedBuff::OnSpeedBuffOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() == false)
	{
		return;
	}

	//오버랩한 Actor가 유효하지 않으면 처리하지 않는다.
	if (IsValid(OtherActor) == false)
	{
		return;
	}

	//오버랩한 Actor가 차량 Pawn인지 확인한다.
	ACXPawn* VehiclePawn = Cast<ACXPawn>(OtherActor);
	if (IsValid(VehiclePawn) == false)
	{
		return;
	}

	//차량 Pawn을 조종하는 Controller를 가져온다.
	AController* VehicleController = VehiclePawn->GetController();
	if (IsValid(VehicleController) == false)
	{
		return;
	}

	//차량을 조종하는 플레이어의 PlayerState를 가져온다.
	ACXPlayerState* CXPS = VehicleController->GetPlayerState<ACXPlayerState>();
	if (IsValid(CXPS) == false)
	{
		return;
	}

	//플레이어에게 스피드 버프 보상을 지급한다.
	CXPS->AddBonusMaxSpeed(BonusSpeedValue);

	//서버에서 스피드 버프 Actor를 파괴해 모든 클라이언트에 획득 결과를 동기화한다.
	Destroy();
}

