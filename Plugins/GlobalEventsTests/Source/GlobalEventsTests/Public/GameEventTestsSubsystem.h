// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TestObject.h"
#include "GameEventTestsSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class GLOBALEVENTSTESTS_API UGameEventTestsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION()
	void OnGlobalEventReceived(const FName& InName, UDynamicEventContext* InContext);

private:
	void RegisterDebugEvent();
	void UnRegisterDebugEvent();
	void SendDebugEvent();
	void TestDynamicTuple();
	void TestReferenceParameter();

private:
	FRawTestsObject RawObj;
	TSharedPtr<FRawTestsObject> RawObjPtr;

	UPROPERTY()
	UTestObject* TestsObj;
};
