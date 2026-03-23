#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "Android-Memory-Debug.hpp"
#include "UE_Offset.h"
#include "UE_Tool.h"

class UE_UPackage
{
public:
    struct Member
    {
        std::string Type;
        std::string Name;
        std::string extra;  // extra comment
        uint32_t Offset = 0;
        uint32_t Size = 0;
    };
    struct Function
    {
        std::string Name;
        std::string FullName;
        std::string CppName;
        std::string Params;
        uint32_t EFlags = 0;
        std::string Flags;
        int8_t NumParams = 0;
        int16_t ParamSize = 0;
        uintptr_t Func = 0;
    };
    struct Struct
    {
        std::string Name;
        std::string FullName;
        std::string CppName;
        uint32_t Inherited = 0;
        uint32_t Size = 0;
        std::vector<Member> Members;
        std::vector<Function> Functions;
    };
    struct Enum
    {
        std::string FullName;
        std::string CppName;
        std::vector<std::pair<std::string, uint64_t>> Members;
    };

private:
    std::pair<UE_UObject, std::vector<UE_UObject>> *Package;

public:
    std::vector<Struct> Classes;
    std::vector<Struct> Structures;
    std::vector<Enum> Enums;

    static void GenerateFunction(const UE_UFunction &fn, Function *out);
    static void GenerateStruct(const UE_UStruct &object, std::vector<Struct> &arr);
    static void GenerateEnum(const UE_UEnum &object, std::vector<Enum> &arr);

    static void GenerateBitPadding(std::vector<Member> &members, uint32_t offset, uint8_t bitOffset, uint8_t size);
    static void GeneratePadding(std::vector<Member> &members, uint32_t offset, uint32_t size);
    static void FillPadding(const UE_UStruct &object, std::vector<Member> &members, uint32_t &offset, uint8_t &bitOffset, uint32_t end);

    static void AppendEnums(std::vector<Enum> &arr, std::ostream &out);
    static void AppendStructs(std::vector<Struct> &arr, std::ostream &out);

    void Process();

    UE_UPackage(std::pair<UE_UObject, std::vector<UE_UObject>> &package) : Package(&package) {};
    inline UE_UObject GetObject() const { return UE_UObject(Package->first); }

};



void UE_UPackage::GenerateBitPadding(std::vector<Member> &members, uint32_t offset, uint8_t bitOffset, uint8_t size)
{
    Member padding;
    padding.Type = "uint8_t";
    //padding.Name = fmt::format("BitPad_0x{:X}_{} : {}", offset, bitOffset, size);
    std::ostringstream os;
    os << "BitPad_0x" << std::hex << offset << '_' << int(bitOffset) << " : " << int(size);
    padding.Name = os.str();
    padding.Offset = offset;
    padding.Size = 1;
    members.push_back(padding);
}

void UE_UPackage::GeneratePadding(std::vector<Member> &members, uint32_t offset, uint32_t size)
{
    Member padding;
    padding.Type = "uint8_t";
    //padding.Name = fmt::format("Pad_0x{:X}[0x{:X}]", offset, size);
    std::ostringstream os;
    os << "Pad_0x" << std::hex << offset << "[0x" << std::hex << size << "]";
    padding.Name = os.str();
    padding.Offset = offset;
    padding.Size = size;
    members.push_back(padding);
}

void UE_UPackage::FillPadding(const UE_UStruct &object, std::vector<Member> &members, uint32_t &offset, uint8_t &bitOffset, uint32_t end)
{
    (void)object;

    if (bitOffset && bitOffset < 8)
    {
        UE_UPackage::GenerateBitPadding(members, offset, bitOffset, 8 - bitOffset);
        bitOffset = 0;
        offset++;
    }

    if (offset != end)
    {
        GeneratePadding(members, offset, end - offset);
        offset = end;
    }
}


