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

#include "Details/TypeInfo.h"
#include "SignatureInterface.h"

namespace UE
{
    namespace GlobalEvents
    {
        namespace Details
        {
            class GLOBALEVENTS_API FBaseSignature : public ISignature
            {
            public:
                FBaseSignature();
                FBaseSignature(FBaseSignature&& InBaseSignature);
                FBaseSignature(const FBaseSignature& InBaseSignature);

                FBaseSignature& operator = (FBaseSignature&& InBaseSignature);

                virtual const FName& GetName() const override;
                virtual bool EqualTo(const ISignature* InOtherSignature) const override;
                virtual const TArray<FGlobalEventParamType>& GetParameters() const override;
                virtual bool CheckInvokeableFrom(const ISignature* InInvokerSignature) const override;

                inline int32 Num() const { return Parameters.Num(); }
                inline bool  IsEmpty() const { return Parameters.Num() == 0; }
                virtual void Clear() override;

                template <typename T>
                static FGlobalEventParamType ConstructGlobalEventParamType()
                {
                    int Flags = 0;

                    if constexpr (TIsReferenceType<T>::Value)
                    {
                        Flags |= (int)EGlobalEventParameterFlags::Reference;
                    }

                    if constexpr (TIsPointer<T>::Value)
                    {
                        Flags |= (int)EGlobalEventParameterFlags::Pointer;
                    }

                    if constexpr (TIsConst<T>::Value)
                    {
                        Flags |= (int)EGlobalEventParameterFlags::Constant;
                    }

                    if constexpr (TIsTArray<typename TDecay<T>::Type>::Value)
                    {
                        Flags |= (int)EGlobalEventParameterFlags::Array;
                    }

                    return FGlobalEventParamType(
                        UE::GlobalEvents::Details::TTypeInfo<typename TDecay<T>::Type>::GetTypeName(),
                        UE::GlobalEvents::Details::TTypeInfo<typename TDecay<T>::Type>::GetTypeId(),
                        Flags,
                        UE::GlobalEvents::Details::TTypeInfo<typename TDecay<T>::Type>::GetFieldType()
                    );
                }

            protected:
                bool EqualToSlowAndSafe(const ISignature* InOtherSignature) const;
                bool EqualToFast(const ISignature* InOtherSignature) const;

            protected:
                FName                          Signature;
                TArray<FGlobalEventParamType>  Parameters;
            };

            class GLOBALEVENTS_API FDynamicSignature : public FBaseSignature
            {
            public:
                typedef FBaseSignature Super;

                FDynamicSignature();
                FDynamicSignature(const FDynamicSignature& InOtherSignature);
                FDynamicSignature(FDynamicSignature&& InOtherSignature);

                FDynamicSignature& operator = (FDynamicSignature&& InOtherSignature);

                virtual bool IsValid() const override;

            public:
                template <typename T>
                void Add()
                {
                    Add(Super::ConstructGlobalEventParamType<T>());
                }

                void Add(const FGlobalEventParamType& InParamType);
                void Add(FGlobalEventParamType&& InParamType);

                inline const FGlobalEventParamType& GetParamType(int InIndex) const { return Parameters[InIndex]; }

            private:
                void AppendTypeString(const FName& InTypeName);
            };

            template <typename... ParamTypes>
            class TGenericSignature : public FBaseSignature
            {
            public:
                typedef FBaseSignature Super;

                TGenericSignature()
                {
                    Signature = *BuildSignature();
                }

                virtual bool IsValid() const override { return true; }

            private:
                template <typename T, typename... ExtraParamTypes>
                void AppendTypeInfo(FString& InTarget)
                {
                    InTarget.Append(TTypeInfo<typename TDecay<T>::Type>::GetTypeName());

                    Parameters.Emplace(Super::ConstructGlobalEventParamType<T>());

                    if constexpr (sizeof...(ExtraParamTypes) > 0)
                    {
                        InTarget.Append(", ");
                        AppendTypeInfo<ExtraParamTypes...>(InTarget);
                    }
                }

                FString BuildSignature()
                {
                    FString TempSignature;

                    if constexpr (sizeof...(ParamTypes) > 0)
                    {
                        AppendTypeInfo<ParamTypes...>(TempSignature);
                    }

                    return TempSignature;
                }
            };

            class GLOBALEVENTS_API FUFunctionSignature : public FBaseSignature
            {
            public:
                typedef FBaseSignature Super;

                FUFunctionSignature();
                FUFunctionSignature(const UFunction* InFunction);
                FUFunctionSignature(const FUFunctionSignature& InSignature);
                FUFunctionSignature(FUFunctionSignature&& InSignature);

                virtual bool IsValid() const override;

                inline const UFunction* GetFunction() const { return Function.IsValid() ? Function.Get() : nullptr; }

                FUFunctionSignature& operator = (FUFunctionSignature&& InSignature);

                virtual void Clear() override;

            private:
                bool BuildSignature(const UFunction* InFunction);
                bool AppendType(FString& InSignatureRef, FProperty* InProperty);
            private:
                TWeakObjectPtr<const UFunction>       Function;
            };
        }
    }
}

