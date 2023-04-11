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


	// ȸ��ó�� �Լ�
	void Turn(const FInputActionValue& Values);
	// �̵�ó�� �Լ�
	void Move(const FInputActionValue& Values);
	void Jump();
	void JumpEnd();
	void Fire();
	// ��� �õ�
	void TryGrab();
	// ��� ����
	void UnTryGrab();
	// ����ִ���
	void Grabbing();

	UPROPERTY(BlueprintReadOnly)
		class UCameraComponent* FPSCamera;

	// ���� ��ü�� �ִ��� ���� ����� ����
	bool IsGrabbed = false;

	// �ʿ�Ӽ� : �̵��ӵ�, �Է¾׼�, �Է¸������ؽ�Ʈ
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		float moveSpeed = 500.0f;
	// Input Mapping Context
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputMappingContext* IMC_Default;
	// Input Action for Move
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputAction* IA_Move;
	// ���콺 �Է�ó��
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
	// ���Լհ��� ǥ���� �����Ʈ�ѷ�
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HandComp", meta = (AllowPrivateAccess = true))
		class UMotionControllerComponent* RightAim;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		class UInputMappingContext* IMC_Hand;

	// ���� ��ü ���
	UPROPERTY()
		class UPrimitiveComponent* GrabbedObject;

	// ���� ��ġ
	FVector PrevPos;
	// ���� ȸ����
	FQuat PrevRot;
	// ȸ������
	FQuat DeltaRotation;

	// ���� ����
	FVector ThrowDirection;
	UPROPERTY(EditAnywhere, Category = "Grab")
		float ThrowPower = 1000;


	// ���� ����
	UPROPERTY(EditDefaultsOnly, Category = "Grab")
		float GrabRange = 100;
	// ȸ��������
	UPROPERTY(EditAnywhere, Category = "Grab")
		float ToquePower = 1000;



};
