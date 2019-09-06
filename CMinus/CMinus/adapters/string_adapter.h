#pragma once

#include "../type/string_type.h"

namespace cminus::adapter{
	class string{
	public:
		struct call_info{
			std::string name;
			std::shared_ptr<cminus::memory::reference> context;
			std::vector<std::shared_ptr<cminus::memory::reference>> args;
		};

		string(cminus::logic::runtime &runtime, const std::string &value);

		string(cminus::logic::runtime &runtime, std::shared_ptr<cminus::memory::reference> value);

		const char *data() const;

		std::size_t size() const;

		bool empty() const;

		char at(std::size_t position) const;

		void clear();

		std::shared_ptr<cminus::memory::reference> get_value() const;

	private:
		static std::shared_ptr<cminus::memory::reference> call_(cminus::logic::runtime &runtime, const call_info &info);

		cminus::logic::runtime *runtime_;
		std::shared_ptr<cminus::memory::reference> value_;
	};
}
