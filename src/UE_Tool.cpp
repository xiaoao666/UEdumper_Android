#include "UE_Tool.h"



std::string GetNameByIndex(uint32_t id)
{
    //  v4 = (unsigned __int16 *)(*(_QWORD *)((char *)&GName + (((unsigned __int64)(unsigned int)*a1 >> 13) & 0x7FFF8) + 64) + 2LL * (unsigned __int16)*a1);
    uint64_t FNameEntry = offsets.GName + offsets.FNameEntry.FNameEntry_offset;
    uint64_t Block = id >> 13;
    uint64_t Offset = Block & 0x7FFF8;
    uint64_t NameChuck = mem.ReadDword64(FNameEntry + Offset);
    uint64_t NameEntry = NameChuck + (uint16_t)id * 2;

    int16_t EntryHeader = mem.Read<int16_t>(NameEntry);

    /*struct UE_FNameEntryHeader
    {
    uint16_t bIsWide : 1;
    #if WITH_CASE_PRESERVING_NAME
    uint16 Len : 15;
    #else
    static constexpr uint32_t ProbeHashBits = 5;
    uint16_t LowercaseProbeHash : ProbeHashBits;
    uint16_t Len : 10;
    #endif
    };*/
    int len = EntryHeader >> 6;
    //printf("EntryHeader: %d, len: %d\n", EntryHeader, len);
    std::string name = mem.ReadString(NameEntry + offsets.FNameEntry.NameEntry_size,len);

    auto pos = name.rfind('/');
    if (pos != std::string::npos)
    {
        name = name.substr(pos + 1);
    }

    return name;
}



uint32_t GetFNameEntryId(uint32_t Block, uint32_t Offset)
{
    return  (Block << FNameBlockOffsetBits | Offset) ;
}

// GetNextSize 算法
uint16_t GetNextSize(uint16_t Header)
{
    uint16_t bytes = offsets.FNameEntry.NameEntry_size + (Header >> 6) * ((Header & 0x1) ? 2 : 1);//Header & 0x1 = bIsWide  (Header >> 6) = Len
    return (bytes + Stride - 1u) & ~(Stride - 1u);
}


// uintptr_t GetObjectPtrByIndex(uint32_t index){
//     if (index >= mem.Read<int32_t>(offsets.GUObjectArray + offsets.ObjectArray.NumElements_offset))
//         return 0;
//     uint32_t ChunkIndex = index / NumElementsPerChunk;
//     uint32_t WithinChunkIndex = index % NumElementsPerChunk;
//     uintptr_t ObjectItem = mem.ReadDword64(mem.ReadDword64(offsets.GUObjectArray + offsets.ObjectArray.Objects_offset + ChunkIndex * 0x8)) + WithinChunkIndex * 0x8;//0x8是ObjectItem的大小
//     uintptr_t obj = mem.ReadDword64(ObjectItem);
//     return obj;
// }


uintptr_t GetObjectPtrByIndex(uint32_t index){
    if (index >= mem.Read<int32_t>(offsets.GUObjectArray + offsets.ObjectArray.NumElements_offset))
        return 0;
    uint32_t ChunkIndex = index / NumElementsPerChunk;
    uint32_t WithinChunkIndex = index % NumElementsPerChunk;
    uintptr_t ObjectItem = mem.ReadDword64(mem.ReadDword64(offsets.GUObjectArray + offsets.ObjectArray.Objects_offset) + ChunkIndex * 0x8) + WithinChunkIndex * 0x18;//0x8是ObjectItem的大小
    uintptr_t obj = mem.ReadDword64(ObjectItem);
    return obj;
}

int32_t UE_UObject::GetIndex() const
{
    if (!object) return -1;

    return mem.Read<int32_t>(object + offsets.UObject.InternalIndex);
}

UE_UClass UE_UObject::GetClass() const
{
    if (!object) return UE_UClass();

    return mem.Read<UE_UClass>(object + offsets.UObject.ClassPrivate);
}



UE_UObject UE_UObject::GetOuter() const
{
    if (!object) return UE_UObject();

    return mem.Read<UE_UObject>(object + offsets.UObject.OuterPrivate);
}

UE_UObject UE_UObject::GetPackageObject() const
{
    if (!object) return UE_UObject();

    UE_UObject package;
    for (auto outer = GetOuter(); outer.object; outer = outer.GetOuter())
    {
        package = outer;
    }
    return package;
}

std::string UE_UObject::GetName() const
{
    if (!object) return "";

    int32_t nameIndex = mem.Read<int32_t>(object + offsets.UObject.NamePrivate);
    return GetNameByIndex(nameIndex);
}

std::string UE_UObject::GetFullName() const{
    if (!object) return "";

    std::string temp;
    for (auto outer = GetOuter(); outer.object; outer = outer.GetOuter())
    {
        temp = outer.GetName() + "." + temp;
    }
    UE_UClass objectClass = GetClass();
    std::string name = objectClass.GetName() + " " + temp + GetName();
    return name;
}



UE_FUObjectArray GetObjects() {
    UE_FUObjectArray GUObjectArray = mem.Read<UE_FUObjectArray>(offsets.GUObjectArray);
    return GUObjectArray;
}


UE_UObject UE_FUObjectArray::GetObjectPtr(int32_t index) const
{
    return UE_UObject(GetObjectPtrByIndex(index));
}



int32_t UE_FUObjectArray::GetNumElements() const
{
    return mem.Read<int32_t>(offsets.GUObjectArray + offsets.ObjectArray.NumElements_offset);
}


