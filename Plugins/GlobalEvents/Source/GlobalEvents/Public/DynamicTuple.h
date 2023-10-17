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

#include "EventParamType.h"
#include "Details/TypeInfo.h"
#include "Details/Signature.h"

namespace UE
{
    namespace GlobalEvents
    {
        /*
        * Dynamic tuples are designed to dynamically generate data with the same memory layout as TTuple.
        * This data will be used to call the UFunction through the script engine
        */
        class GLOBALEVENTS_API FDynamicTuple
        {
        private:
            FDynamicTuple(const FDynamicTuple&) = delete;
            FDynamicTuple& operator = (const FDynamicTuple&) = delete;
        public:
            FDynamicTuple();
            ~FDynamicTuple();
            FDynamicTuple(FDynamicTuple&& InOther);
            FDynamicTuple& operator = (FDynamicTuple&& InOther);

            void Clear();

            inline int32 Num() const { return Signature.Num(); }

            inline const void* GetData() const { return (const void*)TupleBuffer.GetData(); }
            inline const ISignature* GetSignature() const { return &Signature; }
            inline int32 GetOffset(int32 InIndex) const { return TupleElementRecords[InIndex].Offset; }

        private:
            struct FParamRecord
            {
                uint32                              Offset;
                TFunction<void(const void*)>        Deletor;
            };

        public:
            template <typename T>
            inline void Push(const T& InValue)
            {
                static_assert(Details::TTypeInfo<T>::IsSupportedType(), "Unsupported type checked.");

                PushInternal(sizeof(T), alignof(T),
                    [&](const void* Address) {
                        *(T*)Address = InValue;
                    },
                    [&](const void* InPtr)
                    {
                        if constexpr (TIsTriviallyDestructible<T>::Value)
                        {
                            if (InPtr != nullptr)
                            {
                                ((T*)InPtr)->~T();
                            }
                        }
                    }
                );

                Signature.Add<T>();
            }

            template <typename T>
            inline const T& Get(int InIndex) const
            {
                static_assert(Details::TTypeInfo<T>::IsSupportedType(), "Unsupported type checked.");

                checkSlow(InIndex >= 0 && InIndex < TupleElementRecords.Num());
                checkSlow(Details::TTypeInfo<T>::GetTypeId() == Signature.GetParamType(InIndex).GetTypeId());
                checkSlow(Details::TTypeInfo<T>::GetTypeName() == Signature.GetParamType(InIndex).GetName());

                const FParamRecord& Record = TupleElementRecords[InIndex];

                const uint8* Address = TupleBuffer.GetData() + Record.Offset;

                return *(const T*)Address;
            }

            template <typename T>
            inline T& Get(int InIndex)
            {
                static_assert(Details::TTypeInfo<T>::IsSupportedType(), "Unsupported type checked.");

                checkSlow(InIndex >= 0 && InIndex < TupleElementRecords.Num());
                checkSlow(Details::TTypeInfo<T>::GetTypeId() == Signature.GetParamType(InIndex).GetTypeId());
                checkSlow(Details::TTypeInfo<T>::GetTypeName() == Signature.GetParamType(InIndex).GetName());

                const FParamRecord& Record = TupleElementRecords[InIndex];

                const uint8* Address = TupleBuffer.GetData() + Record.Offset;

                return *(const T*)Address;
            }

            inline const void* GetAddress(int InIndex) const
            {
                checkSlow(InIndex >= 0 && InIndex < TupleElementRecords.Num());

                const FParamRecord& Record = TupleElementRecords[InIndex];

                const uint8* Address = TupleBuffer.GetData() + Record.Offset;

                return Address;
            }

            /* Push A struct based on struct define */
            void Push(const UScriptStruct* InScriptStruct, const void* InAddress);

            /* Push a value based on Property */
            void Push(const FProperty* InProperty, const void* InSourceAddress);

        private:
            constexpr SIZE_T AlignAddress(SIZE_T InAddress, SIZE_T InAlignment)
            {
                return (InAddress + (InAlignment - 1)) & ~(InAlignment - 1);
            }

            void PushInternal(int InElementSize, int InAlignmentSize, TFunction<void(const void*)>&& InCopyFunctor, TFunction<void(const void*)>&& InDeleteFunctor);

        private:
            TArray<uint8>                       TupleBuffer;
            TArray<FParamRecord>                TupleElementRecords;
            Details::FDynamicSignature          Signature;
        };
    }
}

