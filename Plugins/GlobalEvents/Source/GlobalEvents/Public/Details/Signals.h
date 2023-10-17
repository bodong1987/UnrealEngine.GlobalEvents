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

#include "GlobalEventsLog.h"
#include "Details/EventObserverInterfaces.h"
#include "SignalInterface.h"

namespace UE
{
    namespace GlobalEvents
    {
        namespace Details
        {
            class IEventObserver;

            enum class ESignalInvokeType
            {
                Static,
                Dynamic
            };

            class GLOBALEVENTS_API FBaseSignal : public ISignal
            {
            public:
                typedef TArray<TSharedPtr<IEventObserver>>      DelegateListType;
                FBaseSignal();
                FBaseSignal(FBaseSignal&& InSignal);

                virtual bool IsLocked() const override;
                virtual void DisconnectAll() override;
                virtual FDelegateHandle Connect(IEventObserver* InInstance) override;
                virtual bool Disconnect(IEventObserver* InInstance) override;
                virtual bool Disconnect(FDelegateHandle InHandle) override;
                virtual bool IsEmpty() const override;
                virtual int  Num() const override;

            private:
                template <typename DisconnectEvaluatorType>
                bool DisconnectImpl(DisconnectEvaluatorType&& InEvaluator);

                void Lock();
                void UnLock();

#if ENGINE_MAJOR_VERSION < 5
            private:
                template <typename... Types>
                struct TypeList
                {
                };

                template <int Index, typename TypeList>
                struct TypeAt;

                template <typename Head, typename... Tail>
                struct TypeAt<0, TypeList<Head, Tail...>>
                {
                    using Type = Head;
                };

                template <int Index, typename Head, typename... Tail>
                struct TypeAt<Index, TypeList<Head, Tail...>>
                {
                    using Type = typename TypeAt<Index - 1, TypeList<Tail...>>::Type;
                };
#endif

            protected:
                struct GLOBALEVENTS_API FUnLockHelper
                {
                    FBaseSignal* Owner;

                    FUnLockHelper(FBaseSignal* InOwner) :
                        Owner(InOwner)
                    {
                        Owner->Lock();
                    }

                    ~FUnLockHelper()
                    {
                        Owner->UnLock();
                    }
                };

                template <size_t Index = 0, typename... ParamTypes, typename... ArgTypes>
                typename TEnableIf<Index == sizeof...(ParamTypes), void>::Type
                    MoveLeftReferenceBack(TTuple<ParamTypes...>&, TTuple<typename TDecay<ParamTypes>::Type...>&, ArgTypes&...)
                {
                }

                template <size_t Index = 0, typename... ParamTypes, typename... ArgTypes>
                typename TEnableIf < Index < sizeof...(ParamTypes), void>::Type
                    MoveLeftReferenceBack(TTuple<ParamTypes...>& InParamsRef, TTuple<typename TDecay<ParamTypes>::Type...>& InTempTupleRef, ArgTypes&... Args)
                {
                    #if ENGINE_MAJOR_VERSION >= 5
                    using Type = typename TTupleElement<Index, TTuple<ParamTypes...>>::Type;
                    #else
                    using Type = typename TypeAt<Index, TypeList<ParamTypes...>>::Type;
                    #endif

                    if constexpr (TIsNonConstLValueReference<Type>::Value)
                    {
                        InParamsRef.template Get<Index>() = MoveTemp(InTempTupleRef.template Get<Index>());
                    }

                    MoveLeftReferenceBack<Index + 1>(InParamsRef, InTempTupleRef, Args...);
                }

                inline bool IsTargetsEmpty() const { return Targets.Num() == 0; }

                template <typename... ParamTypes>
                void RaiseEventInternal(ParamTypes... InParams)
                {
                    if (IsTargetsEmpty())
                    {
                        return;
                    }

                    FUnLockHelper Helper(this);

                    TOptional<TTuple<typename TDecay<ParamTypes>::Type...>> Stack;

                    constexpr bool bNeedWriteBack = THasNonConstLValueReference<ParamTypes...>::Value;
                    bool MaybeChanged = false;

                    for (int32 i = 0; i < Targets.Num(); ++i)
                    {
                        // don't use reference here
                        auto Instance = Targets[i];

                        if (!Instance->IsPendingDestroy())
                        {
                            if (Instance->IsGeneric())
                            {
                                ((TBaseEventObserver<ParamTypes...>*)Instance.Get())->Invoke(InParams...);

                                MaybeChanged = bNeedWriteBack;
                            }
                            else
                            {
                                if ((bNeedWriteBack && MaybeChanged) || !Stack.IsSet())
                                {
                                    Stack = TTuple<typename TDecay<ParamTypes>::Type...>{ InParams... };
                                }

                                Instance.Get()->ExecuteInvoke(&Stack.GetValue());

                                if constexpr (bNeedWriteBack)
                                {
                                    // write tuple reference values to InParams
                                    TTuple<ParamTypes...> ParamsTuple(InParams...);
                                    MoveLeftReferenceBack(ParamsTuple, Stack.GetValue(), InParams...);

                                    MaybeChanged = false;
                                }
                            }
                        }
                    }
                }
            protected:
                DelegateListType                            Targets;
            private:
                bool                                        LockedFlag = false;
            };

