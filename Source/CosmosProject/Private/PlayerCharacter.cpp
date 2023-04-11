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


	// �� �߰�
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(RootComponent);
	LeftHand->SetTrackingMotionSource(FName("Left"));
	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));

	RightHand->SetupAttachment(RootComponent);
	RightHand->SetTrackingMotionSource(FName("Right"));

	// ���̷�Ż �޽� ������Ʈ �����
	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHand);
	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightHand);

	// ���Լհ���
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
	
	// Enhanced Input ���ó��
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

	// HMD�� ����Ǿ� ���� ������
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		//  Hand�� �׽�Ʈ �� �� �ִ� ��ġ�� �̵���Ű��.
		RightAim->SetRelativeLocation(FVector(20, 20, 0));
		RightHand->SetRelativeLocation(FVector(20, 20, 0));
		// ī�޶��� UsePawn Control Rotation �� Ȱ��ȭ ��Ű��.
		FPSCamera->bUsePawnControlRotation = true;

		FPSCamera->AddRelativeLocation(FVector(0, 0, 88));


	}
	// HMD�� ����Ǿ� �ִٸ�
	else
	{
		// -> �⺻ Ʈ��ŷ offset ����
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
		FPSCamera->bUsePawnControlRotation = false;
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// HMD �� ����� ���� ������
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() == false)
	{
		// -> ���� ī�޶� ����� ��ġ�ϵ��� ����
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
	// ������� �Է¿� ���� �� , �� , ��, ��� �̵��ϰ� �ʹ�.
	// 1. ������� �Է¿� ����
	FVector2D Axis = Values.Get<FVector2D>();
	XMovement = Axis.X;
	YMovement = Axis.Y;
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
	// 2. �յ��¿��� ������ �ʿ�
	//FVector Dir(Axis.X, Axis.Y, 0);
	// 3. �̵��ϰ� �ʹ�.
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
	// �߽���
	FVector Center = RightHand->GetComponentLocation();
	// �浹üũ(���浹)
	// �浹�� ��ü�� ����� �迭
	// �浹 ���� �ۼ�
	FCollisionQueryParams Param;
	Param.AddIgnoredActor(this);
	Param.AddIgnoredComponent(RightHand);
	TArray<FOverlapResult> HitObjs;
	bool bHit = GetWorld()->OverlapMultiByChannel(HitObjs, Center, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(GrabRange), Param);
	// �浹���� �ʾҴٸ� �ƹ�ó�� ���� �ʴ´�.
	if (bHit == false)
	{
		return;
	}
	// -> ���� ����� ��ü �⵵�� ���� (�������)

	// ���� ����� ��ü �ε���
	int Closest = 0;
	for (int i = 0; i < HitObjs.Num(); i++)
	{
		// 1. ���� ����� Ȱ��ȭ �Ǿ� �ִ� �༮�� �Ǵ�
		// -> ���� �ε��� ������Ʈ�� ��������� ��Ȱ��ȭ �Ǿ� �ִٸ�
		if (HitObjs[i].GetComponent()->IsSimulatingPhysics() == false)
		{
			// �����ϰ� ���� �ʴ�.
			continue;
		}
		// ��Ҵ�!
		IsGrabbed = true;

		// 2. ���� �հ� ���� ����� �༮�� �̹��� ������ �༮�� �� ����� �༮�� �ִٸ�
		// -> �ʿ�Ӽ� : ���� ���尡��� �༮�� �հ��� �Ÿ�
		float ClosestDist = FVector::Dist(HitObjs[Closest].GetActor()->GetActorLocation(), Center);
		// -> �ʿ�Ӽ� : �̹��� ������ �༮�� �հ��� �Ÿ�
		float NextDist = FVector::Dist(HitObjs[i].GetActor()->GetActorLocation(), Center);
		// 3. ���� �̹����� ���粨 ���� �� �����ٸ�
		if (NextDist < ClosestDist)
		{
			//  -> ���� ����� �༮���� �����ϱ�
			Closest = i;
		}
	}

	// ���� ��Ҵٸ�
	if (IsGrabbed)
	{
		GrabbedObject = HitObjs[Closest].GetComponent();
		// -> ��ü ������� ��Ȱ��ȭ
		GrabbedObject->SetSimulatePhysics(false);
		GrabbedObject->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// -> �տ� �ٿ�����
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

	// 1. �������� ���·� ��ȯ
	IsGrabbed = false;
	// 2. �տ��� �����
	GrabbedObject->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	// 3. ������� Ȱ��ȭ
	GrabbedObject->SetSimulatePhysics(true);
	// 4. �浹��� Ȱ��ȭ
	GrabbedObject->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// ������
	GrabbedObject->AddForce(ThrowDirection * ThrowPower * GrabbedObject->GetMass());

	// ȸ�� ��Ű��
	// ���ӵ� = (1 / dt) * dTheta(Ư�� �� ���� ���� ���� Axis, angle)
	float Angle;
	FVector Axis;
	DeltaRotation.ToAxisAndAngle(Axis, Angle);
	float dt = GetWorld()->DeltaTimeSeconds;
	FVector AngularVelocity = (1.0f / dt) * Angle * Axis;
	GrabbedObject->SetPhysicsAngularVelocityInRadians(AngularVelocity * ToquePower, true);

	GrabbedObject = nullptr;
}

// ���� ������ ������Ʈ�ϱ����� ���
void APlayerCharacter::Grabbing()
{
	if (IsGrabbed == false)
	{
		return;
	}

	// �������� ������Ʈ
	ThrowDirection = RightHand->GetComponentLocation() - PrevPos;
	// ȸ������ ������Ʈ
	// ���ʹϿ� ����
	// Angle1 = Q1, Angle2 = Q2
	// Angle1 + Angle2 = Q1 * Q2
	// -Angle1 = Q1.Inverse()
	// Angle2 - Angle1 = Q2 * Q1.Inverse()
	DeltaRotation = RightHand->GetComponentQuat() * PrevRot.Inverse();

	// ������ġ ������Ʈ
	PrevPos = RightHand->GetComponentLocation();
	// ����ȸ���� ������Ʈ
	PrevRot = RightHand->GetComponentQuat();
}



