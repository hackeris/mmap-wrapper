//
// Created by hackeris on 2017/3/16.
//

#ifndef MMAP_WRAPPER_MAPFILE_H
#define MMAP_WRAPPER_MAPFILE_H

#include <iostream>
#include <string>

#ifdef _MSC_VER

#include <Windows.h>

class FileMap {

	static std::wstring c2w(const char *pc);

public:
	FileMap(const std::wstring& filename);

	FileMap(const std::string &filename);

	const inline void *get() const {
		return _buf;
	}

	inline size_t size() const {
		return _size;
	}

	inline bool ok() const {
		return _buf != nullptr;
	}

	~FileMap();

private:
	FileMap(const FileMap &) = delete;

	FileMap(FileMap &&) = delete;

	FileMap &operator=(const FileMap &) = delete;

private:
	HANDLE _fd;
	HANDLE _map;
	const void *_buf;
	size_t _size;
};

#else   // for Unix like OS

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

class FileMap {

public:
	FileMap(const std::string &filename);

	const inline void *get() const {
		return _buf;
	}

	inline size_t size() const {
		return _size;
	}

	inline bool ok() const {
		return _buf != nullptr;
	}

	~FileMap();

private:
	FileMap(const FileMap &) = delete;

	FileMap(FileMap &&) = delete;

	FileMap &operator=(const FileMap &) = delete;

private:
	int _fd;
	const void *_buf;
	size_t _size;
};

#endif

#endif //MMAP_WRAPPER_MAPFILE_H
