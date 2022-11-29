#pragma once
#include <string>
#include <cstdint>
#include <fstream>
#include <unordered_map>

class BinaryWriter
{
public:
	BinaryWriter(std::string filepath);
	~BinaryWriter();

	// Write raw data to file
	void WriteRaw(uint8_t* data, size_t count);
	
	// Write fixed values to file
	void WriteUByte(uint8_t b);
	void WriteUShort(uint16_t s);
	void WriteUInt(uint32_t i);
	void WriteByte(int8_t b);
	void WriteShort(int16_t s);
	void WriteInt(int32_t i);
	void WriteFloat(float f);
	void WriteDouble(double d);

	// TODO: Write Vec3 etc?

	// Write a 32 bit marker value to be filled in later with the location marked as "ident"
	void WritePointer(std::string ident);

	// Mark a location to be refered to by a pointer
	void MarkAddress(std::string ident);

	// This is what actually goes back and fills in all the marker values with the actual addresses
	// they point to.
	void Finish();

	void SeekAbs(size_t addr);
	void SeekRel(int32_t off);

	size_t TellPos();

private:
	// The file we're writing
	std::fstream fs;

	// A list of pointers we need to fix when we Finish
	std::vector<std::tuple<std::string, size_t>> pointerLocations;
	
	// The locations we've marked
	std::unordered_map<std::string, size_t> markedLocations;
};