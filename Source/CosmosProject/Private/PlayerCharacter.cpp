// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionControllerComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	// 손 추가
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(RootComponent);
	LeftHand->SetTrackingMotionSource(FName("Left"));
	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));

	RightHand->SetupAttachment(RootComponent);
	RightHand->SetTrackingMotionSource(FName("Right"));

	// 스켈레탈 메시 컴포넌트 만들기
	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHand);
	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightHand);

	// 집게손가락
	RightAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightAim"));
	RightAim->SetupAttachment(RootComponent);
	RightAim->SetTrackingMotionSource(FName("RightAim"));

	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_QuinnXR_left.SKM_QuinnXR_left'"));
	if (TempMesh.Succeeded())
	{
		LeftHandMesh->SetSkeletalMesh(TempMesh.Object);
		LeftHandMesh->SetRelativeLocation(FVector(-2.9f, -3.5f, 4.5f));
		LeftHandMesh->SetRelativeRotation(FRotator(-25, -180, 90));
	}

	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh2(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_QuinnXR_right.SKM_QuinnXR_right'"));
	if (TempMesh2.Succeeded())
	{
		RightHandMesh->SetSkeletalMesh(TempMesh2.Object);
		RightHandMesh->SetRelativeLocation(FVector(-2.9f, 3.5f, 4.5f));
		RightHandMesh->SetRelativeRotation(FRotator(25, 0, 90));
	}

	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	FPSCamera->SetupAttachment(RootComponent);
	FPSCamera->bUsePawnControlRotation = false;
	


}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Enhanced Input 사용처리
	auto PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		// LocalPlayer
		auto localPlayer = PC->GetLocalPlayer();
		auto subSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(localPlayer);
		if (subSystem)
		{
			subSystem->AddMappingContext(IMC_Default, 0);
			subSystem->AddMappingContext(IMC_Hand, 0);
		}
	}

	// HMD가 연결되어 있지 않으면
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		//  Hand를 테스트 할 수 있는 위치로 이동시키자.
		RightAim->SetRelativeLocation(FVector(20, 20, 0));
		RightHand->SetRelativeLocation(FVector(20, 20, 0));
		// 카메라의 UsePawn Control Rotation 을 활성화 시키자.
		FPSCamera->bUsePawnControlRotation = true;

		FPSCamera->AddRelativeLocation(FVector(0, 0, 88));


	}
	// HMD가 연결되어 있다면
	else
	{
		// -> 기본 트랙킹 offset 설정
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
		FPSCamera->bUsePawnControlRotation = false;
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// HMD 가 연결돼 있지 않으면
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		// -> 손이 카메라 방향과 일치하도록 하자
		RightHand->SetRelativeRotation(FPSCamera->GetRelativeRotation());
		RightAim->SetRelativeRotation(FPSCamera->GetRelativeRotation());
	}

	Grabbing();
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	auto InputSystem = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (InputSystem)
	{
		//Binding
		InputSystem->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		InputSystem->BindAction(IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacter::Turn);
		InputSystem->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &APlayerCharacter::Jump);
		InputSystem->BindAction(IA_Jump, ETriggerEvent::Completed, this, &APlayerCharacter::JumpEnd);
		InputSystem->BindAction(IA_Fire, ETriggerEvent::Triggered, this, &APlayerCharacter::Fire);
		InputSystem->BindAction(IA_Grab, ETriggerEvent::Triggered, this, &APlayerCharacter::TryGrab);
		InputSystem->BindAction(IA_Grab, ETriggerEvent::Completed, this, &APlayerCharacter::UnTryGrab);

	}
}

void APlayerCharacter::Move(const FInputActionValue& Values)
{
	// 사용자의 입력에 따라 앞 , 뒤 , 좌, 우로 이동하고 싶다.
	// 1. 사용자의 입력에 따라
	FVector2D Axis = Values.Get<FVector2D>();
	XMovement = Axis.X;
	YMovement = Axis.Y;
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
	// 2. 앞뒤좌우라는 방향이 필요
	//FVector Dir(Axis.X, Axis.Y, 0);
	// 3. 이동하고 싶다.
	// P = P0 + vt
	//FVector P0 = GetActorLocation();
	//FVector vt = Dir * moveSpeed *GetWorld()->DeltaTimeSeconds;
	//FVector P = P0 + vt;
	//SetActorLocation(P);

}

