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
#include "EventParamType.generated.h"

// Global Event Parameter Type Fast Access Type
// used to provide a fast type check
UENUM(BlueprintType)
enum class EGlobalEventParameterType : uint8
{
	GEPT_Undefined,
	GEPT_Boolean,
	GEPT_Byte,
	GEPT_Int32,
	GEPT_Int64,
	GEPT_Float,
	GEPT_Double,
	GEPT_String,
	GEPT_Name,
	GEPT_Text,
	GEPT_Struct,
	GEPT_Object,
	GEPT_Enum,
	GEPT_Array,
	GEPT_Set,
	GEPT_Map
};

enum class EGlobalEventParameterFlags
{
	None        = 0,
	Reference   = 1 << 0,
	Pointer     = 1 << 1,
	Constant    = 1 << 2
};

/*
* Describes the type information of a UFunction or a parameter of a C++ function
*/
USTRUCT(BlueprintType)
struct GLOBALEVENTS_API FGlobalEventParamType
{
	GENERATED_BODY()
public:
	FGlobalEventParamType();
	FGlobalEventParamType(
		const FName& InCppName, 
		EGlobalEventParameterType InTypeId, 
		int InFlags,
		const UField* InObjectType
		);

	FGlobalEventParamType(const FProperty* InProperty);
	FGlobalEventParamType(const UScriptStruct* InStruct);
	FGlobalEventParamType(const FGlobalEventParamType& InOther);
	FGlobalEventParamType(FGlobalEventParamType&& InOther) noexcept;

	FGlobalEventParamType& operator = (const FGlobalEventParamType& InOther);
	FGlobalEventParamType& operator = (FGlobalEventParamType&& InOther) noexcept;

	bool operator == (const FGlobalEventParamType& InOther) const;
	bool operator != (const FGlobalEventParamType& InOther) const;

	inline const FName& GetName() const { return CppName; }

	inline bool IsReference() const { return (Flags & (int)EGlobalEventParameterFlags::Reference) != 0; }
	inline bool IsPointer() const { return (Flags & (int)EGlobalEventParameterFlags::Pointer) != 0; }
	inline bool IsConst() const { return (Flags & (int)EGlobalEventParameterFlags::Constant) != 0; }

	inline bool IsArray() const { return TypeId == EGlobalEventParameterType::GEPT_Array; }
	inline bool IsSet() const { return TypeId == EGlobalEventParameterType::GEPT_Set; }
	inline bool IsMap() const { return TypeId == EGlobalEventParameterType::GEPT_Map; }

	inline EGlobalEventParameterType GetTypeId() const { return TypeId; }

	bool CheckInvokeConvertibleFrom(const FGlobalEventParamType& InSourceParameter) const;

	static EGlobalEventParameterType GetPropertyTypeId(const FProperty* InProperty);
	static FString GetPropertyFullName(const FProperty* InProperty);

private:
	FName								CppName;
	EGlobalEventParameterType			TypeId;
	TWeakObjectPtr<const UField>		ObjectType;	
	int                                 Flags;
};


