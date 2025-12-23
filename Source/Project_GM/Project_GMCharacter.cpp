#include "Project_GMCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Blueprint/UserWidget.h"
#include "CharacterComponents/DashComponent.h"
#include "CharacterComponents/WallMechanicsComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AProject_GMCharacter::AProject_GMCharacter()
{
	// Enable tick for sliding logic monitoring
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Disable controller rotation affecting the character mesh directly
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

	// Movement defaults
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	
	// Enable Double Jump
	JumpMaxCount = 2;

	// Store default physics values for reset logic later
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	DefaultCrouchedWalkSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched;

	// Enable crouching capability on the movement component
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Create a camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; 
	CameraBoom->bUsePawnControlRotation = true; 

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false;
}

void AProject_GMCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Cache default movement values to restore them later
	if (GetCharacterMovement())
	{
		DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
		DefaultBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;
		DefaultMaxAcceleration = GetCharacterMovement()->MaxAcceleration;
	}
	
	// Save the initial rotation rate set in the CharacterMovement Component
	DefaultRotationRate = GetCharacterMovement()->RotationRate.Yaw;
	
	// Initialize stamina to max on spawn
	CurrentStamina = MaxStamina;
	
	// --- UI INITIALIZATION ---
	// Only create HUD for the local player
	if (IsLocallyControlled() && HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}
}

void AProject_GMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Monitor sliding state
	if (bIsSliding)
	{
		float GroundSpeed = GetVelocity().Size2D();

		// Stop sliding if speed drops too low or player is no longer grounded
		if (GroundSpeed < MinSlideSpeed || !GetCharacterMovement()->IsMovingOnGround())
		{
			StopSlide();
		}
	}
	
	if (GetCharacterMovement()->CurrentFloor.HitResult.PhysMaterial.IsValid())
	{
		// Otteniamo il tipo di superficie
		EPhysicalSurface SurfaceType = GetCharacterMovement()->CurrentFloor.HitResult.PhysMaterial->SurfaceType;

		if (SurfaceType == SurfaceType2) 
		{
			// Fisica Ghiaccio: Scivoloso, bassa accelerazione
			GetCharacterMovement()->BrakingDecelerationWalking = 0.0f;
			GetCharacterMovement()->MaxAcceleration = IceMaxAcceleration;
			
			// --- AGGIUNTA: Riduci velocità di rotazione su ghiaccio ---
			GetCharacterMovement()->RotationRate.Yaw = IceRotationRate;
            
			// Logica gravità su pendenza ghiacciata (Codice che avevi nel file originale)
			FHitResult FloorHit = GetCharacterMovement()->CurrentFloor.HitResult;
			if (FloorHit.ImpactNormal.Z < 1.0f) 
			{
				FVector SlopeForce = FVector(FloorHit.ImpactNormal.X, FloorHit.ImpactNormal.Y, 0.0f);
				GetCharacterMovement()->AddForce(SlopeForce * IceSlopeGravityForce * DeltaTime); 
			}
		}
		else
		{
			// Ripristino fisica Normale (SOLO se non siamo sotto Risucchio)
			// Se siamo sotto risucchio, è la SuctionZone a comandare questi valori.
			if (!bIsUnderSuction) 
			{
				GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration; 
				GetCharacterMovement()->MaxAcceleration = DefaultMaxAcceleration;
				
				// --- AGGIUNTA: Ripristina rotazione normale ---
				GetCharacterMovement()->RotationRate.Yaw = DefaultRotationRate;
			}
		}
	}
	// Caso in cui saltiamo o siamo in aria (PhysMaterial non valido)
	else if (!bIsUnderSuction)
	{
		// Assicuriamoci che la rotazione sia normale anche in aria/atterraggio
		GetCharacterMovement()->RotationRate.Yaw = DefaultRotationRate;
	}
	
	if (!bIsUnderSuction && CurrentStamina < MaxStamina)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenRate * DeltaTime), 0.0f, MaxStamina);
	}
	
	// Check surface type every frame
	HandleGroundPhysics();
}

void AProject_GMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jump is bound to DoJumpStart to allow intercepting input for Wall Jump check
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AProject_GMCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProject_GMCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProject_GMCharacter::Look);

		// Dashing
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AProject_GMCharacter::OnDash);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AProject_GMCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AProject_GMCharacter::StopSprint);

		// Crouching and Sliding
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AProject_GMCharacter::StartCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AProject_GMCharacter::StopCrouch);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AProject_GMCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AProject_GMCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// --- MOVEMENT ABILITIES & LOGIC ---

void AProject_GMCharacter::DoJumpStart()
{
	// Try to find the Wall Mechanics Component
	UWallMechanicsComponent* WallMechComp = FindComponentByClass<UWallMechanicsComponent>();

	// Attempt Wall Jump if component exists
	if (WallMechComp && WallMechComp->AttemptWallJump())
	{
		// Reset jump count on successful wall jump
		JumpCurrentCount = 0;
		return;
	}

	// Fallback to standard jump behavior
	Jump();
}

void AProject_GMCharacter::OnDash()
{
	// Prevent dashing while on the ground to allow Sprint input priority
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	// Try to find the Dash Component representing the unlockable ability
	UDashComponent* DashComp = FindComponentByClass<UDashComponent>();

	if (DashComp)
	{
		DashComp->PerformDash();
	}
}

