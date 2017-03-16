#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <algorithm>
#include <sys/mman.h>
#include <sys/stat.h>
#include <vector>

#ifdef _MSC_
#else

class FileMap {

    using StringType = std::string;

public:
    FileMap(const StringType &filename) : _fd(-1), _buf(nullptr), _size(0) {
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

    FileMap &operator=(const FileMap &)= delete;

private:
    int _fd;
    const void *_buf;
    off_t _size;
};

#endif

std::vector<std::tuple<size_t, size_t>>
filemap_into_blocks(const FileMap &fileMap, int nblocks, char spliter = '\n') {

    using block_type = std::tuple<size_t, size_t>;

    const char *buffer = (const char *) fileMap.get();
    off_t size = fileMap.size();

    std::vector<block_type> blocks;

    size_t blockSize = (size_t) (size / nblocks);
    off_t pos = 0;
    for (int i = 0; i < nblocks; i++) {
        pos = std::max((off_t) (i * blockSize), pos);
        off_t blockStart = pos;
        pos = (off_t) std::min(size_t(blockSize + pos), (size_t) size);
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

    for (const auto &blk: blocks) {
        size_t start, end;
        std::tie(start, end) = blk;
        std::cout << "(" << start << "," << end << ")" << std::endl;
    }

    return 0;
}