UE_UClass UE_UObject::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Object");
    return obj;
}

UE_UClass UE_UClass::StaticClass()
{
    static UE_UClass obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Class");
    return obj;
}

UE_UClass UE_AActor::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class Engine.Actor");
    return obj;
}

UE_UClass UE_UInterface::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Interface");
    return obj;
}

UE_UStruct UE_UStruct::GetSuper() const
{
    return mem.Read<UE_UStruct>(this->GetAddress() + offsets.UStruct.SuperStruct);
}

int32_t UE_UStruct::GetSize() const
{
    return mem.Read<uint32_t>(this->GetAddress() + offsets.UStruct.PropertiesSize);
}

UE_FField UE_UStruct::GetChildProperties() const
{
    return mem.Read<UE_FField>(this->GetAddress() + offsets.UStruct.ChildProperties);
}

UE_UField UE_UStruct::GetChildren() const
{
    return mem.Read<UE_UField>(this->GetAddress() + offsets.UStruct.Children);
}


UE_UClass UE_UStruct::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Struct");//正常是大写Struct
    if (obj == 0x0)
        obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.struct");
    return obj;
}


std::string IUProperty::GetName() const
{
    return ((UE_UProperty *)(this->prop))->GetName();
}

int32_t IUProperty::GetArrayDim() const
{
    return ((UE_UProperty *)(this->prop))->GetArrayDim();
}

int32_t IUProperty::GetSize() const
{
    return ((UE_UProperty *)(this->prop))->GetSize();
}

int32_t IUProperty::GetOffset() const
{
    return ((UE_UProperty *)(this->prop))->GetOffset();
}

uint64_t IUProperty::GetPropertyFlags() const
{
    return ((UE_UProperty *)(this->prop))->GetPropertyFlags();
}

std::pair<UEPropertyType, std::string> IUProperty::GetType() const
{
    return ((UE_UProperty *)(this->prop))->GetType();
}



int32_t UE_UProperty::GetArrayDim() const
{
    return mem.Read<int32_t>(this->GetAddress() + offsets.UProperty.ArrayDim);
}

int32_t UE_UProperty::GetSize() const
{
    return mem.Read<int32_t>(this->GetAddress() + offsets.UProperty.ElementSize);
}

int32_t UE_UProperty::GetOffset() const
{
    return mem.Read<int32_t>(this->GetAddress() + offsets.UProperty.Offset_Internal);
}

uint64_t UE_UProperty::GetPropertyFlags() const
{
    return mem.Read<uint64_t>(this->GetAddress() + offsets.UProperty.PropertyFlags);
}


std::pair<UEPropertyType, std::string> UE_UProperty::GetType() const
{
    if (IsA<UE_UDoubleProperty>())
    {
        return {UEPropertyType::DoubleProperty, Cast<UE_UDoubleProperty>().GetTypeStr()};
    }
    if (IsA<UE_UFloatProperty>())
    {
        return {UEPropertyType::FloatProperty, Cast<UE_UFloatProperty>().GetTypeStr()};
    }
    if (IsA<UE_UIntProperty>())
    {
        return {UEPropertyType::IntProperty, Cast<UE_UIntProperty>().GetTypeStr()};
    }
    if (IsA<UE_UInt16Property>())
    {
        return {UEPropertyType::Int16Property, Cast<UE_UInt16Property>().GetTypeStr()};
    }
    if (IsA<UE_UInt32Property>())
    {
        return {UEPropertyType::Int32Property, Cast<UE_UInt32Property>().GetTypeStr()};
    }
    if (IsA<UE_UInt64Property>())
    {
        return {UEPropertyType::Int64Property, Cast<UE_UInt64Property>().GetTypeStr()};
    }
    if (IsA<UE_UInt8Property>())
    {
        return {UEPropertyType::Int8Property, Cast<UE_UInt8Property>().GetTypeStr()};
    }
    if (IsA<UE_UUInt16Property>())
    {
        return {UEPropertyType::UInt16Property, Cast<UE_UUInt16Property>().GetTypeStr()};
    }
    if (IsA<UE_UUInt32Property>())
    {
        return {UEPropertyType::UInt32Property, Cast<UE_UUInt32Property>().GetTypeStr()};
    }
    if (IsA<UE_UUInt64Property>())
    {
        return {UEPropertyType::UInt64Property, Cast<UE_UUInt64Property>().GetTypeStr()};
    }
    if (IsA<UE_UTextProperty>())
    {
        return {UEPropertyType::TextProperty, Cast<UE_UTextProperty>().GetTypeStr()};
    }
    if (IsA<UE_UStrProperty>())
    {
        return {UEPropertyType::StrProperty, Cast<UE_UStrProperty>().GetTypeStr()};
    }
    if (IsA<UE_UClassProperty>())
    {
        return {UEPropertyType::ClassProperty, Cast<UE_UClassProperty>().GetTypeStr()};
    }
    if (IsA<UE_UStructProperty>())
    {
        return {UEPropertyType::StructProperty, Cast<UE_UStructProperty>().GetTypeStr()};
    }
    if (IsA<UE_UNameProperty>())
    {
        return {UEPropertyType::NameProperty, Cast<UE_UNameProperty>().GetTypeStr()};
    }
    if (IsA<UE_UBoolProperty>())
    {
        return {UEPropertyType::BoolProperty, Cast<UE_UBoolProperty>().GetTypeStr()};
    }
    if (IsA<UE_UByteProperty>())
    {
        return {UEPropertyType::ByteProperty, Cast<UE_UByteProperty>().GetTypeStr()};
    }
    if (IsA<UE_UArrayProperty>())
    {
        return {UEPropertyType::ArrayProperty, Cast<UE_UArrayProperty>().GetTypeStr()};
    }
    if (IsA<UE_UEnumProperty>())
    {
        return {UEPropertyType::EnumProperty, Cast<UE_UEnumProperty>().GetTypeStr()};
    }
    if (IsA<UE_USetProperty>())
    {
        return {UEPropertyType::SetProperty, Cast<UE_USetProperty>().GetTypeStr()};
    }
    if (IsA<UE_UMapProperty>())
    {
        return {UEPropertyType::MapProperty, Cast<UE_UMapProperty>().GetTypeStr()};
    }
    if (IsA<UE_UInterfaceProperty>())
    {
        return {UEPropertyType::InterfaceProperty, Cast<UE_UInterfaceProperty>().GetTypeStr()};
    }
    if (IsA<UE_UMulticastDelegateProperty>())
    {
        return {UEPropertyType::MulticastDelegateProperty, Cast<UE_UMulticastDelegateProperty>().GetTypeStr()};
    }
    if (IsA<UE_UWeakObjectProperty>())
    {
        return {UEPropertyType::WeakObjectProperty, Cast<UE_UWeakObjectProperty>().GetTypeStr()};
    }
    if (IsA<UE_ULazyObjectProperty>())
    {
        return {UEPropertyType::LazyObjectProperty, Cast<UE_ULazyObjectProperty>().GetTypeStr()};
    }
    if (IsA<UE_UObjectProperty>())
    {
        return {UEPropertyType::ObjectProperty, Cast<UE_UObjectProperty>().GetTypeStr()};
    }
    if (IsA<UE_UObjectPropertyBase>())
    {
        return {UEPropertyType::ObjectProperty, Cast<UE_UObjectPropertyBase>().GetTypeStr()};
    }
    return {UEPropertyType::Unknown, GetClass().GetName()};
}

