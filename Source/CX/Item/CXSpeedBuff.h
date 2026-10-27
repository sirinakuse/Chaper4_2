// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXSpeedBuff.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS()
class CX_API ACXSpeedBuff : public AActor
{
	GENERATED_BODY()
	
protected:
	//차량과의 오버랩을 감지하는 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;
	//코인의 외형을 표시하는 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BuffMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Coin")
	float BonusSpeedValue = 10.0f;

public:	
	ACXSpeedBuff();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSpeedBuffOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
