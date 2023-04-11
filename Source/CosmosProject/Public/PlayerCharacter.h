// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class COSMOSPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	// 회전처리 함수
	void Turn(const FInputActionValue& Values);
	// 이동처리 함수
	void Move(const FInputActionValue& Values);
	void Jump();
	void JumpEnd();
	void Fire();
	// 잡기 시도
	void TryGrab();
	// 잡기 해제
	void UnTryGrab();
	// 잡고있는중
	void Grabbing();

	UPROPERTY(BlueprintReadOnly)
		class UCameraComponent* FPSCamera;

	// 잡은 물체가 있는지 여부 기억할 변수
	bool IsGrabbed = false;

	// 필요속성 : 이동속도, 입력액션, 입력매핑컨텍스트
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		float moveSpeed = 500.0f;
	// Input Mapping Context
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputMappingContext* IMC_Default;
	// Input Action for Move
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputAction* IA_Move;
	// 마우스 입력처리
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputAction* IA_Look;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputAction* IA_Jump;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputAction* IA_Fire;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputAction* IA_Grab;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float XMovement;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float YMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MotionController")
		class UMotionControllerComponent* LeftHand;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MotionController")
		class UMotionControllerComponent* RightHand;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MotionController")
		class USkeletalMeshComponent* LeftHandMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MotionController")
		class USkeletalMeshComponent* RightHandMesh;
	// 집게손가락 표시할 모션컨트롤러
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandComp", meta = (AllowPrivateAccess = true))
		class UMotionControllerComponent* RightAim;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputMappingContext* IMC_Hand;

	// 잡은 물체 기억
	UPROPERTY()
		class UPrimitiveComponent* GrabbedObject;

	// 직전 위치
	FVector PrevPos;
	// 이전 회전값
	FQuat PrevRot;
	// 회전방향
	FQuat DeltaRotation;

	// 던질 방향
	FVector ThrowDirection;
	UPROPERTY(EditAnywhere, Category = "Grab")
		float ThrowPower = 1000;


	// 잡을 범위
	UPROPERTY(EditDefaultsOnly, Category = "Grab")
		float GrabRange = 100;
	// 회전빠르기
	UPROPERTY(EditAnywhere, Category = "Grab")
		float ToquePower = 1000;



};
