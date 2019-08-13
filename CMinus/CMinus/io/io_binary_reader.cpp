#include "io_binary_reader.h"

cminus::io::binary_reader::~binary_reader() = default;

std::size_t cminus::io::binary_reader::get_offset() const{
	return offset_;
}

std::size_t cminus::io::binary_reader::get_size() const{
	return size_;
}

std::size_t cminus::io::binary_reader::get_bytes_remaining() const{
	return (size_ - offset_);
}

std::size_t cminus::io::binary_reader::ignore(std::size_t size) const{
	if ((size_ - offset_) < size)
		size = (size_ - offset_);

	offset_ += size;
	update_offset_();

	return size;
}

std::size_t cminus::io::binary_reader::read(std::byte *buffer, std::size_t size) const{
	auto read_size = read_(buffer, (((size_ - offset_) < size) ? (size_ - offset_) : size));
	offset_ += read_size;
	return read_size;
}

std::size_t cminus::io::binary_reader::read_scalar(const std::any &buffer) const{
	if (auto typed_buffer = std::any_cast<std::byte *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<char *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<unsigned char *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<wchar_t *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<__int8 *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<unsigned __int8 *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<__int16 *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<unsigned __int16 *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<__int32 *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<unsigned __int32 *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<__int64 *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<unsigned __int64 *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<float *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<double *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<long double *>(&buffer); typed_buffer != nullptr)
		return read_buffer(*typed_buffer, 1u);

	if (auto typed_buffer = std::any_cast<std::string *>(&buffer); typed_buffer != nullptr)
		return read_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	if (auto typed_buffer = std::any_cast<std::wstring *>(&buffer); typed_buffer != nullptr)
		return read_buffer((*typed_buffer)->data(), (*typed_buffer)->size());

	return 0u;
}
