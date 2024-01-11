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
#include "DynamicTuple.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DynamicEventFunctionLibrary.generated.h"

class UDynamicEventContext;

UCLASS()
class GLOBALEVENTS_API UDynamicEventFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    // Send a message, the parameters of this message need to be packaged into UDynamicEventContext
	UFUNCTION(BlueprintCallable, Category = "Global Events", meta=(BlueprintInternalUseOnly = "true"))
	static bool BroadcastEvent(FName EventName, UDynamicEventContext* Context);

    // register a global event
	UFUNCTION(BlueprintCallable, Category = "Global Events", meta = (DefaultToSelf = "Target"))
	static bool RegisterGlobalEvent(FName EventName, UObject* Target, FName FunctionName);

    // unregister a global event
	UFUNCTION(BlueprintCallable, Category = "Global Events", meta = (DefaultToSelf = "Target"))
	static bool UnRegisterGlobalEvent(FName EventName, UObject* Target, FName FunctionName);

	static const FName PushDynamicEventFunctionName;
private:
    // Internally used interface to dynamically add parameters to UDynamicEventContext
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(CustomStructureParam="Value", AutoCreateRefTerm = "Value", DisplayName="Add Dynamic Event Param", BlueprintInternalUseOnly = "true"))
	static void PushDynamicEventParam(UDynamicEventContext* Context, const int32& Value);
	DECLARE_FUNCTION(execPushDynamicEventParam);	
};

