#include "UnrealObjects.h"
#include "Offsets.h"

#include <format>


std::unordered_map<std::string, uint32_t> UEProperty::UnknownProperties;

void* UEObject::GetAddress()
{
	return Object;
}

EObjectFlags UEObject::GetFlags()
{
	return *reinterpret_cast<EObjectFlags*>(Object + Off::UObject::Flags);
}
int32 UEObject::GetIndex()
{
	return *reinterpret_cast<int32*>(Object + Off::UObject::Index);
}
UEClass UEObject::GetClass()
{
	return UEClass(*reinterpret_cast<void**>(Object + Off::UObject::Class));
}
FName UEObject::GetFName()
{
	return *reinterpret_cast<FName*>(Object + Off::UObject::Name);
}
UEObject UEObject::GetOuter()
{
	return UEObject(*reinterpret_cast<void**>(Object + Off::UObject::Outer));
}

bool UEObject::HasAnyFlags(EObjectFlags Flags)
{
	return GetFlags() & Flags;
}

template<typename UEType>
UEType UEObject::Cast()
{
	return UEType(this->GetAddress());
}

bool UEObject::IsA(EClassCastFlags TypeFlags)
{
	return TypeFlags != EClassCastFlags::None ? GetClass().IsType(TypeFlags) : true;
}

UEObject UEObject::GetOutermost()
{
	UEObject Outermost;

	for (UEObject Outer = GetOuter(); Outer; Outer = Outer.GetOuter())
	{
		Outermost = Outer;
	}
	return Outermost;
}

std::string UEObject::GetName()
{
	return GetFName().ToString();
}

