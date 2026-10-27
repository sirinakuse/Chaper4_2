#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXCoin.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS()
class CX_API ACXCoin : public AActor
{
	GENERATED_BODY()
	
protected:
	//차량과의 오버랩을 감지하는 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;
	//코인의 외형을 표시하는 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CoinMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Coin")
	int32 CoinValue = 10;

public:	
	ACXCoin();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCoinOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
