#pragma once

#include <fstream>

#include "io_binary_writer.h"

namespace cminus::io{
	class file_binary_writer : public binary_writer{
	public:
		explicit file_binary_writer(const std::string &file_name, std::size_t offset = 0u);

		explicit file_binary_writer(std::ofstream &stream);

		virtual ~file_binary_writer();

	protected:
		virtual std::size_t set_(std::byte value, std::size_t size) override;

		virtual std::size_t write_(const std::byte *buffer, std::size_t size) override;

		virtual std::size_t advance_(std::size_t size) override;

		void init_();

		std::ofstream my_stream_;
		std::ofstream &stream_;
	};
}