            template <typename... ParamTypes>
            class TSignal : public FBaseSignal
            {
            public:
                typedef TBaseEventObserver<ParamTypes...>              ImplEventObserverType;
                typedef FBaseSignal Super;

                // if you get an compile error here
                // it means you use an unsupported type in global event systems
                static_assert(TIsSupportedTypes<ParamTypes...>::Value, "Don't use unsupported type!!!");

                virtual const ISignature* GetSignature() const override
                {
                    return StaticSignature();
                }

                static const ISignature* StaticSignature()
                {
                    static const TGenericSignature<ParamTypes...> Z_StaticSignature;
                    return &Z_StaticSignature;
                }

                virtual int GetInvokeType() const override
                {
                    return (int)ESignalInvokeType::Static;
                }

                void RaiseEvent(ParamTypes... InParams)
                {
                    Super::template RaiseEventInternal<ParamTypes...>(InParams...);
                }

                virtual void ExecuteRaiseEvent(const void* InParams) override
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
            };

            class GLOBALEVENTS_API FBaseDynamicSignal : public FBaseSignal
            {
            public:
                typedef FBaseSignal Super;

                FBaseDynamicSignal();
                FBaseDynamicSignal(FBaseDynamicSignal&& InSignal);

                template <typename... ParamTypes>
                void RaiseEvent(ParamTypes... InParams)
                {
                    FBaseSignal::template RaiseEventInternal<ParamTypes...>(InParams...);
                }

                virtual int GetInvokeType() const override;
                virtual void ExecuteRaiseEvent(const void* InParams) override;
            };

            class GLOBALEVENTS_API FUFunctionSignal : public FBaseDynamicSignal
            {
            public:
                typedef FBaseDynamicSignal Super;

                FUFunctionSignal(const UFunction* InFunction);
                FUFunctionSignal(const FUFunctionSignature& InSignature);
                FUFunctionSignal(FUFunctionSignal&& InSignal);

                virtual const ISignature* GetSignature() const override;

            private:
                FUFunctionSignature               Signature;
            };

            class GLOBALEVENTS_API FAnonymousSignal : public FBaseDynamicSignal
            {
            public:
                typedef FBaseDynamicSignal Super;

                FAnonymousSignal();
                FAnonymousSignal(FDynamicSignature&& InSignature);
                FAnonymousSignal(FAnonymousSignal&& InSignal);

                virtual const ISignature* GetSignature() const override;

            private:
                FDynamicSignature                  Signature;
            };

            template <typename... ParamTypes>
            class TSignalInvoker
            {
            public:
                static bool Invoke(TSharedPtr<ISignal> InSignal, const FName& InEventName, ParamTypes... InParams)
                {
                    if (InSignal)
                    {
                        static const TGenericSignature<ParamTypes...> s_Signature;

                        // exists signal's parameters must be convertible from broadcast parameters
                        if (!InSignal->GetSignature()->CheckInvokeableFrom(&s_Signature))
                        {
                            UE_LOG(GlobalEventsLog, Error,
                                TEXT("Invalid Operation, failed convert signature. EventName = (%s), Signal Signature = (%s), Broadcast Signature = (%s)"),
                                *InEventName.ToString(),
                                *InSignal->GetSignature()->ToString(),
                                *s_Signature.ToString()
                            );

                            return false;
                        }

                        if (InSignal->GetInvokeType() == (int)UE::GlobalEvents::Details::ESignalInvokeType::Static)
                        {
                            TSharedPtr<UE::GlobalEvents::Details::TSignal<ParamTypes...>> Signal = StaticCastSharedPtr<UE::GlobalEvents::Details::TSignal<ParamTypes...>>(InSignal);

                            Signal->RaiseEvent(InParams...);
                        }
                        else
                        {
                            TSharedPtr<UE::GlobalEvents::Details::FBaseDynamicSignal> Signal = StaticCastSharedPtr< UE::GlobalEvents::Details::FBaseDynamicSignal>(InSignal);

                            Signal->template RaiseEvent<ParamTypes...>(InParams...);
                        }

                        return true;
                    }

                    return false;
                }
            };
        }
    }
}


