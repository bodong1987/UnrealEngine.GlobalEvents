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
/*
* Provide basic static type information
*/
#pragma once

#include "EventParamType.h"

namespace UE
{
    namespace GlobalEvents
    {
        namespace Details
        {
            template <typename... Types>
            struct TTypeList
            {
            };

            template <int Index, typename TypeList>
            struct TTypeAt;

            template <typename Head, typename... Tail>
            struct TTypeAt<0, TTypeList<Head, Tail...>>
            {
                using Type = Head;
            };

            template <int Index, typename Head, typename... Tail>
            struct TTypeAt<Index, TTypeList<Head, Tail...>>
            {
                using Type = typename TTypeAt<Index - 1, TTypeList<Tail...>>::Type;
            };

            constexpr bool IsUObjectPtr(const volatile UObjectBase*) { return true; }
            constexpr bool IsUObjectPtr(...) { return false; }

#define UE_GLOBALEVENTS_DEFINE_HAS_SIGNATURE(TraitsName, FuncName, Signature)         \
            template <typename U>                                                     \
            class TraitsName                                                          \
            {                                                                         \
            private:                                                                  \
                template<typename T, T> struct Helper;                                \
                template<typename T>                                                  \
                static uint8 Z_Check(Helper<Signature, &FuncName>*);                  \
                template<typename T> static uint16 Z_Check(...);                      \
            public:                                                                   \
                constexpr static bool Value = sizeof(Z_Check<U>(0)) == sizeof(uint8); \
            }

            // check if this struct has StaticStruct member function...
            UE_GLOBALEVENTS_DEFINE_HAS_SIGNATURE(IsUStruct, T::StaticStruct, UClass* ());

#define UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(type) \
            template <> \
            class IsUStruct<type> \
            { \
            public: \
                constexpr static bool Value = true; \
            }

            // Manually add engine non-exported structures
            // check NoExportTypes.h
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FVector2D);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FVector);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FVector4);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FDateTime);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FBox);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FBox2D);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FRotator);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FGuid);            
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FColor);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FLinearColor);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FQuat);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FTransform);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FRay);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FPlane);
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FSphere);

            #if ENGINE_MAJOR_VERSION >= 5
            UE_GLOBALEVENTS_DEFINE_NO_EXPORT_USTRUCT(FMatrix);
            #endif

            // common base class
            template <typename T, typename TEnabledType = void>
            struct TTypeInfo
            {
                constexpr static bool IsSupportedType() { return false; }
                constexpr static const TCHAR* GetTypeName() { return TEXT("UnsupportedType"); }
                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Undefined; }
                static const UField* GetFieldType() { return nullptr; }
            };

            // for all UStruct
            template <typename T>
            struct TTypeInfo<T, typename TEnableIf<IsUStruct<T>::Value>::Type>
            {
                typedef FStructProperty    PropertyType;

                constexpr static bool IsSupportedType() { return true; }

                static const TCHAR* GetTypeName()
                {
                    static const FString Name = TBaseStructure<T>::Get()->GetStructCPPName();
                    return *Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Struct; }

                static const UField* GetFieldType() { return TBaseStructure<T>::Get(); }
            };

            template <typename T>
            struct TTypeInfo<T*, typename TEnableIf<IsUStruct<T>::Value>::Type>
            {
                constexpr static bool IsSupportedType() { return false; }

                static const TCHAR* GetTypeName()
                {
                    static const FString Name = TBaseStructure<T>::Get()->GetStructCPPName() + "*";
                    return *Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Struct; }

                static const UField* GetFieldType() { return TBaseStructure<T>::Get(); }
            };

            template <typename T>
            struct TTypeInfo<const T*, typename TEnableIf<IsUStruct<T>::Value>::Type>
            {
                constexpr static bool IsSupportedType() { return false; }

                static const TCHAR* GetTypeName()
                {
                    static const FString Name = FString(TEXT("const ")) + TBaseStructure<T>::Get()->GetStructCPPName() + "*";
                    return *Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Struct; }

                static const UField* GetFieldType() { return TBaseStructure<T>::Get(); }
            };

            template <typename T>
            struct TTypeInfo<T&, typename TEnableIf<IsUStruct<T>::Value>::Type>
            {
                constexpr static bool IsSupportedType() { return true; }

                static const TCHAR* GetTypeName()
                {
                    static const FString Name = TBaseStructure<T>::Get()->GetStructCPPName() + "&";
                    return *Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Struct; }

                static const UField* GetFieldType() { return TBaseStructure<T>::Get(); }
            };

            template <typename T>
            struct TTypeInfo<const T&, typename TEnableIf<IsUStruct<T>::Value>::Type>
            {
                constexpr static bool IsSupportedType() { return true; }

                static const TCHAR* GetTypeName()
                {
                    static const FString Name = FString(TEXT("const ")) + TBaseStructure<T>::Get()->GetStructCPPName() + "&";
                    return *Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Struct; }

                static const UField* GetFieldType() { return TBaseStructure<T>::Get(); }
            };

            // disable all UObject values
            template <typename T>
            struct TTypeInfo<T, typename TEnableIf<IsUObjectPtr((T*)nullptr)>::Type>
            {
                constexpr static bool IsSupportedType() { return false; }
                static const TCHAR* GetTypeName()
                {
                    static const FString Name = FString(StaticClass<T>()->GetPrefixCPP()) + StaticClass<T>()->GetName();
                    return *Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Object; }

                static const UField* GetFieldType() { return StaticClass<T>(); }
            };

            // for all UObject pointer
            template <typename T>
            struct TTypeInfo<T*, typename TEnableIf<IsUObjectPtr((T*)nullptr)>::Type>
            {
                constexpr static bool IsSupportedType() { return true; }
                static const TCHAR* GetTypeName()
                {
                    static const FString Name = FString(StaticClass<T>()->GetPrefixCPP()) + StaticClass<T>()->GetName() + TEXT("*");
                    return *Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Object; }
                static const UField* GetFieldType() { return StaticClass<T>(); }
            };

            // for all enum types
            // if you use an enum which is not an unreal enum type
            // you will get an link error for unresolved symbo : StaticEnum<T>()...
            template <typename T>
            struct TTypeInfo<T, typename TEnableIf<TIsEnum<T>::Value>::Type>
            {
                constexpr static bool IsSupportedType() { return true; }

                static const TCHAR* GetTypeName()
                {
                    static const FString Name = StaticEnum<T>()->GetName();
                    return *Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Enum; }

                static const UField* GetFieldType() { return StaticEnum<T>(); }
            };

