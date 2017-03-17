#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <tuple>
#include "FileMap.h"

std::vector<std::tuple<size_t, size_t>>
csv_filemap_into_blocks(const FileMap &fileMap, int nblocks, bool skipHeader = true) {

	using block_type = std::tuple<size_t, size_t>;
	using std::max;
	using std::min;

	const char *buffer = (const char *)fileMap.get();
	size_t size = fileMap.size();

	std::vector<block_type> blocks;

	size_t blockSize = (size_t)(size / nblocks);
	size_t pos = 0;

	//	skip table header
	if (skipHeader) {
		while (buffer[pos++] != '\n') { pos++; }
		if (buffer[pos] == '\r') { pos++; }
	}

	for (int i = 0; i < nblocks; i++) {
		pos = max((size_t)(i * blockSize), pos);
		size_t blockStart = pos;
		pos = (size_t)min(size_t(blockSize + pos), (size_t)size);
		while (pos < size && buffer[pos++] != '\n');
		if (buffer[pos] == '\r') {
			pos ++;
		}
		blocks.push_back(std::make_tuple(blockStart, pos));
	}

	return blocks;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <filename> <n blocks>" << std::endl;
        return -1;
    }
    FileMap fileMap(argv[1]);
    int nblocks = atoi(argv[2]);

    if (!fileMap.ok()) {
        std::cout << "Open file map error." << std::endl;
        return -1;
    }

    const char *pFile = static_cast<const char *> (fileMap.get());
    size_t size = fileMap.size();

    auto blocks = csv_filemap_into_blocks(fileMap, nblocks);

    for (const auto &blk : blocks) {
        size_t start, end;
        std::tie(start, end) = blk;
        std::cout << "(" << start << "," << end << ")" << std::endl;
    }

    return 0;
}
