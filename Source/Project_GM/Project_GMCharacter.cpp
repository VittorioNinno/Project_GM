#include "Project_GMCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CharacterComponents/DashComponent.h"
#include "CharacterComponents/WallMechanicsComponent.h"
#include "CharacterComponents/StaminaComponent.h"
#include "CharacterComponents/FlyComponent.h"
#include "DrawDebugHelpers.h"
#include "CharacterComponents/HealthComponent.h"
#include "Components/ProgressBar.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AProject_GMCharacter::AProject_GMCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	MovementComponent = GetCharacterMovement();
	
	/** RESTORE BASE MOVEMENT: Ensure character rotates to movement, not camera */
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	MovementComponent->bOrientRotationToMovement = true; 
	MovementComponent->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	MovementComponent->NavAgentProps.bCanCrouch = true;
	MovementComponent->JumpZVelocity = 500.f;
	MovementComponent->AirControl = 0.35f;

	JumpMaxCount = 2;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
	WallMechanicsComponent = CreateDefaultSubobject<UWallMechanicsComponent>(TEXT("WallMechanicsComponent"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	FlyComponent = CreateDefaultSubobject<UFlyComponent>(TEXT("FlyComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	
}

void AProject_GMCharacter::BeginPlay()
{
	
	/** Capture the default values set in Blueprint at start */
	
	DefaultGroundFriction = MovementComponent->GroundFriction;
	DefaultBrakingDeceleration = MovementComponent->BrakingDecelerationWalking;
	DefaultMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
	DefaultMaxAcceleration = MovementComponent->MaxAcceleration;
	DefaultRotationRate = MovementComponent->RotationRate.Yaw;
	DefaultCrouchedWalkSpeed = MovementComponent->MaxWalkSpeedCrouched;

	if (IsLocallyControlled() && HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidgetInstance) HUDWidgetInstance->AddToViewport();
	}
	Super::BeginPlay();
}

void AProject_GMCharacter::HandleFlyPhysics() const
{
	MovementComponent->AddImpulse(FlyComponent->GetFlyingUpDirection(Controller) * FlyComponent->CalculateFinalFlyForce());
}

void AProject_GMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DisplaySurfaceDebugInfo();
	
	HandleGroundPhysics(DeltaTime);
	
	if (bIsSliding)
	{
		if (GetVelocity().Size2D() < MinSlideSpeed || !MovementComponent->IsMovingOnGround())
		{
			StopCrouch();
		}
	}
	
	HandleFlyPhysics();
}



void AProject_GMCharacter::HandleGroundPhysics(float DeltaTime)
{
	if (!MovementComponent || !StaminaComponent) return;

	/** 1. SUCTION ZONE OVERRIDE */
	if (bIsUnderSuction)
	{
		FVector CharacterLoc = GetActorLocation();
		FVector SuctionDir = (SuctionTargetLocation - CharacterLoc).GetSafeNormal();
		MovementComponent->AddForce(SuctionDir * CurrentSuctionStrength * MovementComponent->Mass);

		if (StaminaComponent->GetStaminaPercentage() <= 0.0f)
		{
			MovementComponent->GroundFriction = 0.0f; 
			MovementComponent->MaxWalkSpeed = 0.0f;
			MovementComponent->AddInputVector(SuctionDir * 0.1f);
			return;
		}

		FVector InputDir = GetLastMovementInputVector();
		float Alignment = FVector::DotProduct(InputDir.GetSafeNormal(), SuctionDir);

		if (Alignment > 0.2f)
		{
			MovementComponent->GroundFriction = 8.0f; 
			MovementComponent->BrakingDecelerationWalking = 0.0f; 
			MovementComponent->MaxWalkSpeed = 1200.0f;
			StaminaComponent->SetIsRegenerating(true);
		}
		else if (Alignment < -0.2f)
		{
			StaminaComponent->SetIsRegenerating(false);
			StaminaComponent->ModifyStamina(-25.0f * FMath::Abs(Alignment) * DeltaTime);

			MovementComponent->GroundFriction = 12.0f;
			MovementComponent->MaxWalkSpeed = 300.0f;
		}
		else
		{
			MovementComponent->GroundFriction = 0.0f;
			if (InputDir.IsNearlyZero()) MovementComponent->AddInputVector(SuctionDir * 0.5f);
			MovementComponent->MaxWalkSpeed = 600.0f;
			StaminaComponent->SetIsRegenerating(true);
		}
		return;
	}

	/** 2. ICE DETECTION (Original Line Trace Implementation) */
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, 150.0f);
	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);
	TraceParams.bReturnPhysicalMaterial = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
	bool bIsOnIce = false;

	if (bHit && HitResult.PhysMaterial.IsValid())
	{
		if (HitResult.PhysMaterial->SurfaceType == EPhysicalSurface::SurfaceType1)
		{
			bIsOnIce = true;
		}
	}

	if (bIsOnIce)
	{
		MovementComponent->GroundFriction = 0.0f; 
		MovementComponent->BrakingDecelerationWalking = 0.0f; 
		MovementComponent->MaxAcceleration = IceMaxAcceleration; 
		MovementComponent->RotationRate = FRotator(0.0f, IceRotationRate, 0.0f);

		/** Slope Gravity Logic based on your original code */
		FVector FloorNormal = HitResult.ImpactNormal;
		if (FloorNormal.Z < 0.99f) 
		{
			FVector SlopeForce = FVector(FloorNormal.X, FloorNormal.Y, 0.0f);
			MovementComponent->AddForce(SlopeForce * IceSlopeGravityForce * DeltaTime);
		}
	}
	else
	{
		/** 3. RESTORE DEFAULT MOVEMENT PROPERTIES */
		MovementComponent->GroundFriction = DefaultGroundFriction;
		MovementComponent->BrakingDecelerationWalking = DefaultBrakingDeceleration;
		MovementComponent->MaxAcceleration = DefaultMaxAcceleration;
		MovementComponent->RotationRate = FRotator(0.0f, DefaultRotationRate, 0.0f);
		StaminaComponent->SetIsRegenerating(true);
	}
}