#define UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE_DETAIL(type, typeName, supportValue, supportPointer, supportConstPointer, supportReference, supportConstReference, typeId) \
    template <> \
    struct TTypeInfo<type> \
    { \
        constexpr static bool IsSupportedType() { return supportValue; } \
        constexpr static const TCHAR* GetTypeName() { return TEXT(#typeName); } \
        constexpr static EGlobalEventParameterType GetTypeId() { return typeId; } \
        static const UField* GetFieldType() { return nullptr; } \
    }; \
    template <> \
    struct TTypeInfo<type*> \
    { \
        constexpr static bool IsSupportedType() { return supportPointer; } \
        constexpr static const TCHAR* GetTypeName() { return TEXT(#typeName) TEXT("*"); } \
        constexpr static EGlobalEventParameterType GetTypeId() { return typeId; } \
        static const UField* GetFieldType() { return nullptr; } \
    }; \
    template <> \
    struct TTypeInfo<const type*> \
    { \
        constexpr static bool IsSupportedType() { return supportConstPointer; } \
        constexpr static const TCHAR* GetTypeName() { return TEXT("const ") TEXT(#typeName) TEXT("*"); } \
        constexpr static EGlobalEventParameterType GetTypeId() { return typeId; } \
        static const UField* GetFieldType() { return nullptr; } \
    }; \
    template <> \
    struct TTypeInfo<type&> \
    { \
        constexpr static bool IsSupportedType() { return supportReference; } \
        constexpr static const TCHAR* GetTypeName() { return TEXT(#typeName) TEXT("&"); } \
        constexpr static EGlobalEventParameterType GetTypeId() { return typeId; } \
        static const UField* GetFieldType() { return nullptr; } \
    }; \
    template <> \
    struct TTypeInfo<const type&> \
    { \
        constexpr static bool IsSupportedType() { return supportConstReference; } \
        constexpr static const TCHAR* GetTypeName() { return TEXT("const ") TEXT(#typeName) TEXT("&"); } \
        constexpr static EGlobalEventParameterType GetTypeId() { return typeId; } \
        static const UField* GetFieldType() { return nullptr; } \
    }

#define UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE_IMPL(type, typeName, typeId) UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE_DETAIL(type, typeName, true, false, false, true, false, typeId)
#define UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE(type, typeId) UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE_DETAIL(type, type, true, false, false, true, false, typeId)
#define UE_GLOBALEVENTS_DEFINE_ENGINE_BUILTIN_TYPE(type, typeId) UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE_DETAIL(type, type, true, false, false, true, true, typeId)

            UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE(bool, EGlobalEventParameterType::GEPT_Boolean);
            UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE(uint8, EGlobalEventParameterType::GEPT_Byte);
            UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE(int32, EGlobalEventParameterType::GEPT_Int32);
            UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE(int64, EGlobalEventParameterType::GEPT_Int64);
            UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE(float, EGlobalEventParameterType::GEPT_Float);
            UE_GLOBALEVENTS_DEFINE_BUILTIN_TYPE(double, EGlobalEventParameterType::GEPT_Double);

            UE_GLOBALEVENTS_DEFINE_ENGINE_BUILTIN_TYPE(FName, EGlobalEventParameterType::GEPT_Name);
            UE_GLOBALEVENTS_DEFINE_ENGINE_BUILTIN_TYPE(FString, EGlobalEventParameterType::GEPT_String);
            UE_GLOBALEVENTS_DEFINE_ENGINE_BUILTIN_TYPE(FText, EGlobalEventParameterType::GEPT_Text);


            template <typename T>
            struct TTypeInfo<TArray<T>>
            {
                static_assert(TTypeInfo<T>::IsSupportedType() && !TIsReferenceType<T>::Value, "Unsupported type in TArray");

                typedef T           ElementType;

                constexpr static bool IsSupportedType()
                {
                    return TTypeInfo<T>::IsSupportedType() && !TIsReferenceType<T>::Value;
                }

                static const TCHAR* GetTypeName()
                {
                    static const FString Z_Name = FString(TEXT("TArray<")) + TTypeInfo<T>::GetTypeName() + TEXT(">");
                    return *Z_Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Array; }

                constexpr static EGlobalEventParameterType GetElementTypeId() { return TTypeInfo<T>::GetTypeId(); }

                static const UField* GetFieldType() { return nullptr; }
            };

            template <typename T>
            struct TTypeInfo<const TArray<T>&>
            {
                static_assert(TTypeInfo<T>::IsSupportedType() && !TIsReferenceType<T>::Value, "Unsupported type in TArray");

                typedef T           ElementType;

                constexpr static bool IsSupportedType()
                {
                    return TTypeInfo<T>::IsSupportedType() && !TIsReferenceType<T>::Value;
                }

                static const TCHAR* GetTypeName()
                {
                    static const FString Z_Name = FString(TEXT("const TArray<")) + TTypeInfo<T>::GetTypeName() + TEXT(">&");
                    return *Z_Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Array; }

                constexpr static EGlobalEventParameterType GetElementTypeId() { return TTypeInfo<T>::GetTypeId(); }

                static const UField* GetFieldType() { return nullptr; }
            };

            template <typename T>
            struct TTypeInfo<TSet<T>>
            {
                static_assert(TTypeInfo<T>::IsSupportedType() && !TIsReferenceType<T>::Value, "Unsupported type in TSet");

                typedef T           ElementType;

                constexpr static bool IsSupportedType()
                {
                    return TTypeInfo<T>::IsSupportedType() && !TIsReferenceType<T>::Value;
                }

                static const TCHAR* GetTypeName()
                {
                    static const FString Z_Name = FString(TEXT("TSet<")) + TTypeInfo<T>::GetTypeName() + TEXT(">");
                    return *Z_Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Set; }

                constexpr static EGlobalEventParameterType GetElementTypeId() { return TTypeInfo<T>::GetTypeId(); }

                static const UField* GetFieldType() { return nullptr; }
            };

            template <typename T>
            struct TTypeInfo<const TSet<T>&>
            {
                static_assert(TTypeInfo<T>::IsSupportedType() && !TIsReferenceType<T>::Value, "Unsupported type in TSet");

                typedef T           ElementType;

                constexpr static bool IsSupportedType()
                {
                    return TTypeInfo<T>::IsSupportedType() && !TIsReferenceType<T>::Value;
                }

                static const TCHAR* GetTypeName()
                {
                    static const FString Z_Name = FString(TEXT("const TSet<")) + TTypeInfo<T>::GetTypeName() + TEXT(">&");
                    return *Z_Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Set; }

                constexpr static EGlobalEventParameterType GetElementTypeId() { return TTypeInfo<T>::GetTypeId(); }

                static const UField* GetFieldType() { return nullptr; }
            };

            template <typename TKey, typename TValue>
            struct TTypeInfo<TMap<TKey, TValue>>
            {
                static_assert(TTypeInfo<TKey>::IsSupportedType() && !TIsReferenceType<TKey>::Value, "Unsupported type in TMap");
                static_assert(TTypeInfo<TValue>::IsSupportedType() && !TIsReferenceType<TValue>::Value, "Unsupported type in TMap");

                typedef TKey           KeyType;
                typedef TValue         ValueType;

                constexpr static bool IsSupportedType()
                {
                    return TTypeInfo<TKey>::IsSupportedType() && !TIsReferenceType<TKey>::Value &&
                        TTypeInfo<TValue>::IsSupportedType() && !TIsReferenceType<TValue>::Value;
                }

                static const TCHAR* GetTypeName()
                {
                    static const FString Z_Name = FString::Printf(TEXT("TMap<%s, %s>"), TTypeInfo<KeyType>::GetTypeName(), TTypeInfo<ValueType>::GetTypeName());
                    return *Z_Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Map; }

                constexpr static EGlobalEventParameterType GetKeyTypeId() { return TTypeInfo<KeyType>::GetTypeId(); }
                constexpr static EGlobalEventParameterType GetValueTypeId() { return TTypeInfo<ValueType>::GetTypeId(); }

                static const UField* GetFieldType() { return nullptr; }
            };

            template <typename TKey, typename TValue>
            struct TTypeInfo<const TMap<TKey, TValue>&>
            {
                static_assert(TTypeInfo<TKey>::IsSupportedType() && !TIsReferenceType<TKey>::Value, "Unsupported type in TMap");
                static_assert(TTypeInfo<TValue>::IsSupportedType() && !TIsReferenceType<TValue>::Value, "Unsupported type in TMap");

                typedef TKey           KeyType;
                typedef TValue         ValueType;

                constexpr static bool IsSupportedType()
                {
                    return TTypeInfo<TKey>::IsSupportedType() && !TIsReferenceType<TKey>::Value &&
                        TTypeInfo<TValue>::IsSupportedType() && !TIsReferenceType<TValue>::Value;
                }

                static const TCHAR* GetTypeName()
                {
                    static const FString Z_Name = FString::Printf(TEXT("const TMap<%s, %s>&"), TTypeInfo<KeyType>::GetTypeName(), TTypeInfo<ValueType>::GetTypeName());
                    return *Z_Name;
                }

                constexpr static EGlobalEventParameterType GetTypeId() { return EGlobalEventParameterType::GEPT_Map; }

                constexpr static EGlobalEventParameterType GetKeyTypeId() { return TTypeInfo<KeyType>::GetTypeId(); }
                constexpr static EGlobalEventParameterType GetValueTypeId() { return TTypeInfo<ValueType>::GetTypeId(); }

                static const UField* GetFieldType() { return nullptr; }
            };


            template <typename... ParamTypes>
            struct TIsSupportedTypes
            {
                static constexpr bool Value = true;
            };

            template <typename T, typename... ParamTypes>
            struct TIsSupportedTypes<T, ParamTypes...>
            {
                // if you get an compile error here
                // it means you use an unsupported type in global event systems
                static_assert(TTypeInfo<T>::IsSupportedType(), "Don't use unsupported type!!!");

                static constexpr bool Value = TIsSupportedTypes<ParamTypes...>::Value;
            };

            // check is non const left reference
            template <typename T>
            struct TIsNonConstLValueReference
            {
                static constexpr bool Value = TIsLValueReferenceType<T>::Value && !TIsConst<typename TRemoveReference<T>::Type>::Value;

                // compatible with std::disjunction_v
                static constexpr bool value = Value;
            };

            template <typename... ParamTypes>
            struct THasNonConstLValueReference
            {
                static constexpr bool Value = std::disjunction_v<TIsNonConstLValueReference<ParamTypes>...>;
            };
        }
    }
}

