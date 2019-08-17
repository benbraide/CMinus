#pragma once

#include <string>
#include <iostream>
#include <iterator>

#include "io_writer.h"

namespace cminus::io{
	class stream_writer : public writer{
	public:
		using backend_type = std::ostream;
		using char_type = char;

		explicit stream_writer(std::ostream &stream);

		virtual ~stream_writer();

		virtual std::size_t get_offset() const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t set(std::byte value, std::size_t size) override;

		virtual std::size_t write(const std::byte *buffer, std::size_t size) override;

		virtual std::size_t write_scalar(const std::any &buffer) override;

		using writer::write_scalar;

		virtual std::size_t fill(char_type value, std::size_t count);

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
		using backend_type = std::wostream;
		using char_type = wchar_t;

		explicit wide_stream_writer(std::wostream &stream);

		virtual ~wide_stream_writer();

		virtual std::size_t get_offset() const override;

		virtual std::size_t get_size() const override;

		virtual std::size_t set(std::byte value, std::size_t size) override;

		virtual std::size_t write(const std::byte *buffer, std::size_t size) override;

		virtual std::size_t write_scalar(const std::any &buffer) override;

		using writer::write_scalar;

		virtual std::size_t fill(char_type value, std::size_t count);

	protected:
		template <typename target_type>
		std::size_t write_(const target_type &buffer) const{
			stream_ << buffer;
			return sizeof(target_type);
		}

		std::wostream &stream_;
	};

	class tabbed_stream_writer_base{
	public:
		virtual ~tabbed_stream_writer_base() = default;

		virtual void write_tabs() = 0;

		virtual void set_tab_size(std::size_t value) = 0;

		virtual void tab() = 0;

		virtual void untab() = 0;

		virtual std::size_t get_tab_size() const = 0;

		virtual std::size_t get_tab_count() const = 0;
	};

	template <class stream_writer_type>
	class tabbed_stream_writer : public stream_writer_type, public tabbed_stream_writer_base{
	public:
		using m_stream_writer_type = stream_writer_type;
		using backend_type = typename stream_writer_type::backend_type;
		using char_type = typename stream_writer_type::char_type;

		explicit tabbed_stream_writer(backend_type &stream)
			: stream_writer_type(stream){}

		virtual ~tabbed_stream_writer() = default;

		virtual void write_tabs() override{
			if (0u < (tab_size_ * tab_count_))
				stream_writer_type::fill(fill_char_, (tab_size_ * tab_count_));
		}

		virtual void set_tab_size(std::size_t value) override{
			tab_size_ = value;
		}

		virtual void tab() override{
			++tab_count_;
		}

		virtual void untab() override{
			if (0u < tab_count_)
				--tab_count_;
		}

		virtual std::size_t get_tab_size() const override{
			return tab_size_;
		}

		virtual std::size_t get_tab_count() const override{
			return tab_count_;
		}

		virtual void set_fill_char(char_type value){
			fill_char_ = value;
		}

		virtual char_type get_fill_char() const{
			return fill_char_;
		}

	protected:
		char_type fill_char_ = static_cast<char_type>(' ');
		std::size_t tab_size_ = 2u;
		std::size_t tab_count_ = 0u;
	};
}