void AProject_GMCharacter::DisplaySurfaceDebugInfo()
{
	if (!GEngine) return;

	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, 150.0f);
	FHitResult DebugHit;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);
	TraceParams.bReturnPhysicalMaterial = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(DebugHit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
	
	FString MaterialName = (bHit && DebugHit.PhysMaterial.IsValid()) ? DebugHit.PhysMaterial->GetName() : TEXT("NONE");
	int32 SurfaceIndex = (bHit && DebugHit.PhysMaterial.IsValid()) ? (int32)DebugHit.PhysMaterial->SurfaceType : -1;

	GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Cyan, FString::Printf(TEXT("SURFACE DEBUG -> Type: %d | Material: %s"), SurfaceIndex, *MaterialName));
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, -1, 0, 2.0f);
}

void AProject_GMCharacter::ApplySuctionForce(FVector TargetLocation, float Strength, float DeltaTime)
{
	bIsUnderSuction = true;
	SuctionTargetLocation = TargetLocation;
	CurrentSuctionStrength = Strength;
}

void AProject_GMCharacter::ResetSuctionPhysics()
{
	bIsUnderSuction = false;
}

/** Original Ability Handlers */
void AProject_GMCharacter::DoJumpStart()
{
	if (GetCharacterMovement()->MovementMode== MOVE_Flying) return;
	
	if (WallMechanicsComponent && WallMechanicsComponent->AttemptWallJump())
	{
		JumpCurrentCount = 0;
		return;
	}
	Jump();
}

void AProject_GMCharacter::OnDashAction()
{
	if (GetCharacterMovement()->MovementMode== MOVE_Flying) return;
	
	if (DashComponent) DashComponent->PerformDash();
}
void AProject_GMCharacter::StartSprint()
{
	if (!bIsCrouched && !bIsSliding) GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}
void AProject_GMCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
}

void AProject_GMCharacter::StartCrouch()
{
	if (GetCharacterMovement()->MovementMode== MOVE_Flying) return;
	
	if (GetCharacterMovement()->IsMovingOnGround() && GetVelocity().Size2D() >= MinSlideSpeed)
	{
		bIsSliding = true;
		GetCharacterMovement()->GroundFriction = SlideFriction;
		GetCharacterMovement()->MaxWalkSpeedCrouched = SprintSpeed;
	}
	Crouch();
}

void AProject_GMCharacter::StopCrouch()
{
	if (GetCharacterMovement()->MovementMode== MOVE_Flying) return;
	
	bIsSliding = false;
	GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
	GetCharacterMovement()->MaxWalkSpeedCrouched = DefaultCrouchedWalkSpeed;
	UnCrouch();
}

float AProject_GMCharacter::GetStaminaPercentage() const { return StaminaComponent ? StaminaComponent->GetStaminaPercentage() : 0.0f; }

void AProject_GMCharacter::Move(const FInputActionValue& Value)
{
	FVector2D V = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator YawR(0, Controller->GetControlRotation().Yaw, 0);
		AddMovementInput(FRotationMatrix(YawR).GetUnitAxis(EAxis::X), V.Y);
		AddMovementInput(FRotationMatrix(YawR).GetUnitAxis(EAxis::Y), V.X);
	}
}

void AProject_GMCharacter::Look(const FInputActionValue& Value)
{
	FVector2D V = Value.Get<FVector2D>();
	if (Controller) { AddControllerYawInput(V.X); AddControllerPitchInput(V.Y); }
}

void AProject_GMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AProject_GMCharacter::DoJumpStart);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProject_GMCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProject_GMCharacter::Look);
		EIC->BindAction(DashAction, ETriggerEvent::Started, this, &AProject_GMCharacter::OnDashAction);
		EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AProject_GMCharacter::StartSprint);
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AProject_GMCharacter::StopSprint);
		EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &AProject_GMCharacter::StartCrouch);
		EIC->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AProject_GMCharacter::StopCrouch);
		
		FlyComponent->SetInput(EIC);
	}
}