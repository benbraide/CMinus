#include "io_file_binary_reader.h"

cminus::io::file_binary_reader::file_binary_reader(const std::string &file_name, std::size_t offset)
	: my_stream_(file_name.data(), std::ios::binary), stream_(my_stream_){
	offset_ = offset;
	init_();
}

cminus::io::file_binary_reader::file_binary_reader(std::ifstream &stream)
	: stream_(stream){
	offset_ = 0u;
	init_();
}

cminus::io::file_binary_reader::~file_binary_reader() = default;

std::size_t cminus::io::file_binary_reader::read_(std::byte *buffer, std::size_t size) const{
	stream_.read(reinterpret_cast<char *>(buffer), size);
	return stream_.gcount();
}

void cminus::io::file_binary_reader::update_offset_() const{
	stream_.seekg(offset_, std::ios::beg);
}

void cminus::io::file_binary_reader::init_(){
	if (!stream_.is_open())
		return;

	auto current = stream_.tellg();
	auto end = stream_.seekg(0, std::ios::end).tellg();
	auto begin = stream_.seekg(0, std::ios::beg).tellg();

	size_ = static_cast<std::size_t>(end - begin);
	stream_.seekg(current, std::ios::beg);

	if (0u < offset_)
		stream_.seekg(offset_, std::ios::cur);

	offset_ = (stream_.tellg() - begin);
}
