#pragma once

#include <string>
#include <iostream>
#include <iterator>

#include "io_writer.h"

namespace cminus::io{
	class stream_writer : public writer{
	public:
		explicit stream_writer(std::ostream &stream);

		virtual ~stream_writer();

		virtual std::size_t get_offset() const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t set(std::byte value, std::size_t size) override;

		virtual std::size_t write(const std::byte *buffer, std::size_t size) override;

		virtual std::size_t write_scalar(const std::any &buffer) override;

		using writer::write_scalar;

	protected:
		template <typename target_type>
		std::size_t write_(target_type &buffer) const{
			stream_ << buffer;
			return sizeof(target_type);
		}

		std::ostream &stream_;
	};

	class wide_stream_writer : public writer{
	public:
		explicit wide_stream_writer(std::wostream &stream);

		virtual ~wide_stream_writer();

		virtual std::size_t get_offset() const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t set(std::byte value, std::size_t size) override;

		virtual std::size_t write(const std::byte *buffer, std::size_t size) override;

		virtual std::size_t write_scalar(const std::any &buffer) override;

		using writer::write_scalar;

	protected:
		template <typename target_type>
		std::size_t write_(const target_type &buffer) const{
			stream_ << buffer;
			return sizeof(target_type);
		}

		std::wostream &stream_;
	};
}