IUProperty UE_UProperty::GetInterface() const { return IUProperty(this); }

UE_UClass UE_UProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Property");
    return obj;
}

uint8_t IUProperty::GetFieldMask() const
{
    return ((UE_UBoolProperty *)(this->prop))->GetFieldMask();
}

std::string IFProperty::GetName() const
{
    return ((UE_FProperty *)prop)->GetName();
}

int32_t IFProperty::GetArrayDim() const
{
    return ((UE_FProperty *)prop)->GetArrayDim();
}

int32_t IFProperty::GetSize() const { return ((UE_FProperty *)prop)->GetSize(); }

int32_t IFProperty::GetOffset() const
{
    return ((UE_FProperty *)prop)->GetOffset();
}

uint64_t IFProperty::GetPropertyFlags() const
{
    return ((UE_FProperty *)prop)->GetPropertyFlags();
}

std::pair<UEPropertyType, std::string> IFProperty::GetType() const
{
    return ((UE_FProperty *)prop)->GetType();
}

uint8_t IFProperty::GetFieldMask() const
{
    return ((UE_FBoolProperty *)prop)->GetFieldMask();
}

int32_t UE_FProperty::GetArrayDim() const
{
    return mem.Read<int32_t>(this->GetAddress() + offsets.FProperty.ArrayDim);
}

int32_t UE_FProperty::GetSize() const
{
    return mem.Read<int32_t>(this->GetAddress() + offsets.FProperty.ElementSize);
}

int32_t UE_FProperty::GetOffset() const
{
    return mem.Read<int32_t>(this->GetAddress() + offsets.FProperty.Offset_Internal);
}

uint64_t UE_FProperty::GetPropertyFlags() const
{
    return mem.Read<uint64_t>(this->GetAddress() + offsets.FProperty.PropertyFlags);
}


