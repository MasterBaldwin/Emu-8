#ifndef EMU_8_FILE_H
#define EMU_8_FILE_H

#include <fstream>
#include <string>

namespace Emu8
{
	class File
	{
	private:
		std::fstream fileStream;
		std::string filePath;
		bool isOpen;

		File();
		void openIfClosed();

	public:
		File(File& other);
		File(File&& other);
		File(std::string filePath);
		~File();
		File& operator=(const File other);
		bool open();
		bool open(std::string filePath);
		void close();
		int size();
		void readAll(char* charArray);

		friend void swap(File& first, File& second)
		{
			using std::swap;

			swap(first.fileStream, second.fileStream);
			swap(first.filePath, second.filePath);
			std::swap(first.isOpen, second.isOpen);
		}
	};
}

#endif //EMU_8_FILE_H
