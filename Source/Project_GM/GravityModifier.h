// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GravityModifier.generated.h"

UCLASS()
class PROJECT_GM_API AGravityModifier : public AActor
{
	GENERATED_BODY()
	
public:	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBillboardComponent> Billboard; 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> Box; 
	
	UPROPERTY(BlueprintReadOnly, Category = "Gravity Settings")
	TObjectPtr<UTexture2D> ZeroGravityTexture; 
	UPROPERTY(BlueprintReadOnly, Category = "Gravity Settings")
	TObjectPtr<UTexture2D> PositiveGravityTexture; 
	UPROPERTY(BlueprintReadOnly, Category = "Gravity Settings")
	TObjectPtr<UTexture2D> NegativeGravityTexture; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gravity Settings")
	float GravityScale;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gravity Settings")
	float WalkableFloorZ;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gravity Settings")
	float WalkingSpeedScale;
	
	
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Sets default values for this actor's properties
	AGravityModifier();
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

//public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
	
private:
	
	float DefaultGravityScale;
	float DefaultWalkableFloorZ;
};
