#include "io_binary_writer.h"

cminus::io::binary_writer::~binary_writer() = default;

std::size_t cminus::io::binary_writer::get_offset() const{
	return offset_;
}

std::size_t cminus::io::binary_writer::get_size() const{
	return size_;
}

std::size_t cminus::io::binary_writer::set(std::byte value, std::size_t size){
	return set_(value, (((size_ - offset_) < size) ? (size_ - offset_) : size));
}

std::size_t cminus::io::binary_writer::write(const std::byte *buffer, std::size_t size){
	auto write_size = write_(buffer, (((size_ - offset_) < size) ? (size_ - offset_) : size));;
	offset_ += write_size;
	return write_size;
}

std::size_t cminus::io::binary_writer::write_scalar(const std::any &buffer){
	if (auto typed_buffer = std::any_cast<const std::byte *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const char *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const unsigned char *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const wchar_t *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const __int8 *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const unsigned __int8 *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const __int16 *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const unsigned __int16 *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const __int32 *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const unsigned __int32 *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const __int64 *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const unsigned __int64 *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const float *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const double *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const long double *>(&buffer); typed_buffer != nullptr)
		return write_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<const std::string *>(&buffer); typed_buffer != nullptr)
		return write_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	if (auto typed_buffer = std::any_cast<const std::wstring *>(&buffer); typed_buffer != nullptr)
		return write_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	return 0u;
}

std::size_t cminus::io::binary_writer::advance(std::size_t size){
	return advance_(((size_ - offset_) < size) ? (size_ - offset_) : size);
}
