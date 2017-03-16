#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <tuple>
#include "FileMap.h"

std::vector<std::tuple<size_t, size_t>>
filemap_into_blocks(const FileMap &fileMap, int nblocks, char spliter = '\n') {

    using block_type = std::tuple<size_t, size_t>;
    using std::max;
    using std::min;

    const char *buffer = (const char *) fileMap.get();
    off_t size = fileMap.size();

    std::vector<block_type> blocks;

    size_t blockSize = (size_t) (size / nblocks);
    off_t pos = 0;
    for (int i = 0; i < nblocks; i++) {
        pos = max((off_t) (i * blockSize), pos);
        off_t blockStart = pos;
        pos = (off_t) min(size_t(blockSize + pos), (size_t) size);
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
