#include "io_buffer_binary_writer.h"

cminus::io::buffer_binary_writer::buffer_binary_writer(std::byte *stream, std::size_t size, std::size_t offset)
	: stream_(stream){
	offset_ = offset;
	size_ = size;
}

cminus::io::buffer_binary_writer::~buffer_binary_writer() = default;

std::size_t cminus::io::buffer_binary_writer::set_(std::byte value, std::size_t size){
	memset((stream_ + offset_), static_cast<int>(value), size);
	return size;
}

std::size_t cminus::io::buffer_binary_writer::write_(const std::byte *buffer, std::size_t size){
	memcpy((stream_ + offset_), buffer, size);
	return size;
}

std::size_t cminus::io::buffer_binary_writer::advance_(std::size_t size){
	return size;
}
