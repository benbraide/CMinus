#include "io_file_binary_writer.h"

cminus::io::file_binary_writer::file_binary_writer(const std::string &file_name, std::size_t offset)
	: my_stream_(file_name.data(), std::ios::binary), stream_(my_stream_){
	offset_ = offset;
	init_();
}

cminus::io::file_binary_writer::file_binary_writer(std::ofstream &stream)
	: stream_(stream){
	offset_ = 0u;
	init_();
}

cminus::io::file_binary_writer::~file_binary_writer() = default;

std::size_t cminus::io::file_binary_writer::set_(std::byte value, std::size_t size){
	std::fill_n(std::ostreambuf_iterator(stream_), size, static_cast<char>(value));
	return size;
}

std::size_t cminus::io::file_binary_writer::write_(const std::byte *buffer, std::size_t size){
	stream_.write(reinterpret_cast<const char *>(buffer), size);
	return size;
}

std::size_t cminus::io::file_binary_writer::advance_(std::size_t size){
	stream_.seekp(size, std::ios::beg);
	return size;
}

void cminus::io::file_binary_writer::init_(){
	if (!stream_.is_open())
		return;

	auto current = stream_.tellp();
	auto begin = stream_.seekp(0, std::ios::beg).tellp();

	size_ = static_cast<std::size_t>(-1);
	stream_.seekp(current, std::ios::beg);

	if (0u < offset_)
		stream_.seekp(offset_, std::ios::cur);

	offset_ = (stream_.tellp() - begin);
}
