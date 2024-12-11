#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <chrono>

std::mutex mtx;
std::vector<char> buffer;

struct ThreadData {
	std::string filename;
	std::streampos start;
	std::streamsize size;
};

DWORD WINAPI readFilePart(LPVOID lpParam) {
	ThreadData* data = (ThreadData*)lpParam;
	std::ifstream file(data->filename, std::ios::binary);
	if (!file) {
		std::cerr << "Error opening file: " << data->filename << std::endl;
		return 1;
	}

	std::vector<char> localBuffer(data->size);
	file.seekg(data->start);
	file.read(localBuffer.data(), data->size);

	std::lock_guard<std::mutex> lock(mtx);
	buffer.insert(buffer.end(), localBuffer.begin(), localBuffer.end());

	delete data; // Освобождаем память
	return 0;
}

int main() {
	std::string filename;
	int numThreads;

	while (true) {
		std::cout << "Enter the filename: ";
		std::cin >> filename;

		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file) {
			std::cerr << "Error opening file: " << filename << std::endl;
			std::cerr << "Please try again." << std::endl;
			continue;
		}

		while (true) {
			std::cout << "Enter the number of threads: ";
			std::cin >> numThreads;
			if (numThreads > 0)
				break;
			else
				std::cout << "Error\n";
		}

		std::streamsize fileSize = file.tellg();
		std::streamsize partSize = fileSize / numThreads;

		auto start = std::chrono::high_resolution_clock::now();

		std::vector<HANDLE> threads(numThreads);
		for (int i = 0; i < numThreads; ++i) {
			ThreadData* data = new ThreadData{ filename, i * partSize, (i == numThreads - 1) ? (fileSize - i * partSize) : partSize };
			threads[i] = CreateThread(nullptr, 0, readFilePart, data, 0, nullptr);
		}

		WaitForMultipleObjects(numThreads, threads.data(), TRUE, INFINITE);

		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = end - start;

		std::cout << "File read in " << elapsed.count() << " seconds." << std::endl;
		std::cout << "Buffer size: " << buffer.size() << " bytes." << std::endl;

		break;
	}

	return 0;
}
