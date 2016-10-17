#include "File.h"
#include <fstream>
#include <iostream>

namespace Emu8
{
	File::File()
			: fileStream(), filePath(""), isOpen(false)
	{
	}

	File::File(File& other)
			: fileStream(), filePath(other.filePath), isOpen(false)
	{
		//Ignore returning bool ("QuietOpen").
		open();
	}

	File::File(File&& other)
	: File()
	{
		swap(*this, other);
	}

	File::File(std::string filePath)
	{
		this->filePath = filePath;
	}

	File::~File()
	{
		close();
	}

	File& File::operator=(const File other)
	{
		swap(*this, (File&)other);

		return *this;
	}

	bool File::open()
	{
		close();
		fileStream.open(filePath, std::ios::in | std::ios::out | std::ios::binary);

		if(fileStream.is_open())
		{
			isOpen = true;
			return true;
		}

		return false;
	}

	bool File::open(std::string filePath)
	{
		close();

		this->filePath = filePath;

		fileStream.open(filePath, std::ios::in | std::ios::out | std::ios::binary);

		if(fileStream.is_open())
		{
			isOpen = true;
			return true;
		}

		return false;
	}

	void File::close()
	{
		if(isOpen)
		{
			fileStream.close();
			fileStream.clear();
		}
	}

	int File::size()
	{
		openIfClosed();

		if(isOpen)
		{
			int currentPos = (int)fileStream.tellg();
			fileStream.seekg(0, std::ios::end);
			int fileSize = (int)fileStream.tellg();
			fileStream.seekg(currentPos, std::ios::beg);
			return fileSize;
		}

		return 0;
	}

	void File::readAll(char* charArray)
	{
		fileStream.read(charArray, size());
	}

	void File::openIfClosed()
	{
		if(!isOpen && filePath != "")
		{
			open();
		}
	}
}
