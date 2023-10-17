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
#include "DynamicTuple.h"

namespace UE
{
    namespace GlobalEvents
    {
        FDynamicTuple::FDynamicTuple()
        {
        }

        FDynamicTuple::~FDynamicTuple()
        {
            Clear();
        }

        FDynamicTuple::FDynamicTuple(FDynamicTuple&& InOther) :
            TupleBuffer(MoveTemp(InOther.TupleBuffer)),
            TupleElementRecords(MoveTemp(InOther.TupleElementRecords)),
            Signature(MoveTemp(InOther.Signature))
        {
        }

        FDynamicTuple& FDynamicTuple::operator= (FDynamicTuple&& InOther)
        {
            TupleBuffer = MoveTemp(InOther.TupleBuffer);
            TupleElementRecords = MoveTemp(InOther.TupleElementRecords);
            Signature = MoveTemp(InOther.Signature);

            return *this;
        }

        void FDynamicTuple::Clear()
        {
            for (int32 i = 0; i < TupleElementRecords.Num(); ++i)
            {
                if (TupleElementRecords[i].Deletor)
                {
                    const void* Address = TupleBuffer.GetData() + TupleElementRecords[i].Offset;

                    TupleElementRecords[i].Deletor(Address);
                }
            }

            TupleBuffer.Empty();
            TupleElementRecords.Empty();
            Signature.Clear();
        }

        void FDynamicTuple::PushInternal(int InElementSize, int InAlignmentSize, TFunction<void(const void*)>&& InCopyFunctor, TFunction<void(const void*)>&& InDeleteFunctor)
        {
            const int OrignalOffset = TupleBuffer.Num();
            const int PreAppendOffset = (int)AlignAddress(OrignalOffset, InAlignmentSize);
            const int Padding = PreAppendOffset - OrignalOffset;
            checkSlow(Padding >= 0);

            TupleBuffer.AddZeroed(InElementSize + Padding);

            const uint8* Address = TupleBuffer.GetData() + PreAppendOffset;

            // copy data to this buffer
            InCopyFunctor(Address);

            FParamRecord Record;
            Record.Offset = PreAppendOffset;
            Record.Deletor = InDeleteFunctor;

            TupleElementRecords.Emplace(Record);
        }

        void FDynamicTuple::Push(const UScriptStruct* InScriptStruct, const void* InAddress)
        {
            check(InAddress != nullptr);
            check(InScriptStruct != nullptr);

            const int32 StructSize = InScriptStruct->GetStructureSize();

            PushInternal(
                StructSize,
                InScriptStruct->GetCppStructOps()->GetAlignment(),
                [=](const void* Address)
                {
                    // must initialize
                    InScriptStruct->InitializeStruct((void*)InAddress);

                    // copy from external address
                    InScriptStruct->CopyScriptStruct((void*)Address, (void*)InAddress);
                },
                [=](const void* InPtr)
                {
                    if (InPtr != nullptr)
                    {
                        InScriptStruct->DestroyStruct((void*)InPtr);
                    }
                }
            );

            Signature.Add(FGlobalEventParamType(InScriptStruct));
        }

        void FDynamicTuple::Push(const FProperty* InProperty, const void* InSourceAddress)
        {
            check(InProperty != nullptr);
            check(InSourceAddress != nullptr);

            const int32 PropertySize = InProperty->GetSize();

            PushInternal(
                PropertySize,
                InProperty->GetMinAlignment(),
                [=](const void* Address)
                {
                    // must initialize
                    InProperty->InitializeValue((void*)Address);

                    // copy value
                    InProperty->CopyCompleteValue((void*)Address, (void*)InSourceAddress);
                },
                [=](const void* InPtr)
                {
                    if (InPtr != nullptr)
                    {
                        InProperty->DestroyValue((void*)InPtr);
                    }
                }
            );

            Signature.Add(FGlobalEventParamType(InProperty));
        }
    }
}