void UE_UPackage::GenerateFunction(const UE_UFunction &fn, Function *out)
{
    out->Name = fn.GetName();
    out->FullName = fn.GetFullName();
    out->EFlags = fn.GetFunctionEFlags();
    out->Flags = fn.GetFunctionFlags();
    out->NumParams = fn.GetNumParams();
    out->ParamSize = fn.GetParamSize();
    out->Func = fn.GetFunc();

    auto generateParam = [&](IProperty *prop)
    {
        auto flags = prop->GetPropertyFlags();

        // if property has 'ReturnParm' flag
        if (flags & CPF_ReturnParm)
        {
            out->CppName = prop->GetType().second + " " + fn.GetName();
        }
            // if property has 'Parm' flag
        else if (flags & CPF_Parm)
        {
            if (prop->GetArrayDim() > 1)
            {
                out->Params += prop->GetType().second + "* " + prop->GetName() + ", ";
            }
            else
            {
                if (flags & CPF_OutParm)
                {
                    out->Params += prop->GetType().second + " " + prop->GetName() + ", ";
                }
                else
                {
                    out->Params += prop->GetType().second + " " + prop->GetName() + ", ";
                }
            }
        }
    };

    for (auto prop = fn.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
    {
        auto propInterface = prop.GetInterface();
        generateParam(&propInterface);
    }
    for (auto prop = fn.GetChildren().Cast<UE_UProperty>(); prop.GetAddress(); prop = prop.GetNext().Cast<UE_UProperty>())
    {
        auto propInterface = prop.GetInterface();
        generateParam(&propInterface);
    }
    if (out->Params.size())
    {
        out->Params.erase(out->Params.size() - 2);
    }

    if (out->CppName.size() == 0)
    {
        out->CppName = "void " + fn.GetName();
    }
}


template <typename T>
constexpr uint64_t GetMaxOfType()
{
    return (1ull << (sizeof(T) * 0x8ull)) - 1;
}




void UE_UPackage::GenerateEnum(const UE_UEnum &object, std::vector<Enum> &arr)
{
    Enum e;
    e.FullName = object.GetFullName();

    uint64_t nameSize = offsets.FName.Size;
    uint64_t pairSize = nameSize + sizeof(int64_t);

    auto names = object.GetNames();
    uint64_t max = 0;
    for (int32_t i = 0; i < names.Num(); i++)
    {
        auto pair = (uintptr_t)names.GetData() + i * pairSize;
        auto name = GetNameByIndex(mem.Read<uint32_t>((uintptr_t)pair));
        auto pos = name.find_last_of(':');
        if (pos != std::string::npos)
            name = name.substr(pos + 1);
        auto value = mem.Read<uint64_t>(pair + nameSize);
        if (value > max)
            max = value;

        e.Members.emplace_back(name, value);
    }

    // enum values 按照顺序自动递增
    auto isUninitializedEnum = [](Enum &e) -> bool
    {
        if (e.Members.size() > 1)
        {
            for (size_t i = 1; i < e.Members.size(); ++i)
            {
                if (e.Members[i].second <= e.Members[i - 1].second)
                    return true;
            }
        }
        return false;
    };

    if (isUninitializedEnum(e))
    {
        max = e.Members.size();
        for (size_t i = 0; i < e.Members.size(); ++i)
        {
            e.Members[i].second = i;
        }
    }

    const char *type = nullptr;

    if (max > GetMaxOfType<uint32_t>())
        type = " : uint64_t";
    else if (max > GetMaxOfType<uint16_t>())
        type = " : uint32_t";
    else if (max > GetMaxOfType<uint8_t>())
        type = " : uint16_t";
    else
        type = " : uint8_t";

    e.CppName = "enum class " + object.GetName() + type;

    if (e.Members.size())
    {
        arr.push_back(e);
    }

}



void UE_UPackage::GenerateStruct(const UE_UStruct &object, std::vector<Struct> &arr)
{
    Struct s;
    s.Name = object.GetName();
    s.FullName = object.GetFullName();

    s.CppName = "struct " + object.GetCppName();

    s.Inherited = 0;
    s.Size = object.GetSize();

    if (s.Size == 0)
    {
        arr.push_back(s);
        return;
    }
    auto super = object.GetSuper();
    if (super.GetAddress())
    {
        s.CppName += " : ";
        s.CppName += super.GetCppName();
        s.Inherited = super.GetSize();
    }

    uint32_t offset = s.Inherited;
    uint8_t bitOffset = 0;

    auto generateMember = [&](IProperty *prop, Member *m)
    {
        auto arrDim = prop->GetArrayDim();
        m->Size = prop->GetSize() * arrDim;
        if (m->Size == 0)
        {
            return;
        }  // this shouldn't be zero

        auto type = prop->GetType();
        m->Type = type.second;
        m->Name = prop->GetName();
        m->Offset = prop->GetOffset();

        if (m->Offset > offset)
        {
            UE_UPackage::FillPadding(object, s.Members, offset, bitOffset, m->Offset);
        }
        if (type.first == UEPropertyType::BoolProperty && *(uint32_t *)type.second.data() != 'loob')
        {
            auto boolProp = prop;
            auto mask = boolProp->GetFieldMask();
            uint8_t zeros = 0, ones = 0;
            while (mask & ~1)
            {
                mask >>= 1;
                zeros++;
            }
            while (mask & 1)
            {
                mask >>= 1;
                ones++;
            }
            if (zeros > bitOffset)
            {
                UE_UPackage::GenerateBitPadding(s.Members, offset, bitOffset, zeros - bitOffset);
                bitOffset = zeros;
            }
            m->Name += " : " + std::to_string(ones);;
            bitOffset += ones;

            if (bitOffset == 8)
            {
                offset++;
                bitOffset = 0;
            }

            std::stringstream ss;
            ss << "Mask(0x" << std::hex << std::uppercase
               << boolProp->GetFieldMask() << ")";
            m->extra = ss.str();
        }
        else
        {
            if (arrDim > 1)
            {
                std::stringstream ss;
                ss << "ArrayDim[0x" << std::hex << std::uppercase << arrDim << "]";
                m->extra = ss.str();
            }

            offset += m->Size;
        }
    };
    //UE_FProperty和UE_UProperty是UE版本相关的
    for (auto prop = object.GetChildProperties().Cast<UE_FProperty>(); prop; prop = prop.GetNext().Cast<UE_FProperty>())
    {
        Member m;
        auto propInterface = prop.GetInterface();
        generateMember(&propInterface, &m);
        s.Members.push_back(m);
    }

    for (auto child = object.GetChildren(); child.GetAddress(); child = child.GetNext())
    {
        if (child.IsA<UE_UFunction>())
        {
            auto fn = child.Cast<UE_UFunction>();
            Function f;
            GenerateFunction(fn, &f);
            s.Functions.push_back(f);
        }
        else if (child.IsA<UE_UProperty>())
        {
            auto prop = child.Cast<UE_UProperty>();
            Member m;
            auto propInterface = prop.GetInterface();
            generateMember(&propInterface, &m);
            s.Members.push_back(m);
        }
    }

    if (s.Size > offset)
    {
        UE_UPackage::FillPadding(object, s.Members, offset, bitOffset, s.Size);
    }

    arr.push_back(s);

}



void UE_UPackage::Process()
{
    auto &objects = Package->second;
    for (auto &object : objects)
    {
        if (object.IsA<UE_UClass>())
        {
            GenerateStruct(object.Cast<UE_UStruct>(), Classes);
        }
        else if (object.IsA<UE_UScriptStruct>())
        {
            GenerateStruct(object.Cast<UE_UStruct>(), Structures);
        }
        else if (object.IsA<UE_UEnum>())
        {
            GenerateEnum(object.Cast<UE_UEnum>(), Enums);
        }
    }
}


void UE_UPackage::AppendEnums(std::vector<UE_UPackage::Enum> &arr, std::ostream &out) {
    for (auto &e : arr)
    {
        out << "// Object: " << e.FullName << std::endl;
        out << e.CppName << std::endl;
        out << "{" << std::endl;
        for (auto &member : e.Members)
        {
            out << "    " << member.first << " = " << member.second << "," << std::endl;
        }
        out << "};" << std::endl;
        out << std::endl;
    }
}

void UE_UPackage::AppendStructs(std::vector<Struct> &arr, std::ostream &out)
{
    for (auto &s : arr)
    {
        out << "// Object: " << s.FullName << std::endl;
        out << "// Size: 0x" << std::hex << std::uppercase << s.Size << " (Inherited: 0x" << std::hex << std::uppercase << s.Inherited << ")\n" << s.CppName << std::endl;
        out << "{" << std::endl;

        if (s.Members.size())
        {
            for (auto &m : s.Members)
            {
                out << "\t" << m.Type << " " << m.Name << "; // 0x" << std::hex << std::uppercase << m.Offset << "(0x" << std::hex << std::uppercase << m.Size << ")";
                if (!m.extra.empty())
                {
                    out << "\t\t" << m.extra << std::endl;
                } else{
                    out << std::endl;
                }
            }
        }
        if (s.Functions.size())
        {
            if (s.Members.size())
                out << "\n";

            for (auto &f : s.Functions)
            {
                void *funcOffset = f.Func ? (void *)(f.Func - offsets.UE_base) : nullptr;
                out << "\n\n\t// Object: " << f.FullName << std::endl;
                out << "\t// Flags: [" << f.Flags << "]" << std::endl;
                out << "\t// Offset: " << funcOffset << std::endl;
                out << "\t// Params: [ Num(" << (int)f.NumParams << ") Size(0x" << std::hex << std::uppercase << f.ParamSize << ") ]" << std::endl;
                out << "\t" << f.CppName << "(" << f.Params << ");" << std::endl;
            }
        }
        out << "\n};\n\n";
    }
}