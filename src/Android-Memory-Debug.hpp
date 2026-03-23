// 大部分代码来自于 http://t.csdn.cn/bQCd5
// 我在其基础上对搜索函数进行了优化,并封装了一些常用函数
// 支持编译共享库和二进制
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <fcntl.h>
#include <unordered_map>
#include <sys/mman.h>
#include <future>
#include <algorithm>
#include <condition_variable>
#include <functional>
#include <queue>
#include <cstdlib>
#include <unordered_set>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include "UE_Offset.h"

// 支持的搜索类型
enum
{
	DWORD,
	FLOAT,
	BYTE,
	WORD,
	QWORD,
	XOR,
	DOUBLE,
};

// 支持的内存范围(请参考GG修改器内存范围)
enum
{
	Mem_Auto,					// 所有内存页
	Mem_A,
	Mem_Ca,
	Mem_Cd,
	Mem_Cb,
	Mem_Jh,
	Mem_J,
	Mem_S,
	Mem_V,
	Mem_Xa,
	Mem_Xs,
	Mem_As,
	Mem_B,
	Mem_O,
};

struct MemPage
{
	long start;
	long end;
	char flags[8];
	char name[128];
	void *buf = NULL;
};

struct AddressData
{
	std::vector < long >addrs;
};

// 根据类型判断类型所占字节大小
size_t judgSize(int type);


int memContrast(char *str, char *mem_flags);



class MemoryDebug
{
  private:
	pid_t pid = 0;				// 调试应用的PID
	std::vector < long >res;

	  std::vector < long >ofstmp;
  public:
	// 设置调试的应用包名，返回PID
	int setPackageName(const char *name);
	// 获取模块的基址，@name：模块名，@index：模块在内存中的内存页位置(第几位，从1开始，默认1)
	long getModuleBase(const char *name, int index = 1);
	// 获取模块的BSS基址
	long getBssModuleBase(const char *name);
	// 读内存的基础函数
	size_t preadv(long address, void *buffer, size_t size);
	// 写内存的基础函数
	size_t pwritev(long address, void *buffer, size_t size);
	  template < class T > void Editoffest(T num, int offset);

	  template < class T > void searchoffest(T num, int offset);

	  template < class T > void SearchOffest(T num, int offset);
	// 传lambda进去
	  template < typename F > int searchCall(int type, F && call);
	  template < class T > void EditXa(long address, T newValue);
	// 搜索字符串
	  template < typename T >
		AddressData searchString(const std::string & str, int mem, bool debug = false);
	// 仿memorytools函数
	  template < class T > void searchMem(T value, int type, int mem);

	  template < class T > AddressData searchPointer(std::vector < long >address, long offset,
													 int type, int mem, bool debug = false);

	// 根据值搜索内存，并返回相应地址
	  template < class T > AddressData search(T value, int type, int mem, bool debug = false);
	// 修改内存地址值，返回-1，修改失败，返回1，修改成功
	  template < class T > int edit(T value, uintptr_t address, int type, bool debug = false);

	// 多包名读取
	int getPidByPackageNames(std::vector < std::string > packageNames);
	// 读取一个DWORD(int)数值
	int ReadDword(long address);
	// 读取一个int指针地址数值
	long ReadDword64(long address);
	// 读取一个float类型数值
	float ReadFloat(long address);
	// 读取一个long类型数值
	long ReadLong(long address);
	// 32位指针跳转
	long int ReadPointer(long address);
	// 读取字符串
	std::string ReadString(long address, size_t maxLength = 1024);
	// 写入字符串
	bool WriteString(long address, const std::string &str);

	template<class T> T Read(long address);

	  std::vector < long >getvec();
};



void getRoot(char **argv);




