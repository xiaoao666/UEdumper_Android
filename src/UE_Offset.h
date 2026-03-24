#pragma once


struct UE_Offset;


struct UE_Offset
{
    //这三个是需要手动定位的
    uintptr_t GName_offset = 0xD9A4B40;
    uintptr_t GUObjectArray_offset = 0xD9C06B8;
    uintptr_t GWorld_offset = 0xDF3A198;

    uintptr_t GName = 0;
    uintptr_t GWorld = 0;
    uintptr_t GUObjectArray = 0;
    uintptr_t UE_base = 0;
    struct
    {
        uintptr_t FNameEntry_offset = 0x40;
        uintptr_t CurrentBlock_offset = 0x38;
        uintptr_t NameEntry_size = 0x2;
    } FNameEntry;
    struct
    {
        uintptr_t InternalIndex = 0xc;
        uintptr_t ClassPrivate = 0x10;
        uintptr_t NamePrivate = 0x18;
        uintptr_t OuterPrivate = 0x20;
        uintptr_t size = 0x28;
    } UObject;
    struct
    {
        uintptr_t PersistentLevel_offset = 0x30;
    } world;
    struct
    {
        uintptr_t Actors_offset = 0x98;
    } ULevel;
    struct
    {
        uintptr_t Objects_offset = 0x10;
        uintptr_t MaxElements_offset = 0x20;
        uintptr_t NumElements_offset = 0x24;
    } ObjectArray;
    struct
    {
        uintptr_t SuperStruct = 0x40;
        uintptr_t Children = 0x48;
        uintptr_t ChildProperties = 0x50;
        uintptr_t PropertiesSize = 0x58;
    } UStruct;
    struct
    {
        uintptr_t Next = 0x28;
    } UField;
    struct
    {
        uintptr_t Names = 0x40;
    } UEnum;
    struct
    {
        uintptr_t Size = 0x8;
    } FName;
    struct
    {
        uintptr_t ClassPrivate = 0x8;
        uintptr_t Next = 0x20;
        uintptr_t NamePrivate = 0x28;
    } FField;
    struct
    {
        uintptr_t ArrayDim = 0x34;
        uintptr_t ElementSize = 0x38;
        uintptr_t PropertyFlags = 0x40;
        uintptr_t Offset_Internal = 0x4c;
        uintptr_t Size = 0x78;
    } UProperty;
    struct
    {
        uintptr_t ArrayDim = 0x34;
        uintptr_t ElementSize = 0x38;
        uintptr_t PropertyFlags = 0x40;
        uintptr_t Offset_Internal = 0x4c;
        uintptr_t Size = 0x78;
    } FProperty;
    struct
    {
        uintptr_t EFunctionFlags = 0xb0;
        uintptr_t NumParams = 0xb4;
        uintptr_t ParamSize = 0xb6;
        uintptr_t Func = 0xd8;
    } UFunction;
};

extern UE_Offset offsets;




/*正常UE版本结构体偏移
struct
{
    uintptr_t FNameEntry_offset = 0x40;
    uintptr_t CurrentBlock_offset = 0x38;
    uintptr_t NameEntry_size = 0x2;
} FNameEntry;
struct
{
    uintptr_t InternalIndex = 0xc;
    uintptr_t ClassPrivate = 0x10;
    uintptr_t NamePrivate = 0x18;
    uintptr_t OuterPrivate = 0x20;
    uintptr_t size = 0x28;
} UObject;
struct
{
    uintptr_t PersistentLevel_offset = 0x30;
} world;
struct
{
    uintptr_t Actors_offset = 0x98;
} ULevel;
struct
{
    uintptr_t Objects_offset = 0x10;
    uintptr_t MaxElements_offset = 0x20;
    uintptr_t NumElements_offset = 0x24;
} ObjectArray;
struct
{
    uintptr_t SuperStruct = 0x30;
    uintptr_t Children = 0x38;
    uintptr_t ChildProperties = 0x40;
    uintptr_t PropertiesSize = 0x48;
} UStruct;
struct
{
    uintptr_t Names = 0x40;
} UEnum;
struct
{
    uintptr_t Size = 0x8;
} FName;
struct
{
    uintptr_t ClassPrivate = 0x8;
    uintptr_t Next = 0x20;
    uintptr_t NamePrivate = 0x28;
} FField;
struct
{
    uintptr_t ArrayDim = 0x34;
    uintptr_t ElementSize = 0x38;
    uintptr_t PropertyFlags = 0x40;
    uintptr_t Offset_Internal = 0x4c;
    uintptr_t Size = 0x78;
} UProperty;
struct
{
    uintptr_t ArrayDim = 0x34;
    uintptr_t ElementSize = 0x38;
    uintptr_t PropertyFlags = 0x40;
    uintptr_t Offset_Internal = 0x4c;
    uintptr_t Size = 0x78;
} FProperty;
struct
{
    uintptr_t Next = 0x28;
} UField;
struct
{
    uintptr_t EFunctionFlags = 0xa0;
    uintptr_t NumParams = 0xa4;
    uintptr_t ParamSize = 0xa6;
    uintptr_t Func = 0xc8;
} UFunction;
 */
