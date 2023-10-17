/*
    MIT License

    Copyright (c) 2023 GlobalEvents Plugin For UnrealEngine

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Project URL: https://github.com/bodong1987/UnrealEngine.GlobalEvents
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Inline/EventCenterPrerequirements.inl"
#include "GameEventSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class GLOBALEVENTS_API UGameEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;

	static UGameEventSubsystem* GetInstance(const UObject* InContext);

#define ENABLE_EVENT_CENTER_ON_RECEIVE_GLOBAL_EVENT

public:
	UPROPERTY(BlueprintAssignable)
	FGlobalEventDelegateType           OnReceiveGlobalEvent;

public:
#if CPP
#include "Inline/EventCenterDataInterfacesInline.inl"
#include "Inline/EventCenterCommonInterfacesInline.inl"
#include "Inline/EventCenterTypeSafeInterfacesInline.inl"
#include "Inline/EventCenterDynamicInterfacesInline.inl"
#include "Inline/EventCenterSignatureInterfacesInline.inl"
#endif

#undef ENABLE_EVENT_CENTER_ON_RECEIVE_GLOBAL_EVENT
};