template < class T > void MemoryDebug::searchMem(T value, int type, int mem)
{
	size_t size = judgSize(type);
	MemPage *mp = nullptr;
	AddressData ad;
	char path[128] = "";
	char *line = nullptr;
	size_t mapsize = 0;
	sprintf(path, "/proc/%d/maps", pid);
	auto handle = fopen(path, "r");
	while (getline(&line, &mapsize, handle) > 0)
	{
		mp = (MemPage *) calloc(1, sizeof(MemPage));
		sscanf(line, "%p-%p %s %*p %*p:%*p %*p   %[^\n]%s", &mp->start, &mp->end,
			   mp->flags, mp->name);
		if ((memContrast(mp->name, mp->flags) == mem || mem == Mem_Auto)
			&& strstr(mp->flags, "r") != nullptr)
		{
			std::unique_ptr < T[] > buf(new T[mp->end - mp->start]);
			preadv(mp->start, buf.get(), mp->end - mp->start);
			T *p = std::find(buf.get(), buf.get() + (mp->end - mp->start) / size, value);
			while (p != buf.get() + (mp->end - mp->start) / size)
			{
				res.push_back(mp->start + (p - buf.get()) * size);
				p = std::find(p + 1, buf.get() + (mp->end - mp->start) / size, value);
			}
			free(mp);
		}
	}

	fclose(handle);
}

using namespace std;
template < class T > void MemoryDebug::SearchOffest(T num, int offset)
{
	if (res.size() != 0 && ofstmp.size() == 0)
	{
		for (auto addr:res)
		{
			T local_value = 0;
			preadv(addr + offset, &local_value, sizeof(T));
			if (local_value == num)
			{
				ofstmp.push_back(addr);
			}
		}
		res.clear();
	}
	else
	{

		for (auto addr:ofstmp)
		{
			T local_value = 0;
			preadv(addr + offset, &local_value, sizeof(T));
			if (local_value == num)
			{
				res.push_back(addr);
			}
		}
		ofstmp.clear();
	}
}

template < class T > void MemoryDebug::Editoffest(T num, int offset)
{
	if (res.size() != 0 && ofstmp.size() == 0)
	{
		for (auto addr:res)
		{
			pwritev(addr + offset, &num, sizeof(T));
		}
	}
	else
	{
		for (auto addr:ofstmp)
		{
			pwritev(addr + offset, &num, sizeof(T));
		}
	}
}




template < class T > AddressData MemoryDebug::search(T value, int type, int mem, bool debug)
{
	size_t size = judgSize(type);
	MemPage *mp = nullptr;
	AddressData ad;
	char path[128] = "";
	char *line = nullptr;
	size_t mapsize = 0;
	sprintf(path, "/proc/%d/maps", pid);
	auto handle = fopen(path, "r");
	while (getline(&line, &mapsize, handle) > 0)
	{
		mp = (MemPage *) calloc(1, sizeof(MemPage));
		sscanf(line, "%p-%p %s %*p %*p:%*p %*p   %[^\n]%s", &mp->start, &mp->end,
			   mp->flags, mp->name);
		if ((memContrast(mp->name, mp->flags) == mem || mem == Mem_Auto)
			&& strstr(mp->flags, "r") != nullptr)
		{
			std::unique_ptr < T[] > buf(new T[mp->end - mp->start]);
			preadv(mp->start, buf.get(), mp->end - mp->start);
			T *p = std::find(buf.get(), buf.get() + (mp->end - mp->start) / size, value);
			while (p != buf.get() + (mp->end - mp->start) / size)
			{
				ad.addrs.push_back(mp->start + (p - buf.get()) * size);
				p = std::find(p + 1, buf.get() + (mp->end - mp->start) / size, value);
			}
			free(mp);
		}
	}

	fclose(handle);
	return ad;
}

// 对单个地址的搜索速度还不错
template < class T > AddressData MemoryDebug::searchPointer(std::vector < long >addrs, long offset,
															int type, int mem, bool debug)
{
	size_t size = judgSize(type);
	MemPage *mp = nullptr;
	AddressData ad;
	char path[128] = "";
	char *line = nullptr;
	size_t mapsize = 0;
	sprintf(path, "/proc/%d/maps", pid);
	auto handle = fopen(path, "r");
	while (getline(&line, &mapsize, handle) > 0)
	{
		mp = (MemPage *) calloc(1, sizeof(MemPage));
		sscanf(line, "%p-%p %s %*p %*p:%*p %*p %[^\n]%s", &mp->start, &mp->end, mp->flags,
			   mp->name);
		if ((memContrast(mp->name, mp->flags) == mem || memContrast(mp->name, mp->flags) == Mem_Ca
			 || mem == Mem_Auto) && strstr(mp->flags, "r") != nullptr)


		{
			if (addrs[0] - mp->start <= 0xFFFFFFFF)
			{


				std::vector < long >addrList(addrs.begin(), addrs.end());
				size_t numAddrs = addrList.size();
				std::unique_ptr < T[] > buf(new T[mp->end - mp->start]);
				preadv(mp->start, buf.get(), mp->end - mp->start);
				T *p = buf.get();
				if (numAddrs > 10)
				{
					std::unordered_set < long >addrSet(addrs.begin(), addrs.end());
					while (p != buf.get() + (mp->end - mp->start) / size)
					{
						if (addrSet.find(*p) != addrSet.end())
						{
							ad.addrs.push_back(mp->start + (p - buf.get()) * size);
						}
						p++;
					}
				}
				else
				{
					while (p != buf.get() + (mp->end - mp->start) / size)
					{
						for (size_t i = 0; i < numAddrs; i++)
						{
							if (*p == addrList[i])
							{
								ad.addrs.push_back(mp->start + (p - buf.get()) * size);
								break;
							}
						}
						p++;
					}
				}
			}





			free(mp);
		}
	}

	fclose(handle);
	return ad;
}