UEPropTypeInfo UE_FProperty::GetType() const
{
    auto objectClass = GetClass();
    UEPropTypeInfo type = {UEPropertyType::Unknown, objectClass.GetName()};

    auto str = type.second;
    if (str == "StructProperty")
    {
        type = {UEPropertyType::StructProperty, this->Cast<UE_FStructProperty>().GetTypeStr()};
    }
    else if (str == "ObjectProperty")
    {
        type = {UEPropertyType::ObjectProperty, this->Cast<UE_FObjectPropertyBase>().GetTypeStr()};
    }
    else if (str == "SoftObjectProperty")
    {
        type = {UEPropertyType::SoftObjectProperty, this->Cast<UE_FObjectPropertyBase>().GetTypeStr()};
    }
    else if (str == "FloatProperty")
    {
        type = {UEPropertyType::FloatProperty, "float"};
    }
    else if (str == "ByteProperty")
    {
        type = {UEPropertyType::ByteProperty, "uint8_t"};
    }
    else if (str == "BoolProperty")
    {
        type = {UEPropertyType::BoolProperty, "bool"};
    }
    else if (str == "IntProperty")
    {
        type = {UEPropertyType::IntProperty, "int32_t"};
    }
    else if (str == "Int8Property")
    {
        type = {UEPropertyType::Int8Property, "int8_t"};
    }
    else if (str == "Int16Property")
    {
        type = {UEPropertyType::Int16Property, "int16_t"};
    }
    else if (str == "Int32Property")
    {
        type = {UEPropertyType::Int32Property, "int32_t"};
    }
    else if (str == "Int64Property")
    {
        type = {UEPropertyType::Int64Property, "int64_t"};
    }
    else if (str == "UInt16Property")
    {
        type = {UEPropertyType::UInt16Property, "uint16_t"};
    }
    else if (str == "UInt32Property")
    {
        type = {UEPropertyType::UInt32Property, "uint32_t"};
    }
    else if (str == "UInt64Property")
    {
        type = {UEPropertyType::UInt64Property, "uint64_t"};
    }
    else if (str == "NameProperty")
    {
        type = {UEPropertyType::NameProperty, "struct FName"};
    }
    else if (str == "DelegateProperty")
    {
        type = {UEPropertyType::DelegateProperty, "struct FDelegate"};
    }
    else if (str == "SetProperty")
    {
        type = {UEPropertyType::SetProperty, this->Cast<UE_FSetProperty>().GetTypeStr()};
    }
    else if (str == "ArrayProperty")
    {
        type = {UEPropertyType::ArrayProperty, this->Cast<UE_FArrayProperty>().GetTypeStr()};
    }
    else if (str == "WeakObjectProperty")
    {
        type = {UEPropertyType::WeakObjectProperty, this->Cast<UE_FStructProperty>().GetTypeStr()};
    }
    else if (str == "LazyObjectProperty")
    {
        type = {UEPropertyType::LazyObjectProperty, this->Cast<UE_FStructProperty>().GetTypeStr()};
    }
    else if (str == "StrProperty")
    {
        type = {UEPropertyType::StrProperty, "struct FString"};
    }
    else if (str == "TextProperty")
    {
        type = {UEPropertyType::TextProperty, "struct FText"};
    }
    else if (str == "MulticastSparseDelegateProperty")
    {
        type = {UEPropertyType::MulticastSparseDelegateProperty, "struct FMulticastSparseDelegate"};
    }
    else if (str == "EnumProperty")
    {
        type = {UEPropertyType::EnumProperty, this->Cast<UE_FEnumProperty>().GetTypeStr()};
    }
    else if (str == "DoubleProperty")
    {
        type = {UEPropertyType::DoubleProperty, "double"};
    }
    else if (str == "MulticastDelegateProperty")
    {
        type = {UEPropertyType::MulticastDelegateProperty, "FMulticastDelegate"};
    }
    else if (str == "ClassProperty")
    {
        type = {UEPropertyType::ClassProperty, this->Cast<UE_FClassProperty>().GetTypeStr()};
    }
    else if (str == "MulticastInlineDelegateProperty")
    {
        type = {UEPropertyType::MulticastInlineDelegateProperty, "struct FMulticastInlineDelegate"};//MulticastDelegateProperty
    }
    else if (str == "MapProperty")
    {
        type = {UEPropertyType::MapProperty, this->Cast<UE_FMapProperty>().GetTypeStr()};
    }
    else if (str == "InterfaceProperty")
    {
        type = {UEPropertyType::InterfaceProperty, this->Cast<UE_FInterfaceProperty>().GetTypeStr()};
    }
    else if (str == "FieldPathProperty")
    {
        type = {UEPropertyType::FieldPathProperty, this->Cast<UE_FFieldPathProperty>().GetTypeStr()};
    }
    else if (str == "SoftClassProperty")
    {
        type = {UEPropertyType::SoftClassProperty, this->Cast<UE_FSoftClassProperty>().GetTypeStr()};
    }
    return type;

}


IFProperty UE_FProperty::GetInterface() const { return IFProperty(this); }

// 寻找子类FProperty特有成员的偏移，实际就是FProperty的大小，游戏不同可能会有内存对齐的差异
uintptr_t UE_FProperty::FindSubFPropertyBaseOffset() const
{
    uintptr_t offset = 0;
    if (mem.Read<uintptr_t>(this->GetAddress() + offsets.FProperty.Size) < 0xffffffffff && mem.Read<uintptr_t>(this->GetAddress() + offsets.FProperty.Size) > 0x1000)
    {
        offset = offsets.FProperty.Size;
    }
    else if (mem.Read<uintptr_t>(this->GetAddress() + offsets.FProperty.Size + sizeof(int32_t)) < 0xffffffffff && mem.Read<uintptr_t>(this->GetAddress() + offsets.FProperty.Size + sizeof(int32_t)) > 0x1000)
    {
        offset = offsets.FProperty.Size + sizeof(int32_t);
    }
    else if (mem.Read<uintptr_t>(this->GetAddress() + offsets.FProperty.Size + sizeof(void *)) < 0xffffffffff && mem.Read<uintptr_t>(this->GetAddress() + offsets.FProperty.Size + sizeof(void *)) > 0x1000)
    {
        offset = offsets.FProperty.Size + sizeof(void *);
    }
    return offset;
}

UE_UStruct UE_FStructProperty::GetStruct() const
{
    static uintptr_t offset = 0;
    if (offset == 0)
    {
        offset = FindSubFPropertyBaseOffset();
    }
    return offset ? mem.Read<UE_UStruct>(this->GetAddress() + offset) : UE_UStruct();
}

std::string UE_FStructProperty::GetTypeStr() const
{
    return "struct " + GetStruct().GetCppName();
}

UE_UClass UE_FObjectPropertyBase::GetPropertyClass() const
{
    static uintptr_t offset = 0;
    if (offset == 0)
    {
        offset = FindSubFPropertyBaseOffset();
    }
    return offset ? mem.Read<UE_UClass>(this->GetAddress() + offset) : UE_UClass();
}

