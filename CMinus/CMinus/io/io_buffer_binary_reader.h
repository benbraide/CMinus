#pragma once

#include "io_binary_reader.h"

namespace cminus::io{
	class buffer_binary_reader : public binary_reader{
	public:
		buffer_binary_reader(std::byte *stream, std::size_t size, std::size_t offset = 0u);

		virtual ~buffer_binary_reader();

	protected:
		virtual std::size_t read_(std::byte *buffer, std::size_t size) const override;

		virtual void update_offset_() const override;

		mutable std::byte *stream_;
	};
}
