#include "Android-Memory-Debug.hpp"



size_t judgSize(int type)
{
    switch (type)
    {
        case DWORD:
        case FLOAT:
        case XOR:
            return 4;
        case BYTE:
            return sizeof(char);
        case WORD:
            return sizeof(short);
        case QWORD:
            return sizeof(long);
        case DOUBLE:
            return sizeof(double);
    }
    return 4;
}

int memContrast(char *str, char *mem_flags)
{

    // A内存的判断并不完善,有能力的可以自己优化
    if (strstr(mem_flags, "rw") != NULL && strlen(str) == 0)
        return Mem_A;
    if ((strstr(str, "/data/app/") != NULL && strstr(mem_flags, "r-xp"))
        || (strstr(mem_flags, "r-xp") != nullptr && strlen(str) == 0))
        return Mem_Xa;
    if (strstr(str, "/dev/ashmem/") != NULL)
        return Mem_As;
    if (strstr(str, "/system/fonts/") != NULL)
        return Mem_B;
    if (strstr(str, "/system/framework/") != NULL)
        return Mem_Xs;
    if (strcmp(str, "[anon:libc_malloc]") == 0)
        return Mem_Ca;
    if (strstr(str, ":bss") != NULL)
        return Mem_Cb;
    if (strstr(str, "/data/data/") != NULL)
        return Mem_Cd;
    if (strstr(str, "[anon:dalvik") != NULL)
        return Mem_J;
    if (strcmp(str, "[stack]") == 0)
        return Mem_S;
    if (strcmp(str, "/dev/kgsl-3d0") == 0)
        return Mem_V;
    return Mem_O;
}


int MemoryDebug::getPidByPackageNames(std::vector < std::string > packageNames)
{
    DIR *dir;
    FILE *fp;
    char filename[32];
    char cmdline[256];
    struct dirent *entry;
    int ptid = -1;

    dir = opendir("/proc");
    while ((entry = readdir(dir)) != NULL)
    {
        ptid = atoi(entry->d_name);
        if (ptid != 0)
        {
            sprintf(filename, "/proc/%d/cmdline", ptid);
            fp = fopen(filename, "r");
            if (fp)
            {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);
                for (auto packageName:packageNames)
                {
                    if (strcmp(packageName.c_str(), cmdline) == 0)
                    {
                        return ptid;
                    }
                }
            }
        }
    }
    closedir(dir);
    return -1;
}



int MemoryDebug::setPackageName(const char *name)
{
    int id = -1;
    DIR *dir;
    FILE *fp;
    char filename[32];
    char cmdline[256];
    struct dirent *entry;
    dir = opendir("/proc");
    while ((entry = readdir(dir)) != NULL)
    {
        id = atoi(entry->d_name);
        if (id != 0)
        {
            sprintf(filename, "/proc/%d/cmdline", id);
            fp = fopen(filename, "r");
            if (fp)
            {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);
                if (strcmp(name, cmdline) == 0)
                {
                    pid = id;
                    return id;
                }
            }
        }
    }
    closedir(dir);
    return -1;
}


std::vector < long >MemoryDebug::getvec()
{
    return res;
}


std::vector < uint8_t > stringToByteArray(const std::string & input)
{
    std::vector < uint8_t > result;
    std::istringstream stream(input);
    char c;
    while (stream >> c)
    {
        result.push_back(c);
    }
    return result;
}


long MemoryDebug::getModuleBase(const char *name, int index)
{
    int i = 0;
    long start = 0, end = 0;
    char line[1024] = { 0 };
    char fname[128];
    sprintf(fname, "/proc/%d/maps", pid);
    FILE *p = fopen(fname, "r");
    if (p)
    {
        while (fgets(line, sizeof(line), p))
        {
            if (strstr(line, name) != NULL)
            {
                i++;
                if (i == index)
                {
                    sscanf(line, "%lx-%lx", &start, &end);
                    break;
                }
            }
        }
        fclose(p);
    }
    return start;
}

long MemoryDebug::getBssModuleBase(const char *name)
{
    FILE *fp;
    int cnt = 0;
    long start;
    char tmp[256];
    fp = NULL;
    char line[1024];
    char fname[128];
    sprintf(fname, "/proc/%d/maps", pid);
    fp = fopen(fname, "r");
    while (!feof(fp))
    {
        fgets(tmp, 256, fp);
        if (cnt == 1)
        {
            if (strstr(tmp, "[anon:.bss]") != NULL)
            {
                sscanf(tmp, "%lx-%*lx", &start);
                break;
            }
            else
            {
                cnt = 0;
            }
        }
        if (strstr(tmp, name) != NULL)
        {
            cnt = 1;
        }
    }
    return start;
}

size_t MemoryDebug::pwritev(long address, void *buffer, size_t size)
{
    struct iovec iov_WriteBuffer, iov_WriteOffset;
    iov_WriteBuffer.iov_base = buffer;
    iov_WriteBuffer.iov_len = size;
    iov_WriteOffset.iov_base = (void *)address;
    iov_WriteOffset.iov_len = size;
    return syscall(SYS_process_vm_writev, pid, &iov_WriteBuffer, 1, &iov_WriteOffset, 1, 0);
}

size_t MemoryDebug::preadv(long address, void *buffer, size_t size)
{
    struct iovec iov_ReadBuffer, iov_ReadOffset;
    iov_ReadBuffer.iov_base = buffer;
    iov_ReadBuffer.iov_len = size;
    iov_ReadOffset.iov_base = (void *)address;
    iov_ReadOffset.iov_len = size;
    return syscall(SYS_process_vm_readv, pid, &iov_ReadBuffer, 1, &iov_ReadOffset, 1, 0);
}




long MemoryDebug::ReadDword64(long address)
{
    long local_ptr = 0;
    preadv(address, &local_ptr, 8);
    return local_ptr;
}

long int MemoryDebug::ReadPointer(long address)
{
    long int local_value = 0;
    preadv(address, &local_value, 4);
    return local_value;
}

int MemoryDebug::ReadDword(long address)
{
    int local_value = 0;
    preadv(address, &local_value, 4);
    return local_value;
}




float MemoryDebug::ReadFloat(long address)
{
    float local_value = 0;
    preadv(address, &local_value, 4);
    return local_value;
}

long MemoryDebug::ReadLong(long address)
{
    long local_value = 0;
    preadv(address, &local_value, 8);
    return local_value;
}

std::string MemoryDebug::ReadString(long address, size_t maxLength)
{

    //printf("ReadString: %p\n", address);
    std::vector<char> chars(maxLength, '\0');
    preadv(address, chars.data(), maxLength);
    std::string str = "";
    for (size_t i = 0; i < chars.size(); i++)
    {
        if (chars[i] == '\0')
            break;

        str.push_back(chars[i]);
    }

    chars.clear();
    chars.shrink_to_fit();

    if ((int)str[0] == 0 && str.size() == 1)
        return "";


    return str;
}

bool MemoryDebug::WriteString(long address, const std::string &str)
{
    // 计算需要写入的总长度：字符串长度 + 1个'\0'结束符
    size_t writeLength = str.size() + 1;
    // 调用底层写内存函数，写入字符串内容（包括'\0'结束符）
    if (pwritev(address, (void*)str.c_str(), writeLength) <= 0)
        return false;

    return true;
}

void getRoot(char **argv)
{
    char shellml[64];
    sprintf(shellml, "su -c %s", *argv);
    if (getuid() != 0)
    {
        system(shellml);
        exit(1);
    }
}


MemoryDebug mem;
UE_Offset offsets;