std::string UE_FObjectPropertyBase::GetTypeStr() const
{
    return "struct " + GetPropertyClass().GetCppName() + "*";
}

UE_FProperty UE_FArrayProperty::GetInner() const
{
    static uintptr_t offset = 0;
    if (offset == 0)
    {
        offset = FindSubFPropertyBaseOffset();
    }
    return offset ? mem.Read<UE_FProperty>(this->GetAddress() + offset) : UE_FProperty();
}

std::string UE_FArrayProperty::GetTypeStr() const
{
    return "struct TArray<" + GetInner().GetType().second + ">";
}

UE_UEnum UE_FByteProperty::GetEnum() const
{
    static uintptr_t offset = 0;
    if (offset == 0)
    {
        offset = FindSubFPropertyBaseOffset();
    }
    if (offset == 0) return UE_UEnum();

    auto e = mem.Read<UE_UEnum>(this->GetAddress() + offset);
    return (e.GetAddress() && e.IsA<UE_UEnum>()) ? e : UE_UEnum();
}

std::string UE_FByteProperty::GetTypeStr() const
{
    auto e = GetEnum();
    if (e.GetAddress())
        return "enum class " + e.GetName();
    return "uint8_t";
}

uint8_t UE_FBoolProperty::GetFieldSize() const
{
    return mem.Read<uint8_t>(this->GetAddress() + offsets.FProperty.Size);
}

uint8_t UE_FBoolProperty::GetByteOffset() const
{
    return mem.Read<uint8_t>(this->GetAddress() + offsets.FProperty.Size + 1);
}

uint8_t UE_FBoolProperty::GetByteMask() const
{
    return mem.Read<uint8_t>(this->GetAddress() + offsets.FProperty.Size + 2);
}

uint8_t UE_FBoolProperty::GetFieldMask() const
{
    return mem.Read<uint8_t>(this->GetAddress() + offsets.FProperty.Size + 3);
}

std::string UE_FBoolProperty::GetTypeStr() const
{
    if (GetFieldMask() == 0xFF)
    {
        return "bool";
    }
    return "uint8_t";
}

UE_FProperty UE_FEnumProperty::GetUnderlayingProperty() const
{
    static uintptr_t off = 0;
    if (off == 0)
    {
        auto p = mem.Read<UE_FProperty>(this->GetAddress() + offsets.FProperty.Size);
        if (p.GetAddress() && p.GetName() == "UnderlyingType")
        {
            off = offsets.FProperty.Size;
        }
        else
        {
            p = mem.Read<UE_FProperty>(this->GetAddress() + offsets.FProperty.Size - sizeof(void *));
            if (p.GetAddress() && p.GetName() == "UnderlyingType")
            {
                off = offsets.FProperty.Size + sizeof(void *);
            }
        }
        return off == 0 ? UE_FProperty() : p;
    }

    return mem.Read<UE_FProperty>(this->GetAddress() + off);
}

UE_UEnum UE_FEnumProperty::GetEnum() const
{
    static uintptr_t off = 0;
    if (off == 0)
    {
        auto e = mem.Read<UE_UEnum>(this->GetAddress() + offsets.FProperty.Size + sizeof(void *));
        if (e.GetAddress() && e.IsA<UE_UEnum>())
        {
            off = offsets.FProperty.Size + sizeof(void *);
        }
        else
        {
            e = mem.Read<UE_UEnum>(this->GetAddress() + offsets.FProperty.Size);
            if (e.GetAddress() && e.IsA<UE_UEnum>())
            {
                off = offsets.FProperty.Size + (sizeof(void *) * 2);
            }
        }
        return off == 0 ? UE_UEnum() : e;
    }

    return mem.Read<UE_UEnum>(this->GetAddress() + off);
}

std::string UE_FEnumProperty::GetTypeStr() const
{
    if (GetEnum().GetAddress())
        return "enum class " + GetEnum().GetName();

    return GetUnderlayingProperty().GetType().second;
}

UE_UClass UE_FClassProperty::GetMetaClass() const
{
    static uintptr_t offset = 0;
    if (offset == 0)
    {
        offset = FindSubFPropertyBaseOffset();
    }
    return offset ? mem.Read<UE_UClass>(this->GetAddress() + offset + sizeof(void *)) : UE_UClass();
}

std::string UE_FClassProperty::GetTypeStr() const
{
    return "struct " + GetMetaClass().GetCppName() + "*";
}

std::string UE_FSoftClassProperty::GetTypeStr() const
{
    auto className = GetMetaClass().GetAddress() ? GetMetaClass().GetCppName() : GetPropertyClass().GetCppName();
    return "struct TSoftClassPtr<struct " + className + "*>";
}

UE_FProperty UE_FSetProperty::GetElementProp() const
{
    static uintptr_t offset = 0;
    if (offset == 0)
    {
        offset = FindSubFPropertyBaseOffset();
    }
    return offset ? mem.Read<UE_FProperty>(this->GetAddress() + offset) : UE_FProperty();
}

std::string UE_FSetProperty::GetTypeStr() const
{
    return "struct TSet<" + GetElementProp().GetType().second + ">";
}

UE_FProperty UE_FMapProperty::GetKeyProp() const
{
    static uintptr_t offset = 0;
    if (offset == 0)
    {
        offset = FindSubFPropertyBaseOffset();
    }
    return offset ? mem.Read<UE_FProperty>(this->GetAddress() + offset) : UE_FProperty();
}