void APlayerCharacter::Turn(const FInputActionValue& Values)
{
	FVector2D Axis = Values.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void APlayerCharacter::Jump()
{
	ACharacter::Jump();
}

void APlayerCharacter::JumpEnd()
{
	ACharacter::StopJumping();
}

void APlayerCharacter::Fire()
{



}

void APlayerCharacter::TryGrab()
{
	// 중심점
	FVector Center = RightHand->GetComponentLocation();
	// 충돌체크(구충돌)
	// 충돌한 물체들 기록할 배열
	// 충돌 질의 작성
	FCollisionQueryParams Param;
	Param.AddIgnoredActor(this);
	Param.AddIgnoredComponent(RightHand);
	TArray<FOverlapResult> HitObjs;
	bool bHit = GetWorld()->OverlapMultiByChannel(HitObjs, Center, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(GrabRange), Param);
	// 충돌하지 않았다면 아무처리 하지 않는다.
	if (bHit == false)
	{
		return;
	}
	// -> 가장 가까운 물체 잡도록 하자 (검출과정)

	// 가장 가까운 물체 인덱스
	int Closest = 0;
	for (int i = 0; i < HitObjs.Num(); i++)
	{
		// 1. 물리 기능이 활성화 되어 있는 녀석만 판단
		// -> 만약 부딪힌 컴포넌트가 물리기능이 비활성화 되어 있다면
		if (HitObjs[i].GetComponent()->IsSimulatingPhysics() == false)
		{
			// 검출하고 싶지 않다.
			continue;
		}
		// 잡았다!
		IsGrabbed = true;

		// 2. 현재 손과 가장 가까운 녀석과 이번에 검출할 녀석과 더 가까운 녀석이 있다면
		// -> 필요속성 : 현재 가장가까운 녀석과 손과의 거리
		float ClosestDist = FVector::Dist(HitObjs[Closest].GetActor()->GetActorLocation(), Center);
		// -> 필요속성 : 이번에 검출할 녀석과 손과의 거리
		float NextDist = FVector::Dist(HitObjs[i].GetActor()->GetActorLocation(), Center);
		// 3. 만약 이번에가 현재꺼 보다 더 가깝다면
		if (NextDist < ClosestDist)
		{
			//  -> 가장 가까운 녀석으로 변경하기
			Closest = i;
		}
	}

	// 만약 잡았다면
	if (IsGrabbed)
	{
		GrabbedObject = HitObjs[Closest].GetComponent();
		// -> 물체 물리기능 비활성화
		GrabbedObject->SetSimulatePhysics(false);
		GrabbedObject->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// -> 손에 붙여주자
		GrabbedObject->AttachToComponent(RightHand, FAttachmentTransformRules::KeepWorldTransform);

		PrevPos = RightHand->GetComponentLocation();
		PrevRot = RightHand->GetComponentQuat();
	}
}

void APlayerCharacter::UnTryGrab()
{
	if (IsGrabbed == false)
	{
		return;
	}

	// 1. 잡지않은 상태로 전환
	IsGrabbed = false;
	// 2. 손에서 떼어내기
	GrabbedObject->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	// 3. 물리기능 활성화
	GrabbedObject->SetSimulatePhysics(true);
	// 4. 충돌기능 활성화
	GrabbedObject->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 던지기
	GrabbedObject->AddForce(ThrowDirection * ThrowPower * GrabbedObject->GetMass());

	// 회전 시키기
	// 각속도 = (1 / dt) * dTheta(특정 축 기준 변위 각도 Axis, angle)
	float Angle;
	FVector Axis;
	DeltaRotation.ToAxisAndAngle(Axis, Angle);
	float dt = GetWorld()->DeltaTimeSeconds;
	FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
	GrabbedObject->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);

	GrabbedObject = nullptr;
}

// 던질 정보를 업데이트하기위한 기능
void APlayerCharacter::Grabbing()
{
	if (IsGrabbed == false)
	{
		return;
	}

	// 던질방향 업데이트
	ThrowDirection = RightHand->GetComponentLocation() - PrevPos;
	// 회전방향 업데이트
	// 쿼터니온 공식
	// Angle1 = Q1, Angle2 = Q2
	// Angle1 + Angle2 = Q1 * Q2
	// -Angle1 = Q1.Inverse()
	// Angle2 - Angle1 = Q2 * Q1.Inverse()
	DeltaRotation = RightHand->GetComponentQuat() * PrevRot.Inverse();

	// 이전위치 업데이트
	PrevPos = RightHand->GetComponentLocation();
	// 이전회전값 업데이트
	PrevRot = RightHand->GetComponentQuat();
}



