#pragma once

#include <string>

#include "io_writer.h"

namespace cminus::io{
	class binary_writer : public writer{
	public:
		virtual ~binary_writer();

		virtual std::size_t get_offset() const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t set(std::byte value, std::size_t size) override;

		virtual std::size_t write(const std::byte *buffer, std::size_t size) override;

		virtual std::size_t write_scalar(const std::any &buffer) override;

		using writer::write_scalar;

		virtual std::size_t advance(std::size_t size);

	protected:
		virtual std::size_t set_(std::byte value, std::size_t size) = 0;

		virtual std::size_t write_(const std::byte *buffer, std::size_t size) = 0;

		virtual std::size_t advance_(std::size_t size) = 0;

		mutable std::size_t offset_ = 0u;
		std::size_t size_ = 0u;
	};
}