UE_FProperty UE_FMapProperty::GetValueProp() const
{
    static uintptr_t offset = 0;
    if (offset == 0)
    {
        offset = FindSubFPropertyBaseOffset();
    }
    return offset ? mem.Read<UE_FProperty>(this->GetAddress() + offset + sizeof(void *)) : UE_FProperty();
}

std::string UE_FMapProperty::GetTypeStr() const
{
    //printf("address: %p %s key: %s %p\n", this->GetAddress(), this->GetName().c_str(),GetKeyProp().GetName().c_str(),GetKeyProp().GetAddress());
    return "struct TMap<" + GetKeyProp().GetType().second + ", " + GetValueProp().GetType().second + ">";
}

UE_UClass UE_FInterfaceProperty::GetInterfaceClass() const
{
    static uintptr_t offset = 0;
    if (offset == 0)
    {
        offset = FindSubFPropertyBaseOffset();
    }
    return offset ? mem.Read<UE_UClass>(this->GetAddress() + offset) : UE_UClass();
}

std::string UE_FInterfaceProperty::GetTypeStr() const
{
    return "struct TScriptInterface<I" + GetInterfaceClass().GetName() + ">";
}

int32_t UE_FFieldPathProperty::GetPropertyName() const
{
    //这个地方是个指针，指向的类第一个属性是FName
    return mem.Read<int32_t>(mem.Read<uintptr_t>(this->GetAddress() + offsets.FProperty.Size));
}

std::string UE_FFieldPathProperty::GetTypeStr() const
{
    return "struct TFieldPath<F" + GetNameByIndex(GetPropertyName()) + ">";
}

uintptr_t UE_UFunction::GetFunc() const
{
    return mem.Read<uintptr_t>(this->GetAddress() + offsets.UFunction.Func);
}

int8_t UE_UFunction::GetNumParams() const
{
    return mem.Read<int8_t>(this->GetAddress() + offsets.UFunction.NumParams);
}

int16_t UE_UFunction::GetParamSize() const
{
    return mem.Read<int16_t>(this->GetAddress() + offsets.UFunction.ParamSize);
}

uint32_t UE_UFunction::GetFunctionEFlags() const
{
    return mem.Read<uint32_t>(this->GetAddress() + offsets.UFunction.EFunctionFlags);
}

std::string UE_UFunction::GetFunctionFlags() const
{
    auto flags = GetFunctionEFlags();
    std::string result;
    if (flags == FUNC_None)
    {
        result = "None";
    }
    else
    {
        if (flags & FUNC_Final)
        {
            result += "Final|";
        }
        if (flags & FUNC_RequiredAPI)
        {
            result += "RequiredAPI|";
        }
        if (flags & FUNC_BlueprintAuthorityOnly)
        {
            result += "BlueprintAuthorityOnly|";
        }
        if (flags & FUNC_BlueprintCosmetic)
        {
            result += "BlueprintCosmetic|";
        }
        if (flags & FUNC_Net)
        {
            result += "Net|";
        }
        if (flags & FUNC_NetReliable)
        {
            result += "NetReliable";
        }
        if (flags & FUNC_NetRequest)
        {
            result += "NetRequest|";
        }
        if (flags & FUNC_Exec)
        {
            result += "Exec|";
        }
        if (flags & FUNC_Native)
        {
            result += "Native|";
        }
        if (flags & FUNC_Event)
        {
            result += "Event|";
        }
        if (flags & FUNC_NetResponse)
        {
            result += "NetResponse|";
        }
        if (flags & FUNC_Static)
        {
            result += "Static|";
        }
        if (flags & FUNC_NetMulticast)
        {
            result += "NetMulticast|";
        }
        if (flags & FUNC_UbergraphFunction)
        {
            result += "UbergraphFunction|";
        }
        if (flags & FUNC_MulticastDelegate)
        {
            result += "MulticastDelegate|";
        }
        if (flags & FUNC_Public)
        {
            result += "Public|";
        }
        if (flags & FUNC_Private)
        {
            result += "Private|";
        }
        if (flags & FUNC_Protected)
        {
            result += "Protected|";
        }
        if (flags & FUNC_Delegate)
        {
            result += "Delegate|";
        }
        if (flags & FUNC_NetServer)
        {
            result += "NetServer|";
        }
        if (flags & FUNC_HasOutParms)
        {
            result += "HasOutParms|";
        }
        if (flags & FUNC_HasDefaults)
        {
            result += "HasDefaults|";
        }
        if (flags & FUNC_NetClient)
        {
            result += "NetClient|";
        }
        if (flags & FUNC_DLLImport)
        {
            result += "DLLImport|";
        }
        if (flags & FUNC_BlueprintCallable)
        {
            result += "BlueprintCallable|";
        }
        if (flags & FUNC_BlueprintEvent)
        {
            result += "BlueprintEvent|";
        }
        if (flags & FUNC_BlueprintPure)
        {
            result += "BlueprintPure|";
        }
        if (flags & FUNC_EditorOnly)
        {
            result += "EditorOnly|";
        }
        if (flags & FUNC_Const)
        {
            result += "Const|";
        }
        if (flags & FUNC_NetValidate)
        {
            result += "NetValidate|";
        }
        if (result.size())
        {
            result.erase(result.size() - 1);
        }
    }
    return result;
}

UE_UClass UE_UFunction::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Function");
    return obj;
}



UE_UField UE_UField::GetNext() const
{
    if (!this->GetAddress())
        return 0;

    return mem.Read<UE_UField>(this->GetAddress() + offsets.UField.Next);
}

UE_UClass UE_UField::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Field");
    return obj;
}

