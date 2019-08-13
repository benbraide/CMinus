#include "io_stream_binary_reader.h"

cminus::io::stream_binary_reader::stream_binary_reader(std::istream &stream)
	: stream_(stream){
	offset_ = 0u;
	size_ = static_cast<std::size_t>(-1);
}

cminus::io::stream_binary_reader::~stream_binary_reader() = default;

std::size_t cminus::io::stream_binary_reader::read_(std::byte *buffer, std::size_t size) const{
	stream_.read(reinterpret_cast<char *>(buffer), size);
	return stream_.gcount();
}

void cminus::io::stream_binary_reader::update_offset_() const{
	stream_.ignore(offset_ - last_offset_);
	last_offset_ = offset_;
}
