#pragma once

#include "io_binary_writer.h"

namespace cminus::io{
	class buffer_binary_writer : public binary_writer{
	public:
		buffer_binary_writer(std::byte *stream, std::size_t size, std::size_t offset = 0u);

		virtual ~buffer_binary_writer();

	protected:
		virtual std::size_t set_(std::byte value, std::size_t size) override;

		virtual std::size_t write_(const std::byte *buffer, std::size_t size) override;

		virtual std::size_t advance_(std::size_t size) override;

		std::byte *stream_;
	};
}
