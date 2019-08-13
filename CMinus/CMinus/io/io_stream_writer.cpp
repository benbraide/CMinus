#include "io_stream_writer.h"

cminus::io::stream_writer::stream_writer(std::ostream &stream)
	: stream_(stream){}

cminus::io::stream_writer::~stream_writer() = default;

std::size_t cminus::io::stream_writer::get_offset() const{
	return 0u;
}

std::size_t cminus::io::stream_writer::get_size() const{
	return static_cast<std::size_t>(-1);
}

std::size_t cminus::io::stream_writer::set(std::byte value, std::size_t size){
	std::fill_n(std::ostream_iterator<char>(stream_), size, static_cast<char>(value));
	return size;
}

std::size_t cminus::io::stream_writer::write(const std::byte *buffer, std::size_t size){
	stream_.write(reinterpret_cast<const char *>(buffer), size);
	return size;
}

std::size_t cminus::io::stream_writer::write_scalar(const std::any &buffer){
	if (auto typed_buffer = std::any_cast<const std::byte *>(&buffer); typed_buffer != nullptr)
		return write_(*reinterpret_cast<const char *>(*typed_buffer));

	if (auto typed_buffer = std::any_cast<const char *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const unsigned char *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const wchar_t *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const __int8 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const unsigned __int8 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const __int16 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const unsigned __int16 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const __int32 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const unsigned __int32 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const __int64 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const unsigned __int64 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const float *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const double *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const long double *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const std::string *>(&buffer); typed_buffer != nullptr)
		return write_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	if (auto typed_buffer = std::any_cast<const std::wstring *>(&buffer); typed_buffer != nullptr)
		return write_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	return 0u;
}

cminus::io::wide_stream_writer::wide_stream_writer(std::wostream &stream)
	: stream_(stream){}

cminus::io::wide_stream_writer::~wide_stream_writer() = default;

std::size_t cminus::io::wide_stream_writer::get_offset() const{
	return 0u;
}

std::size_t cminus::io::wide_stream_writer::get_size() const{
	return static_cast<std::size_t>(-1);
}

std::size_t cminus::io::wide_stream_writer::set(std::byte value, std::size_t size){
	std::fill_n(std::ostream_iterator<wchar_t, wchar_t>(stream_), size, static_cast<wchar_t>(value));
	return size;
}

std::size_t cminus::io::wide_stream_writer::write(const std::byte *buffer, std::size_t size){
	stream_.write(reinterpret_cast<const wchar_t *>(buffer), (size / sizeof(wchar_t)));
	return (size - (size % sizeof(wchar_t)));
}

std::size_t cminus::io::wide_stream_writer::write_scalar(const std::any &buffer){
	if (auto typed_buffer = std::any_cast<const wchar_t *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const __int16 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const unsigned __int16 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const __int32 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const unsigned __int32 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const __int64 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const unsigned __int64 *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const float *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const double *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const long double *>(&buffer); typed_buffer != nullptr)
		return write_(**typed_buffer);

	if (auto typed_buffer = std::any_cast<const std::string *>(&buffer); typed_buffer != nullptr)
		return write_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	if (auto typed_buffer = std::any_cast<const std::wstring *>(&buffer); typed_buffer != nullptr)
		return write_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	return 0u;
}
