#include "io_buffer_binary_reader.h"

cminus::io::buffer_binary_reader::buffer_binary_reader(std::byte *stream, std::size_t size, std::size_t offset)
	: stream_(stream){
	offset_ = offset;
	size_ = size;
}

cminus::io::buffer_binary_reader::~buffer_binary_reader() = default;

std::size_t cminus::io::buffer_binary_reader::read_(std::byte *buffer, std::size_t size) const{
	memcpy(buffer, (stream_ + offset_), size);
	return size;
}

void cminus::io::buffer_binary_reader::update_offset_() const{}
