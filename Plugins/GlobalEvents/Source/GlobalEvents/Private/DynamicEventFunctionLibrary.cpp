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
#include "DynamicEventFunctionLibrary.h"
#include "DynamicEventContext.h"
#include "GameEventSubsystem.h"

const FName UDynamicEventFunctionLibrary::PushDynamicEventFunctionName = GET_FUNCTION_NAME_CHECKED(UDynamicEventFunctionLibrary, PushDynamicEventParam);

bool UDynamicEventFunctionLibrary::BroadcastEvent(FName EventName, UDynamicEventContext* Context)
{
    check(Context != nullptr);

    UGameEventSubsystem* Subsystem = UGameEventSubsystem::GetInstance(Context);

    return Subsystem != nullptr && Subsystem->BroadcastDynamic(EventName, Context);
}

void UDynamicEventFunctionLibrary::PushDynamicEventParam(UDynamicEventContext* Context, const int32& Value)
{
    // it will not be called.
    check(0);
}

DEFINE_FUNCTION(UDynamicEventFunctionLibrary::execPushDynamicEventParam)
{
    P_GET_OBJECT(UDynamicEventContext, InputContext);
        
    Stack.StepCompiledIn<FProperty>(nullptr);
    void* SrcAddress = Stack.MostRecentPropertyAddress;

    FProperty* SrcProperty = Stack.MostRecentProperty;
    checkSlow(SrcProperty != nullptr);

    P_FINISH;

    P_NATIVE_BEGIN;
    if (InputContext != nullptr && SrcProperty != nullptr)
    {
        InputContext->Add(SrcProperty, SrcAddress);
    }
    P_NATIVE_END;
}

bool UDynamicEventFunctionLibrary::RegisterGlobalEvent(FName EventName, UObject* Target, FName FunctionName)
{
    check(Target != nullptr);

    UGameEventSubsystem* Subsystem = UGameEventSubsystem::GetInstance(Target);

    return Subsystem != nullptr && Subsystem->Register(EventName, Target, FunctionName).IsValid();
}

bool UDynamicEventFunctionLibrary::UnRegisterGlobalEvent(FName EventName, UObject* Target, FName FunctionName)
{
    check(Target != nullptr);

    UGameEventSubsystem* Subsystem = UGameEventSubsystem::GetInstance(Target);

    return Subsystem != nullptr && Subsystem->UnRegister(EventName, Target, FunctionName);
}