TArray<uintptr_t> UE_UEnum::GetNames() const
{
    return mem.Read<TArray<uintptr_t>>(this->GetAddress() + offsets.UEnum.Names);
}

std::string UE_UEnum::GetName() const
{
    std::string name = UE_UField::GetName();
    if (!name.empty() && name[0] != 'E')
        return "E" + name;
    return name;
}

UE_UClass UE_UEnum::StaticClass()
{
    static UE_UClass obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Enum");
    return obj;
}

UE_UClass UE_UScriptStruct::StaticClass()
{
    static UE_UClass obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.ScriptStruct");
    return obj;
}

std::string UE_FFieldClass::GetName() const
{
    return GetNameByIndex(mem.Read<int32_t>(this->GetAddress()));
}

UE_FField UE_FField::GetNext() const
{
    return mem.Read<UE_FField>(this->GetAddress() + offsets.FField.Next);
}

std::string UE_FField::GetName() const
{
    return GetNameByIndex(mem.Read<int32_t>(this->GetAddress() + offsets.FField.NamePrivate));
}

UE_FFieldClass UE_FField::GetClass() const
{
    return mem.Read<UE_FFieldClass>(this->GetAddress() + offsets.FField.ClassPrivate);
}

std::string UE_UObject::GetCppName() const
{
    if (!object) return "";

    std::string name;
    if (IsA<UE_UClass>())
    {
        for (auto c = Cast<UE_UStruct>(); c.GetAddress(); c = c.GetSuper())
        {
            if (c == UE_AActor::StaticClass())
            {
                name = "A";
                break;
            }
            else if (c == UE_UObject::StaticClass())
            {
                name = "U";
                break;
            }
            else if (c == UE_UInterface::StaticClass())
            {
                name = "I";
                break;
            }
        }
    }
    else
    {
        name = "F";
    }

    name += GetName();
    return name;
}





std::string UE_UDoubleProperty::GetTypeStr() const { return "double"; }

UE_UClass UE_UDoubleProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.DoubleProperty");
    return obj;
}

UE_UStruct UE_UStructProperty::GetStruct() const
{
    return mem.Read<UE_UStruct>(this->GetAddress() + offsets.UProperty.Size);
}

std::string UE_UStructProperty::GetTypeStr() const
{
    return "struct " + GetStruct().GetCppName();
}

UE_UClass UE_UStructProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.StructProperty");
    return obj;
}

std::string UE_UNameProperty::GetTypeStr() const { return "struct FName"; }

UE_UClass UE_UNameProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.NameProperty");
    return obj;
}

UE_UClass UE_UObjectPropertyBase::GetPropertyClass() const
{
    return mem.Read<UE_UClass>(this->GetAddress() + offsets.UProperty.Size);
}

std::string UE_UObjectPropertyBase::GetTypeStr() const
{
    return "struct " + GetPropertyClass().GetCppName() + "*";
}

UE_UClass UE_UObjectPropertyBase::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.ObjectPropertyBase");
    return obj;
}

UE_UClass UE_UObjectProperty::GetPropertyClass() const
{
    return mem.Read<UE_UClass>(this->GetAddress() + offsets.UProperty.Size);
}

std::string UE_UObjectProperty::GetTypeStr() const
{
    return "struct " + GetPropertyClass().GetCppName() + "*";
}

UE_UClass UE_UObjectProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.ObjectProperty");
    return obj;
}

UE_UProperty UE_UArrayProperty::GetInner() const
{
    return mem.Read<UE_UProperty>(this->GetAddress() + offsets.UProperty.Size);
}

std::string UE_UArrayProperty::GetTypeStr() const
{
    return "struct TArray<" + GetInner().GetType().second + ">";
}

UE_UClass UE_UArrayProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.ArrayProperty");
    return obj;
}

UE_UEnum UE_UByteProperty::GetEnum() const
{
    auto e = mem.Read<UE_UEnum>(this->GetAddress() + offsets.UProperty.Size);
    return (e.GetAddress() && e.IsA<UE_UEnum>()) ? e : 0;
}

std::string UE_UByteProperty::GetTypeStr() const
{
    auto e = GetEnum();
    if (e.GetAddress())
        return "enum class " + e.GetName();
    return "uint8_t";
}

UE_UClass UE_UByteProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.ByteProperty");
    return obj;
}

uint8_t UE_UBoolProperty::GetFieldSize() const
{
    return mem.Read<uint8_t>(this->GetAddress() + offsets.UProperty.Size);
}

uint8_t UE_UBoolProperty::GetByteOffset() const
{
    return mem.Read<uint8_t>(this->GetAddress() + offsets.UProperty.Size + 1);
}

uint8_t UE_UBoolProperty::GetByteMask() const
{
    return mem.Read<uint8_t>(this->GetAddress() + offsets.UProperty.Size + 2);
}

uint8_t UE_UBoolProperty::GetFieldMask() const
{
    return mem.Read<uint8_t>(this->GetAddress() + offsets.UProperty.Size + 3);
}

std::string UE_UBoolProperty::GetTypeStr() const
{
    /** Mask of the field with the property value. Either equal to ByteMask or 255 in case of 'bool' type. */
    if (GetFieldMask() == 0xFF)
    {
        return "bool";
    }
    return "uint8_t";
}

UE_UClass UE_UBoolProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.BoolProperty");
    return obj;
}

std::string UE_UFloatProperty::GetTypeStr() const { return "float"; }

UE_UClass UE_UFloatProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.FloatProperty");
    return obj;
}

