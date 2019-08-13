#pragma once

#include "io_binary_reader.h"

namespace cminus::io{
	class stream_binary_reader : public binary_reader{
	public:
		explicit stream_binary_reader(std::istream &stream);

		virtual ~stream_binary_reader();

	protected:
		virtual std::size_t read_(std::byte *buffer, std::size_t size) const override;

		virtual void update_offset_() const override;

		std::istream &stream_;
		mutable std::size_t last_offset_ = 0u;
	};
}
