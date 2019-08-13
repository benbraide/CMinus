#include "io_stream_reader.h"

cminus::io::stream_reader::stream_reader(std::istream &stream)
	: stream_(stream){}

cminus::io::stream_reader::~stream_reader() = default;

std::size_t cminus::io::stream_reader::get_offset() const{
	return 0u;
}

std::size_t cminus::io::stream_reader::get_size() const{
	return static_cast<std::size_t>(-1);
}

std::size_t cminus::io::stream_reader::get_bytes_remaining() const{
	return static_cast<std::size_t>(-1);
}

std::size_t cminus::io::stream_reader::ignore(std::size_t size) const{
	stream_.ignore(size);
	return size;
}

std::size_t cminus::io::stream_reader::read(std::byte *buffer, std::size_t size) const{
	stream_.read(reinterpret_cast<char *>(buffer), size);
	return stream_.gcount();
}

std::size_t cminus::io::stream_reader::read_scalar(const std::any &buffer) const{
	if (auto typed_buffer = std::any_cast<std::byte *>(&buffer); typed_buffer != nullptr)
		return read_(*reinterpret_cast<char *>(*typed_buffer));

	if (auto typed_buffer = std::any_cast<char *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<unsigned char *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<wchar_t *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<__int8 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<unsigned __int8 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<__int16 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<unsigned __int16 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<__int32 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<unsigned __int32 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<__int64 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<unsigned __int64 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<float *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<double *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<long double *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<std::string *>(&buffer); typed_buffer != nullptr)
		return read_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	if (auto typed_buffer = std::any_cast<std::wstring *>(&buffer); typed_buffer != nullptr)
		return read_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	return 0u;
}

cminus::io::wide_stream_reader::wide_stream_reader(std::wistream &stream)
	: stream_(stream){}

cminus::io::wide_stream_reader::~wide_stream_reader() = default;

std::size_t cminus::io::wide_stream_reader::get_offset() const{
	return 0u;
}

std::size_t cminus::io::wide_stream_reader::get_size() const{
	return static_cast<std::size_t>(-1);
}

std::size_t cminus::io::wide_stream_reader::get_bytes_remaining() const{
	return static_cast<std::size_t>(-1);
}

std::size_t cminus::io::wide_stream_reader::ignore(std::size_t size) const{
	stream_.ignore(size);
	return size;
}

std::size_t cminus::io::wide_stream_reader::read(std::byte *buffer, std::size_t size) const{
	stream_.read(reinterpret_cast<wchar_t *>(buffer), (size / sizeof(wchar_t)));
	return (stream_.gcount() * sizeof(wchar_t));
}

std::size_t cminus::io::wide_stream_reader::read_scalar(const std::any &buffer) const{
	if (auto typed_buffer = std::any_cast<wchar_t *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<__int16 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<unsigned __int16 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<__int32 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<unsigned __int32 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<__int64 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<unsigned __int64 *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<float *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<double *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<long double *>(&buffer); typed_buffer != nullptr)
		return read_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<std::string *>(&buffer); typed_buffer != nullptr)
		return read_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	if (auto typed_buffer = std::any_cast<std::wstring *>(&buffer); typed_buffer != nullptr)
		return read_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	return 0u;
}
