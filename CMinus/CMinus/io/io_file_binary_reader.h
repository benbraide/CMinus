#pragma once

#include <fstream>

#include "io_binary_reader.h"

namespace cminus::io{
	class file_binary_reader : public binary_reader{
	public:
		explicit file_binary_reader(const std::string &file_name, std::size_t offset = 0u);

		explicit file_binary_reader(std::ifstream &stream);

		virtual ~file_binary_reader();

	protected:
		virtual std::size_t read_(std::byte *buffer, std::size_t size) const override;

		virtual void update_offset_() const override;

		void init_();

		std::ifstream my_stream_;
		std::ifstream &stream_;
	};
}
