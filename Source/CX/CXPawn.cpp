// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "CXWheelRear.h"
#include "CXPlayerController.h"
//플레이어별 추가 최고 속도 값을 읽기 위해 포함
#include "Player/CXPlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//차량 Pawn을 조종하는 Controller에 접근하기 위해 포함
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "ChaosWheeledVehicleMovementComponent.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ACXPawn::ACXPawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void ACXPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACXPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACXPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACXPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACXPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACXPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACXPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACXPawn::ResetVehicle);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACXPawn::Attack);

		EnhancedInputComponent->BindAction(OpenChatAction, ETriggerEvent::Started, this, &ACXPawn::OpenChat);

		EnhancedInputComponent->BindAction(ToggleCursorAction, ETriggerEvent::Started, this, &ACXPawn::ShowCursor);
		EnhancedInputComponent->BindAction(ToggleCursorAction, ETriggerEvent::Completed, this, &ACXPawn::HideCursor);

	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACXPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));

	if (HasAuthority())
	{
		//이 차량을 조종하는 플레이어의 Controller를 가져온다.
		if (const AController* VehicleController = GetController())
		{
			//PlayerState에 저장된 추가 최고 속도를 가져온다.
			if (const ACXPlayerState* CXPS = VehicleController->GetPlayerState<ACXPlayerState>())
			{
				//기본 최고 속도와 버프 보너스를 합쳐 현재 제한 속도를 계산한다.
				const float MaxSpeedKmh = BaseMaxSpeedKmh + CXPS->GetBonusMaxSpeed();

				//차량 물리 속도 단위(cm/s)로 변환한다.
				const float MaxSpeedCmPerSecond = MaxSpeedKmh / 0.036f;

				//현재 차량의 월드 속도를 가져온다.
				const FVector CurrentVelocity = GetMesh()->GetPhysicsLinearVelocity();

				//차량 전방 방향 성분만 분리한다.
				const FVector ForwardVector = GetActorForwardVector();
				const float ForwardSpeed = FVector::DotProduct(CurrentVelocity, ForwardVector);

				//전진 또는 후진 속도가 제한을 넘었을 때만 속도를 보정한다.
				if (FMath::Abs(ForwardSpeed) > MaxSpeedCmPerSecond)
				{
					//옆으로 미끄러지는 속도와 위아래 속도는 유지한다.
					const FVector OtherVelocity = CurrentVelocity - ForwardVector * ForwardSpeed;

					//전진, 후진 방향의 속도만 제한값 범위로 자른다.
					const float ClampedForwardSpeed = FMath::Clamp(ForwardSpeed, -MaxSpeedCmPerSecond, MaxSpeedCmPerSecond);

					//보정한 최종 물리 속도를 차량 메시에게 적용한다.
					GetMesh()->SetPhysicsLinearVelocity(OtherVelocity + ForwardVector * ClampedForwardSpeed);
				}
			}
		}
	}
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
	// get the input magnitude for steering
	float SteeringValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	// get the input magnitude for the throttle
	float ThrottleValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

void ACXPawn::Brake(const FInputActionValue& Value)
{
	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void ACXPawn::StartBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);
}

void ACXPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ACXPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

void ACXPawn::Attack(const FInputActionValue& Value)
{
	//공격 판정은 서버에서만 처리하도록 서버 RPC를 호출한다.
	ServerAttack();
}

void ACXPawn::OpenChat(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("OpenChat 호출 확인"));
	//내 차량을 조종하는 PlayerController를 가져온다.
	ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(GetController());

	if (IsValid(CXPlayerController))
	{
		//채팅 입력칸이 키보드 입력을 받도록 포커스를 준다.
		CXPlayerController->FocusChatInput();
	}
}

void ACXPawn::ShowCursor(const FInputActionValue& Value)
{
	//내 차량을 조종하는 PlayerController를 가져온다.
	ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(GetController());

	if (IsValid(CXPlayerController) == false)
	{
		return;
	}

	CXPlayerController->ShowCursorMdoe();
}

