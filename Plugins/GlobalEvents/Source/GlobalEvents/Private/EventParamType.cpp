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
#include "EventParamType.h"

FGlobalEventParamType::FGlobalEventParamType() :
    TypeId(EGlobalEventParameterType::GEPT_Undefined),
    Flags(0)
{
}

FGlobalEventParamType::FGlobalEventParamType(
    const FName& InCppName,
    EGlobalEventParameterType InTypeId,
    int InFlags,
    const UField* InObjectType
    ) :
    CppName(InCppName),
    TypeId(InTypeId),    
    ObjectType(InObjectType),
    Flags(InFlags)
{
}

FGlobalEventParamType::FGlobalEventParamType(const FProperty* InProperty) :
    CppName(GetPropertyFullName(InProperty)),
    TypeId(GetPropertyTypeId(InProperty)),
    Flags(0)
{
    if (InProperty->HasAnyPropertyFlags(CPF_ReferenceParm))
    {
        Flags |= (int)EGlobalEventParameterFlags::Reference;
    }

    if (InProperty->IsA<FObjectProperty>() || InProperty->IsA<FInterfaceProperty>())
    {
        Flags |= (int)EGlobalEventParameterFlags::Pointer;
    }

    if (InProperty->HasAnyPropertyFlags(CPF_ConstParm))
    {
        Flags |= (int)EGlobalEventParameterFlags::Constant;
    }

    if (InProperty->IsA<FArrayProperty>())
    {
        Flags |= (int)EGlobalEventParameterFlags::Array;
    }
    
    // 
    if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InProperty))
    {       
        ObjectType = ObjectProperty->PropertyClass;
    }
    else if (const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
    {
        checkSlow(StructProperty);

        ObjectType = StructProperty->Struct;
    }
    else if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(InProperty))
    {        
        checkSlow(EnumProperty);

        ObjectType = EnumProperty->GetEnum();
    }
}

FGlobalEventParamType::FGlobalEventParamType(const UScriptStruct* InStruct) :
    CppName(InStruct->GetStructCPPName()),
    TypeId(EGlobalEventParameterType::GEPT_Struct),    
    ObjectType(InStruct),
    Flags(0)
{
}

FGlobalEventParamType::FGlobalEventParamType(const FGlobalEventParamType& InOther) :
    CppName(InOther.CppName),
    TypeId(InOther.TypeId),
    ObjectType(InOther.ObjectType),
    Flags(InOther.Flags)
{
}

FGlobalEventParamType::FGlobalEventParamType(FGlobalEventParamType&& InOther) noexcept:
    CppName(MoveTemp(InOther.CppName)),
    TypeId(InOther.TypeId),
    ObjectType(MoveTemp(InOther.ObjectType)),
    Flags(InOther.Flags)
{
}

FGlobalEventParamType& FGlobalEventParamType::operator= (const FGlobalEventParamType& InOther)
{
    CppName = InOther.CppName;
    TypeId = InOther.TypeId;
    ObjectType = InOther.ObjectType;
    Flags = InOther.Flags;

    return *this;
}

FGlobalEventParamType& FGlobalEventParamType::operator= (FGlobalEventParamType&& InOther) noexcept
{
    CppName = MoveTemp(InOther.CppName);
    TypeId = InOther.TypeId;
    ObjectType = MoveTemp(InOther.ObjectType);
    Flags = InOther.Flags;

    return *this;
}

FString FGlobalEventParamType::GetPropertyFullName(const FProperty* InProperty)
{
    checkSlow(InProperty != nullptr);

    if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty))
    {
        FProperty* InnerProperty = ArrayProperty->Inner; 

        return FString::Printf(TEXT("TArray<%s>"), *GetPropertyFullName(InnerProperty));
    }
    else if (const FSetProperty* SetProperty = CastField<FSetProperty>(InProperty))
    {
        FProperty* InnerProperty = SetProperty->ElementProp;

        return FString::Printf(TEXT("TSet<%s>"), *GetPropertyFullName(InnerProperty));
    }
    else if (const FMapProperty* MapProperty = CastField<FMapProperty>(InProperty))
    {
        FProperty* KeyProperty = MapProperty->KeyProp;
        FProperty* ValueProperty = MapProperty->ValueProp;

        return FString::Printf(TEXT("TMap<%s, %s>"), *GetPropertyFullName(KeyProperty), *GetPropertyFullName(ValueProperty));
    }

    return InProperty->GetCPPType();
}