template < typename F > int MemoryDebug::searchCall(int mem, F && call)
{
	int ret = 0;
	MemPage *mp = nullptr;
	char path[128] = "";
	char *line = nullptr;
	size_t mapsize = 0;
	sprintf(path, "/proc/%d/maps", pid);
	auto handle = fopen(path, "r");
	while (getline(&line, &mapsize, handle) > 0)
	{
		mp = (MemPage *) calloc(1, sizeof(MemPage));
		sscanf(line, "%p-%p %s %*p %*p:%*p %*p %[^\n]%s", &mp->start, &mp->end, mp->flags,
			   mp->name);
		if ((memContrast(mp->name, mp->flags) == mem || mem == Mem_Auto)
			&& strstr(mp->flags, "r") != nullptr)
		{
			int sz = mp->end - mp->start;
			std::unique_ptr < int[] > buf(new int[sz]);
			preadv(mp->start, buf.get(), sz);
			call(mp->start, sz, buf.get(), res);
		}
		free(mp);
	}
	fclose(handle);
	return ret;
}







template < typename T >
AddressData MemoryDebug::searchString(const std::string & str, int mem, bool debug)
{
	MemPage *mp = nullptr;
	AddressData ad;
	char path[128] = "";
	char *line = nullptr;
	size_t mapsize = 0;
	sprintf(path, "/proc/%d/maps", pid);
	auto handle = fopen(path, "r");

	std::vector < T > bytes;
	for (char c:str)
	{
		bytes.push_back(c);
	}


	while (getline(&line, &mapsize, handle) > 0)
	{
		mp = (MemPage *) calloc(1, sizeof(MemPage));
		sscanf(line, "%p-%p %s %*p %*p:%*p %*p %[^\n]%s", &mp->start, &mp->end, mp->flags,
			   mp->name);
		if ((memContrast(mp->name, mp->flags) == mem || mem == Mem_Auto)
			&& strstr(mp->flags, "r") != nullptr)
		{
			std::unique_ptr < T[] > buf(new T[mp->end - mp->start]);
			preadv(mp->start, buf.get(), mp->end - mp->start);
			for (size_t i = 0; i < mp->end - mp->start; ++i)
			{
				bool match = true;
				for (size_t j = 0; j < bytes.size(); ++j)
				{
					if (buf[i + j] != bytes[j])
					{
						match = false;
						break;
					}
				}
				if (match)
				{
					ad.addrs.push_back(mp->start + i);
				}
			}
			free(mp);
		}
	}
	fclose(handle);
	return ad;
}


template < class T > int MemoryDebug::edit(T value, uintptr_t address, int type, bool debug)
{
	if (-1 == pwritev(address, &value, judgSize(type)))
	{
		if (debug)
			printf("修改失败-> addr:%p\n", address);
		return -1;
	}
	else
	{
		if (debug)
			printf("修改成功-> addr:%p\n", address);
		return 1;
	}
	return -1;
}

template < class T > void MemoryDebug::EditXa(long address, T newValue)
{
	char lj[64];
	int handle;
	sprintf(lj, "/proc/%d/mem", pid);
	handle = open((char*)lj, O_RDWR,0);
	lseek(handle, 0, SEEK_SET);
	pwrite64(handle, &newValue, sizeof(T), address);
	close(handle);
}

template < class T >
T MemoryDebug::Read(long address)
{
	T local_value;
	preadv(address, (void*)(&local_value), sizeof(T));
	return local_value;
}

extern MemoryDebug mem;