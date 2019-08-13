#pragma once

#include <string>

#include "io_reader.h"

namespace cminus::io{
	class binary_reader : public reader{
	public:
		virtual ~binary_reader();

		virtual std::size_t get_offset() const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t get_bytes_remaining() const override;

		virtual std::size_t ignore(std::size_t size) const override;

		virtual std::size_t read(std::byte *buffer, std::size_t size) const override;

		virtual std::size_t read_scalar(const std::any &buffer) const override;

		using reader::read_scalar;

	protected:
		virtual std::size_t read_(std::byte *buffer, std::size_t size) const = 0;

		virtual void update_offset_() const = 0;

		mutable std::size_t offset_ = 0u;
		std::size_t size_ = 0u;
	};
}
