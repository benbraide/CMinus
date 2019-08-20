#pragma once

#include "naming.h"

namespace cminus::memory{
	class object;
	class reference;
}

namespace cminus::logic::type{
	class object{
	public:
		enum class score_result_type{
			nil					= 0,
			exact				= 100,
			gains_const			= 90,
			drops_ref			= 90,
			ancestor			= 60,
			compatible			= 20,
			class_compatible	= 10,
			widened				= 60,
			shortened			= 40,
			too_widened			= 50,
			too_shortened		= 30,
		};

		virtual ~object();

		virtual void print(logic::runtime &runtime, bool is_qualified) const = 0;

		virtual std::size_t get_size() const = 0;

		virtual score_result_type get_score(const object &target) const = 0;

		virtual std::shared_ptr<memory::reference> convert_value(logic::runtime &runtime, std::shared_ptr<memory::reference> data, std::shared_ptr<object> target_type) const = 0;

		virtual std::shared_ptr<memory::reference> convert_value(logic::runtime &runtime, const std::byte *data, std::shared_ptr<object> target_type) const = 0;
	};

	class named_object : public object, public naming::single{
	public:
		named_object(const std::string &value, naming::parent *parent);

		virtual ~named_object();

		virtual void print(logic::runtime &runtime, bool is_qualified) const override;

		virtual std::string get_qualified_naming_value() const override;
	};
}
