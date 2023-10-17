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
#include "Details/Signals.h"
#include "Details/EventObservers.h"
#include "GlobalEventsLog.h"

namespace UE
{
    namespace GlobalEvents
    {
        namespace Details
        {
            FBaseSignal::FBaseSignal()
            {
            }

            FBaseSignal::FBaseSignal(FBaseSignal&& InSignal) :
                Targets(MoveTemp(InSignal.Targets)),
                LockedFlag(InSignal.LockedFlag)
            {
            }

            bool FBaseSignal::IsLocked() const
            {
                return LockedFlag;
            }

            bool FBaseSignal::IsEmpty() const
            {
                return IsTargetsEmpty();
            }

            int FBaseSignal::Num() const
            {
                return Targets.Num();
            }

            void FBaseSignal::DisconnectAll()
            {
                if (LockedFlag)
                {
                    for (auto& Instance : Targets)
                    {
                        Instance->SetPendingDestroy(true);
                    }
                }
                else
                {
                    Targets.Empty();
                }
            }

            FDelegateHandle FBaseSignal::Connect(IEventObserver* InInstance)
            {
                check(InInstance != nullptr);

                if (!InInstance->GetSignature()->CheckInvokeableFrom(GetSignature()))
                {
                    UE_LOG(GlobalEventsLog, Warning, TEXT("Failed connect signal(%s) with delegate(%s)"), *GetSignature()->ToString(), *InInstance->GetSignature()->ToString());

                    return FDelegateHandle();
                }

                for (int32 i = 0; i < Targets.Num(); ++i)
                {
                    auto& Instance = Targets[i];

                    // valid instance is already exists, so skip add new 
                    if (!Instance->IsPendingDestroy() && Instance->EqualTo(InInstance))
                    {
                        return FDelegateHandle();
                    }
                }

                // push to ends
                auto EventObserver = TSharedPtr<IEventObserver>((IEventObserver*)InInstance->CloneAndMove());
                Targets.Push(EventObserver);

                return EventObserver->GetHandle();
            }

            bool FBaseSignal::Disconnect(IEventObserver* InInstance)
            {
                return DisconnectImpl([=](TSharedPtr<IEventObserver>& InstanceRef)
                    {
                        return InstanceRef->EqualTo(InInstance);
                    });
            }

            bool FBaseSignal::Disconnect(FDelegateHandle InHandle)
            {
                return DisconnectImpl([=](TSharedPtr<IEventObserver>& InstanceRef)
                    {
                        return InstanceRef->GetHandle() == InHandle;
                    });
            }

            void FBaseSignal::Lock()
            {
                check(!LockedFlag);

                LockedFlag = true;
            }

            void FBaseSignal::UnLock()
            {
                check(LockedFlag);

                LockedFlag = false;

                Targets.RemoveAll(
                    [](TSharedPtr<IEventObserver>& InPtr)
                    {
                        return InPtr->IsPendingDestroy();
                    }
                );
            }

            template <typename DisconnectEvaluatorType>
            bool FBaseSignal::DisconnectImpl(DisconnectEvaluatorType&& InEvaluator)
            {
                for (int32 i = 0; i < Targets.Num();)
                {
                    // don't use reference here
                    auto Instance = Targets[i];

                    // if current instance is pending destroy and not locked, remove it directly.
                    const bool IsPendingDestroyFlag = Instance->IsPendingDestroy();
                    if (IsPendingDestroyFlag && !LockedFlag)
                    {
                        Targets.RemoveAt(i);
                        continue;
                    }

                    // if we find an valid instance, try compare it
                    if (!IsPendingDestroyFlag && InEvaluator(Instance))
                    {
                        // try to remove it
                        if (LockedFlag)
                        {
                            Instance->SetPendingDestroy(true);
                            ++i;
                        }
                        else
                        {
                            Targets.RemoveAt(i);
                        }

                        return true;
                    }
                    else
                    {
                        ++i;
                    }
                }

                return false;
            }

            FBaseDynamicSignal::FBaseDynamicSignal()
            {
            }

            FBaseDynamicSignal::FBaseDynamicSignal(FBaseDynamicSignal&& InSignal) :
                Super(MoveTemp(InSignal))
            {
            }

            int FBaseDynamicSignal::GetInvokeType() const
            {
                return (int)ESignalInvokeType::Dynamic;
            }

            void FBaseDynamicSignal::ExecuteRaiseEvent(const void* InParams)
            {
                if (IsTargetsEmpty())
                {
                    return;
                }

                FUnLockHelper Helper(this);

                for (int32 i = 0; i < Targets.Num(); ++i)
                {
                    // don't use reference here
                    auto Instance = Targets[i];

                    if (!Instance->IsPendingDestroy())
                    {
                        (Instance.Get())->ExecuteInvoke(InParams);
                    }
                }
            }

            FUFunctionSignal::FUFunctionSignal(const UFunction* InFunction) :
                Signature(InFunction)
            {
            }

            FUFunctionSignal::FUFunctionSignal(const FUFunctionSignature& InSignature) :
                Signature(InSignature)
            {
            }

            FUFunctionSignal::FUFunctionSignal(FUFunctionSignal&& InSignal) :
                Super(MoveTemp(InSignal)),
                Signature(MoveTemp(InSignal.Signature))
            {
            }

            const ISignature* FUFunctionSignal::GetSignature() const
            {
                return &Signature;
            }

            FAnonymousSignal::FAnonymousSignal()
            {
            }

            FAnonymousSignal::FAnonymousSignal(FDynamicSignature&& InSignature) :
                Signature(MoveTemp(InSignature))
            {
            }

            FAnonymousSignal::FAnonymousSignal(FAnonymousSignal&& InSignal) :
                Super(MoveTemp(InSignal)),
                Signature(MoveTemp(InSignal.Signature))
            {
            }

            const ISignature* FAnonymousSignal::GetSignature() const
            {
                return &Signature;
            }
        }
    }
}
