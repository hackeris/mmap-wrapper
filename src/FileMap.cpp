//
// Created by hackeris on 2017/3/16.
//

#include "FileMap.h"

#ifdef _MSC_VER

std::wstring FileMap::c2w(const char *pc) {
	std::wstring val = L"";

	if (NULL == pc) {
		return val;
	}
	size_t size_of_wc;
	size_t destlen = mbstowcs(0, pc, 0);
	if (destlen == (size_t)(-1)) {
		return val;
	}
	size_of_wc = destlen + 1;
	wchar_t * pw = new wchar_t[size_of_wc];
	mbstowcs(pw, pc, size_of_wc);
	val = pw;
	delete[] pw;
	return val;
}

FileMap::FileMap(const std::wstring& filename) : _fd(INVALID_HANDLE_VALUE), _buf(nullptr), _size(0) {

	_fd = CreateFile(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);
	if (_fd == INVALID_HANDLE_VALUE) {
		std::cout << "createfile failed:" << GetLastError() << std::endl;
		return;
	}

	_map = CreateFileMapping(_fd, NULL, PAGE_READONLY, 0, 0, 0);
	if (_map == NULL) {
		std::cout << "CreateFileMapping failed" << std::endl;
		return;
	}

	_buf = MapViewOfFile(_map, FILE_MAP_READ, 0, 0, 0);
	if (_buf == NULL) {
		std::cout << "MapViewOfFile failed" << std::endl;
		return;
	}

	_size = GetFileSize(_fd, NULL);
}

FileMap::FileMap(const std::string &filename) : FileMap(c2w(filename.c_str())) {

}

FileMap::~FileMap() {
	if (_buf != nullptr) {
		UnmapViewOfFile(_buf);
	}
	if (_map != nullptr) {
		CloseHandle(_map);
	}
	if (_fd != INVALID_HANDLE_VALUE) {
		CloseHandle(_fd);
	}
}

#else   // for Unix like OS

FileMap::FileMap(const std::string &filename) : _fd(-1), _buf(nullptr), _size(0) {
	_fd = open(filename.c_str(), O_RDONLY);
	if (_fd < 0) {
		std::cout << "cannot open file " << filename << " as read." << std::endl;
		return;
	}
	struct stat st;
	fstat(_fd, &st);
	_size = st.st_size;
	_buf = mmap(nullptr, size_t(_size), PROT_READ, MAP_PRIVATE, _fd, 0);
	if (_buf == MAP_FAILED) {
		_buf = nullptr;
		std::cout << "cannot map file " << filename << " as read." << std::endl;
	}
}

FileMap::~FileMap() {
	if (_fd > 0) {
		close(_fd);
	}
	if (_buf != nullptr) {
		munmap(const_cast<void *>(_buf), size_t(_size));
	}
}

#endif