std::string UE_UIntProperty::GetTypeStr() const { return "int"; }

UE_UClass UE_UIntProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.IntProperty");
    return obj;
}

std::string UE_UInt16Property::GetTypeStr() const { return "int16_t"; }

UE_UClass UE_UInt16Property::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Int16Property");
    return obj;
}

std::string UE_UInt64Property::GetTypeStr() const { return "int64_t"; }

UE_UClass UE_UInt64Property::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Int64Property");
    return obj;
}

std::string UE_UInt8Property::GetTypeStr() const { return "uint8_t"; }

UE_UClass UE_UInt8Property::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Int8Property");
    return obj;
}

std::string UE_UUInt16Property::GetTypeStr() const { return "uint16_t"; }

UE_UClass UE_UUInt16Property::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.UInt16Property");
    return obj;
}

std::string UE_UUInt32Property::GetTypeStr() const { return "uint32_t"; }

UE_UClass UE_UUInt32Property::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.UInt32Property");
    return obj;
}

std::string UE_UInt32Property::GetTypeStr() const { return "int32_t"; }

UE_UClass UE_UInt32Property::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.Int32Property");
    return obj;
}

std::string UE_UUInt64Property::GetTypeStr() const { return "uint64_t"; }

UE_UClass UE_UUInt64Property::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.UInt64Property");
    return obj;
}

std::string UE_UTextProperty::GetTypeStr() const { return "struct FText"; }

UE_UClass UE_UTextProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.TextProperty");
    return obj;
}

std::string UE_UStrProperty::GetTypeStr() const { return "struct FString"; }

UE_UClass UE_UStrProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.StrProperty");
    return obj;
}

UE_UProperty UE_UEnumProperty::GetUnderlayingProperty() const
{
    return mem.Read<UE_UProperty>(this->GetAddress() + offsets.UProperty.Size);
}

UE_UEnum UE_UEnumProperty::GetEnum() const
{
    auto e = mem.Read<UE_UEnum>(this->GetAddress() + offsets.UProperty.Size);
    return (e.GetAddress() && e.IsA<UE_UEnum>()) ? e : 0;
}

std::string UE_UEnumProperty::GetTypeStr() const
{
    if (GetEnum().GetAddress())
        return "enum class " + GetEnum().GetName();

    return GetUnderlayingProperty().GetType().second;
}

UE_UClass UE_UEnumProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.EnumProperty");
    return obj;
}

UE_UClass UE_UClassProperty::GetMetaClass() const
{
    return mem.Read<UE_UClass>(this->GetAddress() + offsets.UProperty.Size + sizeof(void *));
}

std::string UE_UClassProperty::GetTypeStr() const
{
    return "struct " + GetMetaClass().GetCppName() + "*";
}

UE_UClass UE_UClassProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.ClassProperty");
    return obj;
}

std::string UE_USoftClassProperty::GetTypeStr() const
{
    auto className = GetMetaClass().GetAddress() ? GetMetaClass().GetCppName() : GetPropertyClass().GetCppName();
    return "struct TSoftClassPtr<struct " + className + "*>";
}

UE_UProperty UE_USetProperty::GetElementProp() const
{
    return mem.Read<UE_UProperty>(this->GetAddress() + offsets.UProperty.Size);
}

std::string UE_USetProperty::GetTypeStr() const
{
    return "struct TSet<" + GetElementProp().GetType().second + ">";
}

UE_UClass UE_USetProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.SetProperty");
    return obj;
}

UE_UProperty UE_UMapProperty::GetKeyProp() const
{
    return mem.Read<UE_UProperty>(this->GetAddress() + offsets.UProperty.Size);
}

UE_UProperty UE_UMapProperty::GetValueProp() const
{
    return mem.Read<UE_UProperty>(this->GetAddress() + offsets.UProperty.Size + sizeof(void *));
}

std::string UE_UMapProperty::GetTypeStr() const
{
    return "struct TMap<" + GetKeyProp().GetType().second + ", " + GetValueProp().GetType().second + ">";
}

UE_UClass UE_UMapProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.MapProperty");
    return obj;
}

UE_UProperty UE_UInterfaceProperty::GetInterfaceClass() const
{
    return mem.Read<UE_UProperty>(this->GetAddress() + offsets.UProperty.Size);
}

std::string UE_UInterfaceProperty::GetTypeStr() const
{
    return "struct TScriptInterface<" + GetInterfaceClass().GetType().second + ">";
}

UE_UClass UE_UInterfaceProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.InterfaceProperty");
    return obj;
}

std::string UE_UMulticastDelegateProperty::GetTypeStr() const
{
    return "struct FScriptMulticastDelegate";
}

UE_UClass UE_UMulticastDelegateProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.MulticastDelegateProperty");
    return obj;
}

std::string UE_UWeakObjectProperty::GetTypeStr() const
{
    return "struct TWeakObjectPtr<" + this->Cast<UE_UStructProperty>().GetTypeStr() + ">";
}

UE_UClass UE_UWeakObjectProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.WeakObjectProperty");
    return obj;
}

std::string UE_ULazyObjectProperty::GetTypeStr() const
{
    return "struct TLazyObjectPtr<" + this->Cast<UE_UStructProperty>().GetTypeStr() + ">";
}

UE_UClass UE_ULazyObjectProperty::StaticClass()
{
    static auto obj = GetObjects().FindObject<UE_UClass>("Class CoreUObject.LazyObjectProperty");
    return obj;
}



