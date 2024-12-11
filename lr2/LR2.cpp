
#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <string>
#include <chrono>

const size_t BUFFER_SIZE = 65536; // Размер буфера для чтения/записи
const int MAX_CONCURRENT_IO = 4; // Количество параллельных операций ввода-вывода
const char XOR_KEY = 'A';

struct IOContext {
	OVERLAPPED overlapped;
	std::vector<char> buffer;
	bool isRead; // True для чтения, false для записи
};


/*
void ProcessData(std::vector<char>& data) {
	// Пример обработки данных: сортировка
	auto end = std::find(data.begin(), data.end(), '\0');
	std::sort(data.begin(), end);
	//std::sort(data.begin(), data.end());
	auto nullPos = std::find(data.begin(), data.end(), '\0');

	// Если найдено, обрезаем вектор
	if (nullPos != data.end()) {
		data.erase(nullPos, data.end()); // Удаление всех элементов начиная с nullPos
	}
}

*/

void ProcessData(std::vector<char>& data) {
	// Шифрование посимвольно с помощью XOR
	auto nullPos = std::find(data.begin(), data.end(), '\0');

	// Если найдено, обрезаем вектор
	if (nullPos != data.end()) {
		data.erase(nullPos, data.end()); // Удаление всех элементов начиная с nullPos
	}
	for (auto& ch : data) {
		ch ^= XOR_KEY; // Применение XOR для каждого символа текста
	}
}

void CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
	// Обработка завершения операции ввода-вывода
	IOContext* context = reinterpret_cast<IOContext*>(lpOverlapped);
	if (dwErrorCode == 0) {
		std::cout << (context->isRead ? "Read" : "Write") << " operation completed successfully." << std::endl;
	}
	else {
		std::cerr << "Asynchronous operation failed with error code: " << dwErrorCode << std::endl;
	}
}

void AsyncReadWriteExample(const std::wstring& fileName) {
	HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to open file." << std::endl;
		return;
	}

	LARGE_INTEGER fileSize;
	GetFileSizeEx(hFile, &fileSize);
	size_t totalSize = static_cast<size_t>(fileSize.QuadPart);
	size_t numChunks = (totalSize + BUFFER_SIZE - 1) / BUFFER_SIZE;

	std::vector<IOContext> ioContexts(MAX_CONCURRENT_IO);
	for (int i = 0; i < MAX_CONCURRENT_IO; ++i) {
		ioContexts[i].buffer.resize(BUFFER_SIZE);
		ioContexts[i].isRead = true;
		memset(&ioContexts[i].overlapped, 0, sizeof(OVERLAPPED));
	}

	auto startTime = std::chrono::high_resolution_clock::now();

	size_t chunkIndex = 0;
	while (chunkIndex < numChunks) {
		for (int i = 0; i < MAX_CONCURRENT_IO && chunkIndex < numChunks; ++i) {
			IOContext& context = ioContexts[i];
			context.overlapped.Offset = static_cast<DWORD>(chunkIndex * BUFFER_SIZE);
			context.isRead = true;

			if (!ReadFileEx(hFile, context.buffer.data(), BUFFER_SIZE, &context.overlapped, FileIOCompletionRoutine)) {
				DWORD error = GetLastError();
				if (error != ERROR_IO_PENDING) {
					std::cerr << "Failed to read file. Error: " << error << std::endl;
					CloseHandle(hFile);
					return;
				}
			}
			SleepEx(0, TRUE); // Ожидание завершения асинхронной операции

			// Обработка данных
			std::thread processingThread([&context]() {
				ProcessData(context.buffer);
				});
			processingThread.join();

			// Асинхронная запись
			context.isRead = false;
			size_t size = context.buffer.size();
			if (size < BUFFER_SIZE)
			{
				if (!WriteFileEx(hFile, context.buffer.data(), size, &context.overlapped, FileIOCompletionRoutine)) {
					DWORD error = GetLastError();
					if (error != ERROR_IO_PENDING) {
						std::cerr << "Failed to write file. Error: " << error << std::endl;
						CloseHandle(hFile);
						return;
					}
				}
				SleepEx(0, TRUE);
			}
			else {
				if (!WriteFileEx(hFile, context.buffer.data(), BUFFER_SIZE, &context.overlapped, FileIOCompletionRoutine)) {
					DWORD error = GetLastError();
					if (error != ERROR_IO_PENDING) {
						std::cerr << "Failed to write file. Error: " << error << std::endl;
						CloseHandle(hFile);
						return;
					}
				}
				SleepEx(0, TRUE); // Ожидание завершения асинхронной операции
			}
			++chunkIndex;
		}
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedTime = endTime - startTime;

	CloseHandle(hFile);

	std::cout << "Time taken for asynchronous read/write and processing: " << elapsedTime.count() << " seconds." << std::endl;
}

int main() {
	std::wstring fileName = L"example.txt";

	// Выполнение асинхронного ввода-вывода
	AsyncReadWriteExample(fileName);

	return 0;
}
