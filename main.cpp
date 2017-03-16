#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <tuple>

#ifdef _MSC_VER

#include <Windows.h>

class FileMap {

	std::wstring c2w(const char *pc){
		std::wstring val = L"";

		if (NULL == pc){
			return val;
		}
		size_t size_of_wc;
		size_t destlen = mbstowcs(0, pc, 0);
		if (destlen == (size_t)(-1)){
			return val;
		}
		size_of_wc = destlen + 1;
		wchar_t * pw = new wchar_t[size_of_wc];
		mbstowcs(pw, pc, size_of_wc);
		val = pw;
		delete[] pw;
		return val;
	}

public:
	FileMap(const std::string &filename) : FileMap(c2w(filename.c_str())) {}

	FileMap(const std::wstring& filename) : _fd(INVALID_HANDLE_VALUE), _buf(nullptr), _size(0) {
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

	const void *get() const {
		return _buf;
	}

	off_t size() const {
		return _size;
	}

	bool ok() const {
		return _buf != nullptr;
	}

	~FileMap() {
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

private:
	FileMap(const FileMap &) = delete;

	FileMap(FileMap &&) = delete;

	FileMap &operator=(const FileMap &) = delete;

private:
	HANDLE _fd;
	HANDLE _map;
	const void *_buf;
	off_t _size;
};

#else

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

class FileMap {

public:
	FileMap(const std::string &filename) : _fd(-1), _buf(nullptr), _size(0) {
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

	const void *get() const {
		return _buf;
	}

	off_t size() const {
		return _size;
	}

	bool ok() const {
		return _buf != nullptr;
	}

	~FileMap() {
		if (_fd > 0) {
			close(_fd);
		}
		if (_buf != nullptr) {
			munmap(const_cast<void *>(_buf), size_t(_size));
		}
	}

private:
	FileMap(const FileMap &) = delete;

	FileMap(FileMap &&) = delete;

	FileMap &operator=(const FileMap &) = delete;

private:
	int _fd;
	const void *_buf;
	off_t _size;
};

#endif

std::vector<std::tuple<size_t, size_t>>
filemap_into_blocks(const FileMap &fileMap, int nblocks, char spliter = '\n') {

	using block_type = std::tuple<size_t, size_t>;
	using std::max;
	using std::min;

	const char *buffer = (const char *)fileMap.get();
	off_t size = fileMap.size();

	std::vector<block_type> blocks;

	size_t blockSize = (size_t)(size / nblocks);
	off_t pos = 0;
	for (int i = 0; i < nblocks; i++) {
		pos = max((off_t)(i * blockSize), pos);
		off_t blockStart = pos;
		pos = (off_t)min(size_t(blockSize + pos), (size_t)size);
		while (pos < size && buffer[pos++] != spliter);
		blocks.push_back(std::make_tuple(blockStart, pos));
	}

	return std::move(blocks);
};

int main(int argc, char **argv) {

	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
		return -1;
	}
	FileMap fileMap(argv[1]);

	if (!fileMap.ok()) {
		std::cout << "Open file map error." << std::endl;
		return -1;
	}

	const char *pFile = static_cast<const char *> (fileMap.get());
	off_t size = fileMap.size();

	auto blocks = filemap_into_blocks(fileMap, 4);

	for (const auto &blk : blocks) {
		size_t start, end;
		std::tie(start, end) = blk;
		std::cout << "(" << start << "," << end << ")" << std::endl;
	}

	return 0;
}