void AProject_GMCharacter::StartSprint()
{
	// Only allow sprint if not currently crouching or sliding
	if (!bIsCrouched && !bIsSliding)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void AProject_GMCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

void AProject_GMCharacter::StartCrouch()
{
	float GroundSpeed = GetVelocity().Size2D();

	// Check if speed is sufficient to initiate a slide while on ground
	if (GetCharacterMovement()->IsMovingOnGround() && GroundSpeed >= MinSlideSpeed)
	{
		bIsSliding = true;
		
		// Reduce friction to allow sliding
		GetCharacterMovement()->GroundFriction = SlideFriction;
		
		// Allow high speed while crouched for the slide duration
		GetCharacterMovement()->MaxWalkSpeedCrouched = SprintSpeed; 
		
		Crouch();
	}
	else
	{
		// Perform standard crouch
		bIsSliding = false;
		Crouch();
	}
}

void AProject_GMCharacter::StopCrouch()
{
	StopSlide();
	UnCrouch();
}

void AProject_GMCharacter::StopSlide()
{
	if (bIsSliding)
	{
		bIsSliding = false;
		
		// Restore default movement values
		GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
		GetCharacterMovement()->MaxWalkSpeedCrouched = DefaultCrouchedWalkSpeed;
	}
}

void AProject_GMCharacter::HandleGroundPhysics()
{
    // Safety Check
    if (!GetCharacterMovement()) return;

    // Surface Detection (Line Trace)
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0.0f, 0.0f, 150.0f); // Trace down

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bReturnPhysicalMaterial = true;

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    
    bool bIsOnIce = false;

    // Check Material Type
    if (bHit && HitResult.PhysMaterial.IsValid())
    {
        // SurfaceType1 is "Ice" (Check Project Settings -> Physics)
        if (HitResult.PhysMaterial->SurfaceType == EPhysicalSurface::SurfaceType1)
        {
            bIsOnIce = true;
        }
    }

    // Apply Physics
    if (bIsOnIce)
    {
        // --- ICE BEHAVIOR ---
        GetCharacterMovement()->GroundFriction = 0.0f; 
        GetCharacterMovement()->BrakingDecelerationWalking = 0.0f; 
        GetCharacterMovement()->MaxAcceleration = IceMaxAcceleration; 
    	GetCharacterMovement()->RotationRate = FRotator(0.0f, IceRotationRate, 0.0f);

        // --- SLOPE GRAVITY LOGIC ---
        FVector FloorNormal = HitResult.ImpactNormal;
        if (FloorNormal.Z < 0.99f) 
        {
            FVector SlopeDir = FVector::VectorPlaneProject(FVector::DownVector, FloorNormal).GetSafeNormal();
            float Steepness = 1.0f - FloorNormal.Z;
            FVector SlideForce = SlopeDir * IceSlopeGravityForce * Steepness * GetCharacterMovement()->Mass;
            GetCharacterMovement()->AddForce(SlideForce);
        }
    }
    else
    {
        // --- NORMAL BEHAVIOR (Restore Defaults) ---
        GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
        GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;
        GetCharacterMovement()->MaxAcceleration = DefaultMaxAcceleration;
    	GetCharacterMovement()->RotationRate = FRotator(0.0f, DefaultRotationRate, 0.0f);
    }
}

void AProject_GMCharacter::ApplySuctionForce(FVector SuctionOrigin, float Strength, float DeltaTime)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

	// Update State
	bIsUnderSuction = true;
	SuctionTargetLocation = SuctionOrigin;

	FVector ActorLocation = GetActorLocation();
	FVector ToHoleVector = (SuctionOrigin - ActorLocation);
	FVector SuctionDir = ToHoleVector.GetSafeNormal();
	FVector PullForce = SuctionDir * Strength * MoveComp->Mass;
	MoveComp->AddForce(PullForce);

	// Fail state: Exhaustion
	if (CurrentStamina <= 0.0f)
	{
		MoveComp->GroundFriction = 0.0f; 
		MoveComp->BrakingDecelerationWalking = 0.0f;
		MoveComp->MaxWalkSpeed = 0.0f;
		
		MoveComp->AddInputVector(SuctionDir * 0.1f);
		return; 
	}

	// Input Analysis
	FVector InputDir = GetLastMovementInputVector();
	float Alignment = 0.0f;
	if (!InputDir.IsNearlyZero())
	{
		Alignment = FVector::DotProduct(InputDir, SuctionDir);
	}
	
	// Moving towards the force
	if (Alignment > 0.5f) 
	{
		MoveComp->GroundFriction = SuctionFriction_WithForce; 
		MoveComp->BrakingDecelerationWalking = 0.0f; 
		MoveComp->MaxWalkSpeed = SuctionMoveSpeed_WithForce; 

		// Regen Stamina
		CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenRate * DeltaTime), 0.0f, MaxStamina);
	}
	// Resisting (moving against the force)
	else if (Alignment < -0.2f) 
	{
		// Consume Stamina
		float DrainMultiplier = FMath::Abs(Alignment); 
		float ActualDrain = StaminaDrainRate * DrainMultiplier * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina - ActualDrain, 0.0f, MaxStamina);

		MoveComp->GroundFriction = SuctionFriction_AgainstForce; 
		MoveComp->BrakingDecelerationWalking = SuctionBraking_AgainstForce; 
		MoveComp->MaxWalkSpeed = SuctionMoveSpeed_AgainstForce; 
	}
	// Idle / Passive
	else 
	{
		MoveComp->GroundFriction = 0.0f; 
		MoveComp->BrakingDecelerationWalking = 0.0f;
       
		if (InputDir.IsNearlyZero())
		{
			MoveComp->AddInputVector(SuctionDir * SuctionInputForce_Passive);
		}
       
		MoveComp->MaxWalkSpeed = SuctionMoveSpeed_Passive; 
       
		// Regen Stamina
		CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenRate * DeltaTime), 0.0f, MaxStamina);
	}
}

void AProject_GMCharacter::ResetSuctionPhysics()
{
	bIsUnderSuction = false;

	// Reset movement params to defaults
	GetCharacterMovement()->GroundFriction = 8.0f; 
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}