// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "ElectrifiedZone.generated.h"

UCLASS()
class PROJECT_GM_API AElectrifiedZone : public AActor
{
	GENERATED_BODY()
	
public:	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	float Effect_Time_Duration = 2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	float Time_Before_Starting_Effect_Again = 4;
	
	// Sets default values for this actor's properties
	AElectrifiedZone();

protected:
	
	float current_Effect_Time;
	float current_Time_Before_Starting_Effect_Again;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ACharacter> OverlappedCharacter; 
	
	UFUNCTION()
	static void ApplyElectrifiedEffect(ACharacter* Character);
	UFUNCTION()
	static void RemovedElectrifiedEffect(ACharacter* Character);
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
