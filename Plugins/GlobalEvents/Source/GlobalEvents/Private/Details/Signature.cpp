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
#include "Details/Signature.h"
#include "GlobalEventsLog.h"

namespace UE
{
    namespace GlobalEvents
    {
        namespace Details
        {
            FBaseSignature::FBaseSignature()
            {
            }

            FBaseSignature::FBaseSignature(const FBaseSignature& InBaseSignature) :
                Signature(InBaseSignature.Signature),
                Parameters(InBaseSignature.Parameters)
            {
            }

            FBaseSignature::FBaseSignature(FBaseSignature&& InBaseSignature) :
                Signature(MoveTemp(InBaseSignature.Signature)),
                Parameters(MoveTemp(InBaseSignature.Parameters))
            {
            }

            FBaseSignature& FBaseSignature::operator = (FBaseSignature&& InBaseSignature)
            {
                Signature = MoveTemp(InBaseSignature.Signature);
                Parameters = MoveTemp(InBaseSignature.Parameters);

                return *this;
            }

            const FName& FBaseSignature::GetName() const
            {
                return Signature;
            }

            const TArray<FGlobalEventParamType>& FBaseSignature::GetParameters() const
            {
                return Parameters;
            }

            bool FBaseSignature::EqualTo(const ISignature* InOtherSignature) const
            {
                const bool bEqualToFastResult = EqualToFast(InOtherSignature);

#if !UE_BUILD_SHIPPING
                const bool bEqualToSlowAndSafe = EqualToSlowAndSafe(InOtherSignature);

                checkf(bEqualToSlowAndSafe == bEqualToFastResult,
                    TEXT("Signature slow check failure. (%s) != (%s)"),
                    *ToString(),
                    *InOtherSignature->ToString()
                );
#endif

                return bEqualToFastResult;
            }

            bool FBaseSignature::EqualToSlowAndSafe(const ISignature* InOtherSignature) const
            {
                if (InOtherSignature == nullptr)
                {
                    return false;
                }

                const TArray<FGlobalEventParamType>& OtherParameters = InOtherSignature->GetParameters();

                if (OtherParameters.Num() != Parameters.Num())
                {
                    return false;
                }

                for (int32 i = 0; i < Parameters.Num(); ++i)
                {
                    if (Parameters[i] != OtherParameters[i])
                    {
                        return false;
                    }
                }

                return true;
            }

            bool FBaseSignature::EqualToFast(const ISignature* InOtherSignature) const
            {
                return InOtherSignature != nullptr && GetName() == InOtherSignature->GetName();
            }

            bool FBaseSignature::CheckInvokeableFrom(const ISignature* InInvokerSignature) const
            {
                if (InInvokerSignature == nullptr)
                {
                    return false;
                }

                const TArray<FGlobalEventParamType>& InvokerSourceParameters = InInvokerSignature->GetParameters();

                if (InvokerSourceParameters.Num() != Parameters.Num())
                {
                    return false;
                }

                for (int32 i = 0; i < Parameters.Num(); ++i)
                {
                    if (!Parameters[i].CheckInvokeConvertibleFrom(InvokerSourceParameters[i]))
                    {
                        return false;
                    }
                }

                return true;
            }

            void FBaseSignature::Clear()
            {
                Signature = FName();
                Parameters.Empty();
            }

            FDynamicSignature::FDynamicSignature()
            {
            }

            FDynamicSignature::FDynamicSignature(const FDynamicSignature& InOtherSignature) :
                Super(InOtherSignature)
            {
            }

            FDynamicSignature::FDynamicSignature(FDynamicSignature&& InOtherSignature) :
                Super(InOtherSignature)
            {
            }

            FDynamicSignature& FDynamicSignature::operator = (FDynamicSignature&& InOtherSignature)
            {
                Super::operator = (MoveTemp(InOtherSignature));

                return *this;
            }

            bool FDynamicSignature::IsValid() const
            {
                return true;
            }

            void FDynamicSignature::Add(FGlobalEventParamType&& InParamType)
            {
                AppendTypeString(InParamType.GetName());

                Parameters.Emplace(InParamType);
            }

            void FDynamicSignature::Add(const FGlobalEventParamType& InParamType)
            {
                AppendTypeString(InParamType.GetName());

                Parameters.Add(InParamType);
            }

            void FDynamicSignature::AppendTypeString(const FName& InTypeName)
            {
                FString Text;
                if (Signature != NAME_None)
                {
                    Signature.ToString(Text);
                    Text.Append(TEXT(", "));
                }

                Text.Append(InTypeName.ToString());

                Signature = *Text;
            }


            FUFunctionSignature::FUFunctionSignature() :
                Function(nullptr)
            {
            }

            FUFunctionSignature::FUFunctionSignature(const UFunction* InFunction) :
                Function(nullptr)
            {
                BuildSignature(InFunction);
            }

            FUFunctionSignature::FUFunctionSignature(const FUFunctionSignature& InSignature) :
                Super(InSignature),
                Function(InSignature.Function)
            {
            }

            FUFunctionSignature::FUFunctionSignature(FUFunctionSignature&& InSignature) :
                Super(MoveTemp(InSignature)),
                Function(InSignature.Function)
            {
            }

            FUFunctionSignature& FUFunctionSignature::operator = (FUFunctionSignature&& InSignature)
            {
                Super::operator = (MoveTemp(InSignature));

                Function = InSignature.Function;

                InSignature.Function = nullptr;

                return *this;
            }

            bool FUFunctionSignature::IsValid() const
            {
                return Function != nullptr;
            }

            void FUFunctionSignature::Clear()
            {
                Super::Clear();

                Function = nullptr;
            }

            bool FUFunctionSignature::BuildSignature(const UFunction* InFunction)
            {
                FString TempSignature;

                for (TFieldIterator<FProperty> It(InFunction); It; ++It)
                {
                    FProperty* Property = *It;

                    if (Property->HasAnyPropertyFlags(CPF_Parm))
                    {
                        if (Property->HasAnyPropertyFlags(CPF_ReturnParm))
                        {
                            return false;
                        }

                        if (!AppendType(TempSignature, Property))
                        {
                            UE_LOG(GlobalEventsLog,
                                Error,
                                TEXT("%s is not an valid Event listener. need no return value, no unsupported parameter. unsupported param is:%s %s"),
                                *InFunction->GetFullName(),
                                *Property->GetCPPType(),
                                *Property->GetName()
                            );

                            return false;
                        }
                    }
                }

                Function = InFunction;
                Signature = *TempSignature;

                return Function != nullptr;
            }

            inline FString AdjustPropertyTypeName(FProperty* InProperty)
            {
                return InProperty->GetCPPType();
            }

            bool FUFunctionSignature::AppendType(FString& InSignatureRef, FProperty* InProperty)
            {
                // check property type
                checkSlow(InProperty != nullptr);

                FString TypeName = AdjustPropertyTypeName(InProperty);

                if (!InSignatureRef.IsEmpty())
                {
                    InSignatureRef.Append(TEXT(", "));
                }

                InSignatureRef.Append(TypeName);

                Parameters.Emplace(InProperty);

                return true;
            }
        }
    }
}