EGlobalEventParameterType FGlobalEventParamType::GetPropertyTypeId(const FProperty* InProperty)
{
    if (InProperty->IsA<FBoolProperty>())
    {
        return EGlobalEventParameterType::GEPT_Boolean;
    }
    else if (InProperty->IsA<FIntProperty>())
    {
        return EGlobalEventParameterType::GEPT_Int32;
    }
    else if (InProperty->IsA<FInt64Property>())
    {
        return EGlobalEventParameterType::GEPT_Int64;
    }
    else if (InProperty->IsA<FFloatProperty>())
    {
        return EGlobalEventParameterType::GEPT_Float;
    }
    else if (InProperty->IsA<FDoubleProperty>())
    {
        return EGlobalEventParameterType::GEPT_Double;
    }
    else if (InProperty->IsA<FNameProperty>())
    {
        return EGlobalEventParameterType::GEPT_Name;
    }
    else if (InProperty->IsA<FStrProperty>())
    {
        return EGlobalEventParameterType::GEPT_String;
    }
    else if (InProperty->IsA<FStructProperty>())
    {
        return EGlobalEventParameterType::GEPT_Struct;
    }
    else if (InProperty->IsA<FEnumProperty>())
    {
        return EGlobalEventParameterType::GEPT_Enum;
    }
    else if (InProperty->IsA<FObjectProperty>())
    {
        return EGlobalEventParameterType::GEPT_Object;
    }
    else if (InProperty->IsA<FTextProperty>())
    {
        return EGlobalEventParameterType::GEPT_Text;
    }
    else if (InProperty->IsA<FByteProperty>())
    {
        return EGlobalEventParameterType::GEPT_Byte;
    }
    else if (InProperty->IsA<FArrayProperty>())
    {
        return EGlobalEventParameterType::GEPT_Array;
    }
    else if (InProperty->IsA<FSetProperty>())
    {
        return EGlobalEventParameterType::GEPT_Set;
    }
    else if (InProperty->IsA<FMapProperty>())
    {
        return EGlobalEventParameterType::GEPT_Map;
    }

    return EGlobalEventParameterType::GEPT_Undefined;
}


bool FGlobalEventParamType::operator==(const FGlobalEventParamType& InOtherParameter) const
{
    /*
      The incomplete matching of everything here is intentional and is based on the internal implementation method:
      1. For C++ statically bound events, the code will automatically handle passing by reference, passing by value, etc.
      2. For scripted calls such as UFunction, the parameters are essentially copied to a temporary Tuple before being called. 
         If passed by reference, the value in the temporary memory area is copied back. 
         Therefore pass-by-value and pass-by-reference are equivalent in these cases.
    */
    return TypeId == InOtherParameter.TypeId &&
        CppName == InOtherParameter.CppName &&
        IsPointer() == InOtherParameter.IsPointer();
}

bool FGlobalEventParamType::operator != (const FGlobalEventParamType& InOther) const
{
    return !(*this == InOther);
}

bool FGlobalEventParamType::CheckInvokeConvertibleFrom(const FGlobalEventParamType& InSourceParameter) const
{
    // if equals
    if (*this == InSourceParameter)
    {
        return true;
    }

    // check UObject convertible
    if (TypeId == EGlobalEventParameterType::GEPT_Object && 
        InSourceParameter.TypeId == EGlobalEventParameterType::GEPT_Object &&
        ObjectType.IsValid() &&
        InSourceParameter.ObjectType.IsValid()
        )
    {
        const UStruct* TargetClass = Cast<UStruct>(ObjectType.Get());
        const UStruct* SourceClass = Cast<UStruct>(InSourceParameter.ObjectType.Get());

        return SourceClass->IsChildOf(TargetClass);
    }

    return false;
}

