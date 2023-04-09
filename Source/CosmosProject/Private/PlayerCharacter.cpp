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
	FPSCamera->SetupAttachment(GetMesh());
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