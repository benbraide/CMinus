#pragma once

#include "evaluator_object.h"

namespace cminus::evaluator{
	class arithmetic{
	public:
		using constant_type = node::named_constant::constant_type;

		virtual ~arithmetic() = default;;

	protected:
		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_unary_left_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> value) const{
			switch (op){
			case operator_id::minus:
				if (value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, value->get_type(), nullptr, static_cast<value_type>(-value->read_scalar<value_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(value->get_type(), nullptr, static_cast<value_type>(-value->read_scalar<value_type>(runtime)));
			default:
				break;
			}

			return evaluate_unsigned_unary_left_<value_type>(runtime, op, value);
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_integral_unary_left_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> value) const{
			switch (op){
			case operator_id::minus:
				if (value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, value->get_type(), nullptr, static_cast<value_type>(-value->read_scalar<value_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(value->get_type(), nullptr, static_cast<value_type>(-value->read_scalar<value_type>(runtime)));
			default:
				break;
			}

			return evaluate_unsigned_integral_unary_left_<value_type>(runtime, op, value);
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_unsigned_unary_left_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> value) const{
			switch (op){
			case operator_id::plus:
				if (value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, value->get_type(), nullptr, value->read_scalar<value_type>(runtime));
				return std::make_shared<memory::scalar_reference<value_type>>(value->get_type(), nullptr, value->read_scalar<value_type>(runtime));
			default:
				break;
			}

			return nullptr;
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_unsigned_integral_unary_left_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> value) const{
			switch (op){
			case operator_id::increment:
				value->write_scalar(runtime, static_cast<value_type>(value->read_scalar<value_type>(runtime) + static_cast<value_type>(1)));
				return value;
			case operator_id::decrement:
				value->write_scalar(runtime, static_cast<value_type>(value->read_scalar<value_type>(runtime) - static_cast<value_type>(1)));
				return value;
			default:
				break;
			}

			return evaluate_unsigned_unary_left_<value_type>(runtime, op, value);
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_unsigned_unary_right_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> value) const{
			value_type computed_value;
			switch (op){
			case operator_id::increment:
				computed_value = static_cast<value_type>(value->read_scalar<value_type>(runtime) + static_cast<value_type>(1));
				value->write_scalar(runtime, computed_value);

				if (value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, value->get_type(), nullptr, computed_value);

				return std::make_shared<memory::scalar_reference<value_type>>(value->get_type(), nullptr, computed_value);
			case operator_id::decrement:
				computed_value = static_cast<value_type>(value->read_scalar<value_type>(runtime) - static_cast<value_type>(1));
				value->write_scalar(runtime, computed_value);

				if (value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, value->get_type(), nullptr, computed_value);

				return std::make_shared<memory::scalar_reference<value_type>>(value->get_type(), nullptr, computed_value);
			default:
				break;
			}

			return nullptr;
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_binary_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			switch (op){
			case operator_id::plus:
				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) + right_value->read_scalar<value_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) + right_value->read_scalar<value_type>(runtime)));
			case operator_id::minus:
				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) - right_value->read_scalar<value_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) - right_value->read_scalar<value_type>(runtime)));
			case operator_id::times:
				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) * right_value->read_scalar<value_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) * right_value->read_scalar<value_type>(runtime)));
			default:
				break;
			}

			if (op == operator_id::divide){
				auto right_scalar_value = right_value->read_scalar<value_type>(runtime);
				if (right_scalar_value == static_cast<value_type>(0))//NaN
					return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);

				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) / right_value->read_scalar<value_type>(runtime)));

				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) / right_value->read_scalar<value_type>(runtime)));
			}

			return nullptr;
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_integral_binary_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			switch (op){
			case operator_id::bitwise_and:
				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) & right_value->read_scalar<value_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) & right_value->read_scalar<value_type>(runtime)));
			case operator_id::bitwise_or:
				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) | right_value->read_scalar<value_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) | right_value->read_scalar<value_type>(runtime)));
			case operator_id::bitwise_xor:
				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) ^ right_value->read_scalar<value_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) ^ right_value->read_scalar<value_type>(runtime)));
			default:
				break;
			}

			if (op == operator_id::modulus){
				auto right_scalar_value = right_value->read_scalar<value_type>(runtime);
				if (right_scalar_value == static_cast<value_type>(0))//NaN
					return runtime.global_storage->get_named_constant(node::named_constant::constant_type::nan_);

				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) % right_value->read_scalar<value_type>(runtime)));

				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) % right_value->read_scalar<value_type>(runtime)));
			}

			return evaluate_binary_<value_type>(runtime, op, left_value, right_value);
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> evaluate_integral_shift_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			auto right_type = right_value->get_type();
			if (auto right_primitive_type = dynamic_cast<type::primitive *>(right_type.get()); right_primitive_type != nullptr){
				switch (right_primitive_type->get_id()){
				case type::primitive::id_type::nan_:
				case type::primitive::id_type::int8_:
					return evaluate_integral_shift_<value_type, __int8>(runtime, op, left_value, right_value);
				case type::primitive::id_type::uint8_:
					return evaluate_integral_shift_<value_type, unsigned __int8>(runtime, op, left_value, right_value);
				case type::primitive::id_type::int16_:
					return evaluate_integral_shift_<value_type, __int16>(runtime, op, left_value, right_value);
				case type::primitive::id_type::uint16_:
					return evaluate_integral_shift_<value_type, unsigned __int16>(runtime, op, left_value, right_value);
				case type::primitive::id_type::int32_:
					return evaluate_integral_shift_<value_type, __int32>(runtime, op, left_value, right_value);
				case type::primitive::id_type::uint32_:
					return evaluate_integral_shift_<value_type, unsigned __int32>(runtime, op, left_value, right_value);
				case type::primitive::id_type::int64_:
					return evaluate_integral_shift_<value_type, __int64>(runtime, op, left_value, right_value);
				case type::primitive::id_type::uint64_:
					return evaluate_integral_shift_<value_type, unsigned __int64>(runtime, op, left_value, right_value);
				default:
					break;
				}
			}

			return nullptr;
		}

		template <typename value_type, typename shift_type>
		std::shared_ptr<memory::reference> evaluate_integral_shift_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			switch (op){
			case operator_id::left_shift:
				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) << right_value->read_scalar<shift_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) << right_value->read_scalar<shift_type>(runtime)));
			case operator_id::right_shift:
				if (left_value->is_nan() || right_value->is_nan())
					return std::make_shared<memory::nan_scalar_reference<value_type>>(runtime, left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) >> right_value->read_scalar<shift_type>(runtime)));
				return std::make_shared<memory::scalar_reference<value_type>>(left_value->get_type(), nullptr, static_cast<value_type>(left_value->read_scalar<value_type>(runtime) >> right_value->read_scalar<shift_type>(runtime)));
			default:
				break;
			}

			if (op == operator_id::index){//Get byte
				auto index = static_cast<std::size_t>(right_value->read_scalar<shift_type>(runtime));
				auto value = left_value->read_scalar<value_type>(runtime);

				if (sizeof(value_type) <= index || left_value->is_nan() || right_value->is_nan())//Out of bounds
					return std::make_shared<memory::scalar_reference<std::byte>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::byte_), nullptr, static_cast<std::byte>(0));

				return std::make_shared<memory::scalar_reference<std::byte>>(runtime.global_storage->get_primitve_type(type::primitive::id_type::byte_), nullptr, reinterpret_cast<std::byte *>(&value)[index]);
			}

			return nullptr;
		}

		template <typename value_type>
		std::shared_ptr<memory::reference> compare_(logic::runtime &runtime, operator_id op, std::shared_ptr<memory::reference> left_value, std::shared_ptr<memory::reference> right_value) const{
			constant_type value;
			switch (op){
			case operator_id::less:
				if (left_value->is_nan() || right_value->is_nan()){
					value = ((left_value->read_scalar<value_type>(runtime) < right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
					value = constant_type::false_;
				}
				else
					value = ((left_value->read_scalar<value_type>(runtime) < right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
				break;
			case operator_id::less_or_equal:
				if (left_value->is_nan() || right_value->is_nan()){
					value = ((left_value->read_scalar<value_type>(runtime) <= right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
					value = ((left_value->is_nan() && right_value->is_nan()) ? constant_type::true_ : constant_type::false_);
				}
				else
					value = ((left_value->read_scalar<value_type>(runtime) <= right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
				break;
			case operator_id::equality:
				if (left_value->is_nan() || right_value->is_nan()){
					value = ((left_value->read_scalar<value_type>(runtime) == right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
					value = ((left_value->is_nan() && right_value->is_nan()) ? constant_type::true_ : constant_type::false_);
				}
				else
					value = ((left_value->read_scalar<value_type>(runtime) == right_value->read_scalar<value_type>(runtime)) ? constant_type::true_ : constant_type::false_);
				break;
			case operator_id::inverse_equality:
				if (left_value->is_nan() || right_value->is_nan()){
					value = ((left_value->read_scalar<value_type>(runtime) == right_value->read_scalar<value_type>(runtime)) ? constant_type::false_ : constant_type::true_);
					value = ((left_value->is_nan() && right_value->is_nan()) ? constant_type::false_ : constant_type::true_);
				}
				else
					value = ((left_value->read_scalar<value_type>(runtime) == right_value->read_scalar<value_type>(runtime)) ? constant_type::false_ : constant_type::true_);
				break;
			case operator_id::more_or_equal:
				if (left_value->is_nan() || right_value->is_nan()){
					value = ((left_value->read_scalar<value_type>(runtime) < right_value->read_scalar<value_type>(runtime)) ? constant_type::false_ : constant_type::true_);
					value = ((left_value->is_nan() && right_value->is_nan()) ? constant_type::true_ : constant_type::false_);
				}
				else
					value = ((left_value->read_scalar<value_type>(runtime) < right_value->read_scalar<value_type>(runtime)) ? constant_type::false_ : constant_type::true_);
				break;
			case operator_id::more:
				if (left_value->is_nan() || right_value->is_nan()){
					value = ((left_value->read_scalar<value_type>(runtime) <= right_value->read_scalar<value_type>(runtime)) ? constant_type::false_ : constant_type::true_);
					value = constant_type::false_;
				}
				else
					value = ((left_value->read_scalar<value_type>(runtime) <= right_value->read_scalar<value_type>(runtime)) ? constant_type::false_ : constant_type::true_);
				break;
			default:
				value = constant_type::nil;
				break;
			}

			return ((value == constant_type::nil) ? nullptr : runtime.global_storage->get_named_constant(value));
		}
	};
}