void ACXPawn::HideCursor(const FInputActionValue& Value)
{
	ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(GetController());

	if (IsValid(CXPlayerController) == false)
	{
		return;
	}

	CXPlayerController->HideCursorMdoe();
}


void ACXPawn::MulticastPlayHitEffect_Implementation(FVector HitLocation)
{
	//파티클 에셋을 지정하지 않았다면 재생하지 않는다.
	if (IsValid(HitParticle) == false)
	{
		return;
	}

	//각 클라이언트에서 같은 명중 위치에 파티클을 재생한다.
	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		HitParticle,
		HitLocation
	);
}

float ACXPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (HasAuthority() == false)
	{
		return 0.0f;
	}
	
	//DamageAmount는 float이므로 코인 계산에 사용할 정수로 변환한다.
	const int32 CoinAmount = FMath::RoundToInt(DamageAmount);
	if (CoinAmount <= 0)
	{
		return 0.0f;
	}

	//공격자와 피해자의 Controller가 유효한지 확인한다.
	AController* VictimController = GetController();
	if (IsValid(EventInstigator) == false || IsValid(VictimController) == false)
	{
		return 0.0f;
	}

	//양쪽 플레이어의 PlayerState를 가져온다.
	ACXPlayerState* AttackerPlayerState = EventInstigator->GetPlayerState<ACXPlayerState>();
	ACXPlayerState* VictimPlayerState = VictimController->GetPlayerState<ACXPlayerState>();

	if (IsValid(AttackerPlayerState) == false || IsValid(VictimPlayerState) == false)
	{
		return 0.0f;
	}

	//피해자의 코인을 실제로 차감했을 때만 공격자에게 지급한다.
	if (VictimPlayerState->RemoveCoins(CoinAmount) == false)
	{
		return 0.0f;
	}

	AttackerPlayerState->AddCoins(CoinAmount);

	//실제로 처리한 피해량을 반환한다.
	return DamageAmount;
}

void ACXPawn::MulticastAttackDebugLine_Implementation(FVector Start, FVector End, bool bHit)
{
	//명중 여부에 따라 디버그 선색을 정한다.
	const FColor DebugColor = bHit ? FColor::Green : FColor::Red;
	//각 클라이언트의 화면에 공격 범위를 표시한다.
	DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 1.0f, 0, 3.0f);
}

void ACXPawn::ServerAttack_Implementation()
{
	//공격을 시작할 차량 전방 위치를 계산한다.
	const FVector Start = GetActorLocation() + GetActorForwardVector() * 200.0f;
	//차량 전방 600cm까지 공격을 검사한다.
	const FVector End = Start + GetActorForwardVector() * 600.0f;
	
	//자신은 공격 대상에서 제외하기 위한 충돌 검사 설정이다.
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	//Vehicle 오브젝트 타입인 Actor만 대상으로 라이트레이스를 수행한다.
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult,
		Start,
		End,
		ObjectQueryParams,
		QueryParams
	);

	//서버 판정 결과를 모든 클라이언트 화면에 표시한다.
	MulticastAttackDebugLine(Start, End, bHit);

	//라인트레이스가 아무것도 맞히지 못했으면 공격을 끝낸다.
	if (bHit == false)
	{
		return;
	}
	//맞힌 Actor가 다른 차량 Pawn인지 확인한다.
	ACXPawn* HitVehicle = Cast<ACXPawn>(HitResult.GetActor());
	if (IsValid(HitVehicle) == false || HitVehicle == this)
	{
		return;
	}

	//서버가 확정한 명중 위치에 파티클을 모든 클라이언트에서 재생한다.
	MulticastPlayHitEffect(HitResult.ImpactPoint);

	//서버 로그에 명중한 차량 이름을 출력한다.
	UE_LOG(LogTemp, Warning, TEXT("상대 차량 명중: %s"), *HitVehicle->GetName());

	UGameplayStatics::ApplyDamage(
		HitVehicle,
		5.0f,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
}

#undef LOCTEXT_NAMESPACE