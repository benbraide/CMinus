#pragma once

#include <string>
#include <iostream>

#include "io_reader.h"

namespace cminus::io{
	class stream_reader : public reader{
	public:
		explicit stream_reader(std::istream &stream);

		virtual ~stream_reader();

		virtual std::size_t get_offset() const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t get_bytes_remaining() const override;

		virtual std::size_t ignore(std::size_t size) const override;

		virtual std::size_t read(std::byte *buffer, std::size_t size) const override;

		virtual std::size_t read_scalar(const std::any &buffer) const override;

		using reader::read_scalar;

	protected:
		template <typename target_type>
		std::size_t read_(target_type &buffer) const{
			stream_ >> buffer;
			return sizeof(target_type);
		}

		std::istream &stream_;
	};

	class wide_stream_reader : public reader{
	public:
		explicit wide_stream_reader(std::wistream &stream);

		virtual ~wide_stream_reader();

		virtual std::size_t get_offset() const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t get_bytes_remaining() const override;

		virtual std::size_t ignore(std::size_t size) const override;

		virtual std::size_t read(std::byte *buffer, std::size_t size) const override;

		virtual std::size_t read_scalar(const std::any &buffer) const override;

		using reader::read_scalar;

	protected:
		template <typename target_type>
		std::size_t read_(target_type &buffer) const{
			stream_ >> buffer;
			return sizeof(target_type);
		}

		std::wistream &stream_;
	};
}