std::string UEObject::GetValidName()
{
	std::string Name = GetName();

	for (char& c : Name)
	{
		if (c != '_' && !((c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A') || (c <= '9' && c >= '0')))
		{
			c = '_';
		}
	}

	return Name;
}

std::string UEObject::GetCppName()
{
	std::string Temp = GetValidName();

	if (this->IsA(EClassCastFlags::AActor))
	{
		return 'A' + Temp;
	}
	else if (this->IsA(EClassCastFlags::UStruct))
	{
		return 'F' + Temp;
	}

	return 'U' + Temp;
}

std::string UEObject::GetFullName()
{
	if (GetClass())
	{
		std::string Temp;

		for (UEObject Outer = GetOuter(); Outer; Outer = Outer.GetOuter())
		{
			Temp = Outer.GetName() + "." + Temp;
		}

		std::string Name = this->GetClass().GetName();
		Name += " ";
		Name += Temp;
		Name += GetName();

		return Name;
	}

	return "None";
}

UEObject::operator bool()
{
	return Object != nullptr;
}


UEField UEField::GetNext()
{
	return UEField(*reinterpret_cast<void**>(Object + Off::UField::Next));
}

bool UEField::IsNextValid()
{
	return (bool)GetNext();
}


TArray<TPair<FName, int64>>& UEEnum::GetNameValuePairs()
{
	return *reinterpret_cast<TArray<TPair<FName, int64>>*>(Object + Off::UEnum::Names);
}

std::string UEEnum::GetSingleName(int32 Index)
{
	return GetNameValuePairs()[Index].First.ToString();
}

std::vector<std::string> UEEnum::GetAllNames()
{
	auto& Names = GetNameValuePairs();

	std::vector<std::string> RetVec(Names.Num());

	for (int i = 0; i < Names.Num(); i++)
	{
		RetVec.push_back(Names[i].First.ToString());
	}

	return RetVec;
}

std::string UEEnum::GetEnumTypeAsStr()
{
	std::string Temp = GetName();

	return "enum class " + (Temp[0] == 'E' ? Temp : 'E' + Temp);
}


UEStruct UEStruct::GetSuper()
{
	return UEStruct(*reinterpret_cast<void**>(Object + Off::UStruct::SuperStruct));
}

UEField UEStruct::GetChild()
{
	return UEField(*reinterpret_cast<void**>(Object + Off::UStruct::Children));
}

int32 UEStruct::GetStructSize()
{
	return *reinterpret_cast<int32*>(Object + Off::UStruct::Size);
}


EClassCastFlags UEClass::GetFlags()
{
	return *reinterpret_cast<EClassCastFlags*>(Object + Off::UClass::ClassFlags);
}

bool UEClass::IsType(EClassCastFlags TypeFlag)
{
	return GetFlags() & TypeFlag;
}

UEObject UEClass::GetDefaultObject()
{
	return UEObject(*reinterpret_cast<void**>(Object + Off::UClass::ClassDefaultObject));
}


EFunctionFlags UEFunction::GetFunctionFlags()
{
	return *reinterpret_cast<EFunctionFlags*>(Object + Off::UFunction::FunctionFlags);
}

bool UEFunction::HasFlags(EFunctionFlags FuncFlags)
{
	return GetFunctionFlags() & FuncFlags;
}


int32 UEProperty::GetSize()
{
	return *reinterpret_cast<int32*>(Object + Off::UProperty::ElementSize);
}

int32 UEProperty::GetOffset()
{
	return *reinterpret_cast<int32*>(Object + Off::UProperty::Offset_Internal);
}

EPropertyFlags UEProperty::GetFlags()
{
	return *reinterpret_cast<EPropertyFlags*>(Object + Off::UProperty::PropertyFlags);
}

bool UEProperty::HasPropertyFlags(EPropertyFlags PropertyFlag)
{
	return GetFlags() & PropertyFlag;
}

std::string UEProperty::GetCppType()
{
	EClassCastFlags TypeFlags = GetClass().GetFlags();

	switch (TypeFlags)
	{
	case EClassCastFlags::UInt8Property:
		return "uint8";
		break;
	case EClassCastFlags::UUInt16Property:
		return "uint16";
		break;
	case EClassCastFlags::UUInt32Property:
		return "uint32";
		break;
	case EClassCastFlags::UUInt64Property:
		return "uint64";
		break;
	case EClassCastFlags::UByteProperty:
		return Cast<UEByteProperty>().GetCppType();
		break;
	case EClassCastFlags::UInt16Property:
		return "int16";
		break;
	case EClassCastFlags::UIntProperty:
		return "int32";
		break;
	case EClassCastFlags::UInt64Property:
		return "int64";
		break;
	case EClassCastFlags::UFloatProperty:
		return "float";
		break;
	case EClassCastFlags::UDoubleProperty:
		return "double";
		break;
	case EClassCastFlags::UClassProperty:
		return Cast<UEClassProperty>().GetCppType();
		break;
	case EClassCastFlags::UNameProperty:
		return "FName";
		break;
	case EClassCastFlags::UStrProperty:
		return "FString";
		break;
	case EClassCastFlags::UBoolProperty:
		return Cast<UEBoolProperty>().GetCppType();
		break;
	case EClassCastFlags::UStructProperty:
		return Cast<UEStructProperty>().GetCppType();
		break;
	case EClassCastFlags::UArrayProperty:
		return Cast<UEArrayProperty>().GetCppType();
		break;
	//case EClassCastFlags::UWeakObjectProperty:
	//	break;
	//case EClassCastFlags::ULazyObjectProperty:
	//	break;
	//case EClassCastFlags::USoftObjectProperty:
	//	break;
	case EClassCastFlags::UTextProperty:
		return "FText";
		break;
	//case EClassCastFlags::USoftClassProperty:
	//	break;
	case EClassCastFlags::UObjectProperty:
		return Cast<UEObjectProperty>().GetCppType();
		break;
	case EClassCastFlags::UMapProperty:
		return Cast<UEMapProperty>().GetCppType();
		break;
	case EClassCastFlags::USetProperty:
		return Cast<UESetProperty>().GetCppType();
		break;
	case EClassCastFlags::UEnumProperty:
		return Cast<UEEnumProperty>().GetCppType();
		break;
	default:
		std::string CppName = GetClass().GetCppName();

		UnknownProperties.insert({ CppName, GetSize() });

		return CppName;

		break;
	}
}


UEEnum UEByteProperty::GetEnum()
{
	return UEEnum(*reinterpret_cast<void**>(Object + Off::UByteProperty::Enum));
}

std::string UEByteProperty::GetCppType()
{
	if (UEEnum Enum = GetEnum())
	{
		return Enum.GetEnumTypeAsStr();
	}

	return "uint8";
}


uint8 UEBoolProperty::GetBitIndex()
{
	uint8 FieldMask = reinterpret_cast<Off::UBoolProperty::UBoolPropertyBase*>(Object + Off::UBoolProperty::Base)->FieldMask;

	if (FieldMask != 0xFF)
	{
		if (FieldMask == 0x01) { return 1; }
		if (FieldMask == 0x02) { return 2; }
		if (FieldMask == 0x04) { return 3; }
		if (FieldMask == 0x08) { return 4; }
		if (FieldMask == 0x10) { return 5; }
		if (FieldMask == 0x20) { return 6; }
		if (FieldMask == 0x40) { return 7; }
		if (FieldMask == 0x80) { return 8; }
	}

	return 0xFF;
}

bool UEBoolProperty::IsNativeBool()
{
	return reinterpret_cast<Off::UBoolProperty::UBoolPropertyBase*>(Object + Off::UBoolProperty::Base)->FieldMask == 0xFF;
}

std::string UEBoolProperty::GetCppType()
{
	return IsNativeBool() ? "bool" : "uint8";
}


UEClass UEObjectProperty::GetPropertyClass()
{
	return UEClass(*reinterpret_cast<void**>(Object + Off::UObjectProperty::PropertyClass));
}

std::string UEObjectProperty::GetCppType()
{
	return std::format("{}*", GetPropertyClass().GetCppName());
}


UEClass UEClassProperty::GetMetaClass()
{
	return UEClass(*reinterpret_cast<void**>(Object + Off::UClassProperty::MetaClass));
}

std::string UEClassProperty::GetCppType()
{
	return HasPropertyFlags(EPropertyFlags::UObjectWrapper) ? std::format("TSubclassOf<{}>", GetMetaClass().GetCppName()) : "UClass*";
}


UEStruct UEStructProperty::GetUnderlayingStruct()
{
	return UEStruct(*reinterpret_cast<void**>(Object + Off::UStructProperty::Struct));
}

std::string UEStructProperty::GetCppType()
{
	return GetUnderlayingStruct().GetCppName();
}


UEProperty UEArrayProperty::GetInnerProperty()
{
	return UEProperty(*reinterpret_cast<void**>(Object + Off::UArrayProperty::Inner));
}

std::string UEArrayProperty::GetCppType()
{
	return std::format("TArray<{}>", GetInnerProperty().GetCppType());
}



UEProperty UEMapProperty::GetKeyProperty()
{
	return UEProperty(reinterpret_cast<Off::UMapProperty::UMapPropertyBase*>(Object + Off::UMapProperty::Base)->KeyProperty);
}
UEProperty UEMapProperty::GetValueProperty()
{
	return UEProperty(reinterpret_cast<Off::UMapProperty::UMapPropertyBase*>(Object + Off::UMapProperty::Base)->ValueProperty);
}

std::string UEMapProperty::GetCppType()
{
	return std::format("TMap<{}, {}>", GetKeyProperty().GetCppType(), GetValueProperty().GetCppType());
}


UEProperty UESetProperty::GetElementProperty()
{
	return UEProperty(*reinterpret_cast<void**>(Object + Off::USetProperty::ElementProp));
}

std::string UESetProperty::GetCppType()
{
	return std::format("TSet<{}>", GetElementProperty().GetCppType());
}


UEProperty UEEnumProperty::GetUnderlayingProperty()
{
	return UEProperty(reinterpret_cast<Off::UEnumProperty::UEnumPropertyBase*>(Object + Off::UEnumProperty::Base)->UnderlayingProperty);
}

UEEnum UEEnumProperty::GetEnum()
{
	return UEEnum(reinterpret_cast<Off::UEnumProperty::UEnumPropertyBase*>(Object + Off::UEnumProperty::Base)->Enum);
}

std::string UEEnumProperty::GetCppType()
{
	return GetEnum().GetEnumTypeAsStr();
}


/*
* The compiler won't generate functions for a specific template type unless it's used in the .cpp file corresponding to the
* header it was declatred in.
*
* See https://stackoverflow.com/questions/456713/why-do-i-get-unresolved-external-symbol-errors-when-using-templates
*/
void TemplateTypeCreationForUnrealObjects(void)
{
	UEObject Dummy(nullptr);

	Dummy.Cast<UEObject>();
	Dummy.Cast<UEField>();
	Dummy.Cast<UEEnum>();
	Dummy.Cast<UEStruct>();
	Dummy.Cast<UEClass>();
	Dummy.Cast<UEFunction>();
	Dummy.Cast<UEProperty>();
	Dummy.Cast<UEByteProperty>();
	Dummy.Cast<UEBoolProperty>();
	Dummy.Cast<UEObjectProperty>();
	Dummy.Cast<UEClassProperty>();
	Dummy.Cast<UEStructProperty>();
	Dummy.Cast<UEArrayProperty>();
	Dummy.Cast<UEMapProperty>();
	Dummy.Cast<UESetProperty>();
	Dummy.Cast<UEEnumProperty>();
}



