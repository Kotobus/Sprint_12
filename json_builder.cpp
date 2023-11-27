#include "json_builder.h"
#include "json.h"
#include "log_duration.h"
namespace json {
	Node& json::Builder::Build()

	{
		if (nodes_stack_.size() == 1 && dicts == 0 && arrays == 0) {
			root_ = *nodes_stack_.back();
			return root_;
		}
		else {
			throw std::logic_error("Build Error Node is not complete");
		}
	}

	json::Builder::DictValueContext Builder::Key(std::string key)
	{
		if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
			throw std::logic_error("Key Error");
		}
		else {
			nodes_stack_.emplace_back(new Node(std::move(key)));
		}
		return DictValueContext{ *this };
	}

	json::Builder::BaseContext Builder::Value(Node val)
	{
		
			if (!NodeAddFlag()) {
				throw std::logic_error("Value Error");
			}
			else if (nodes_stack_.empty()) {
				nodes_stack_.emplace_back(new Node(std::move(val)));
				return BaseContext(*this);
			}
			else if (nodes_stack_.back()->IsArray()) {
				auto arr = nodes_stack_.back()->AsArray();
				arr.emplace_back(Node(val));
				nodes_stack_.pop_back();
				nodes_stack_.emplace_back(new Node(std::move(arr)));
				return ArrayContext(*this);
			}
			else if (nodes_stack_.back()->IsString()) {
				auto str = nodes_stack_.back();
				nodes_stack_.pop_back();
				if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
					throw std::logic_error("Dict error");
				}
				else {
					auto map_ = nodes_stack_.back()->AsMap();
					map_.emplace(str->AsString(), Node(std::move(val)));
					nodes_stack_.pop_back();
					nodes_stack_.emplace_back(new Node(std::move(map_)));
					return DictItemContext(*this);
				}
			}
		return json::Builder::BaseContext{*this};
	}

	json::Builder::DictItemContext Builder::StartDict()
	{

		if (NodeAddFlag()) {
			if (nodes_stack_.empty() || nodes_stack_.back()->IsArray()) {
				nodes_stack_.emplace_back(new Node(Dict()));
				dicts += 1;
			}
			else if (nodes_stack_.back()->IsString() && nodes_stack_.size() >= 2 && nodes_stack_[nodes_stack_.size() - 2]->IsMap()) {  //nodes_stack_[nodes_stack_.size() - 3] - проверка что элемент идущий перед строкой это словарь а не нода 

				nodes_stack_.emplace_back(new Node(Dict()));
				dicts += 1;
			}
			else {
				throw std::logic_error("Could not StartDict after Value");
			}
		}
		else {
			throw std::logic_error("Could not StartDict");
		}
		return DictItemContext{ *this };

	}

	json::Builder::ArrayContext Builder::StartArray()
	{
		if (NodeAddFlag()) {
			if (nodes_stack_.empty() || nodes_stack_.back()->IsArray()) {
				nodes_stack_.emplace_back(new Node(Array()));
				arrays += 1;
			}
			else if (nodes_stack_.back()->IsString() && nodes_stack_.size() >= 2 && nodes_stack_[nodes_stack_.size() - 2]->IsMap()) {  //nodes_stack_[nodes_stack_.size() - 3] - проверка что элемент идущий перед строкой это словарь а не нода 
				nodes_stack_.emplace_back(new Node(Array()));
				arrays += 1;
			}
			else {
				throw std::logic_error("Could not StartArray after Value");
			}
		}
		else {
			throw std::logic_error("Could not StartArray");
		}
		return ArrayContext{ *this };
	}

	json::Builder::BaseContext Builder::EndDict()
	{
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap()) {
			if (nodes_stack_.size() > 1) {
				auto dict = nodes_stack_.back();
				nodes_stack_.pop_back();
				if (nodes_stack_.back()->IsArray()) {
					auto arr = nodes_stack_.back()->AsArray();
					arr.emplace_back(dict->AsMap());
					nodes_stack_.pop_back();
					nodes_stack_.emplace_back(new Node(std::move(arr)));
					dicts--;
				}
				else if (nodes_stack_.back()->IsString()) {
					Node* str = nodes_stack_.back();
					nodes_stack_.pop_back();
					if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap()) {
						auto dict_parent = nodes_stack_.back()->AsMap();
						dict_parent.emplace(str->AsString(), dict->AsMap());
						nodes_stack_.pop_back();
						nodes_stack_.emplace_back(new Node(std::move(dict_parent)));
						dicts -= 1;
					}
					else {
						throw std::logic_error("EndDict error backpacking into Dict");
					}

				}

			}
			else if (nodes_stack_.back()->IsMap()) {
				dicts--;
			}

		}
		else throw std::logic_error("EndDict error stack empty or last element is not Dict");
		return BaseContext{ *this };

	}

	json::Builder::BaseContext Builder::EndArray()
	{
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			if (nodes_stack_.size() > 1) {
				auto arr = nodes_stack_.back();
				nodes_stack_.pop_back();
				if (nodes_stack_.back()->IsArray()) {
					auto arr2 = nodes_stack_.back()->AsArray();
					arr2.emplace_back(arr->AsArray());
					nodes_stack_.pop_back();
					nodes_stack_.emplace_back(new Node(std::move(arr2)));
					arrays -= 1;
				}
				else if (nodes_stack_.back()->IsString()) {
					auto str = nodes_stack_.back();
					nodes_stack_.pop_back();
					if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap()) {
						auto dict_parent = nodes_stack_.back()->AsMap();
						dict_parent.emplace(str->AsString(), std::move(arr->AsArray()));
						nodes_stack_.pop_back();
						nodes_stack_.emplace_back(new Node(std::move(dict_parent)));
						arrays -= 1;
					}
					else {
						throw std::logic_error("EndArray error backpacking into Dict");
					}

				}

			}
			else
			{
				arrays--;
			}

		}
		else throw std::logic_error("EndArray error stack empty or last element is not Array");
		return BaseContext{ *this };

	}



	Node& Builder::BaseContext::Build()
	{
		return builder_.Build();
	}

	Builder::DictValueContext Builder::BaseContext::Key(std::string key)
	{
		return builder_.Key(key);
	}

	Builder::BaseContext Builder::BaseContext::Value(Node val)
	{
		return builder_.Value(val);
	}

	Builder::DictItemContext Builder::BaseContext::StartDict()
	{
		return builder_.StartDict();
	}

	Builder::ArrayContext Builder::BaseContext::StartArray()
	{
		return builder_.StartArray();
	}

	Builder::BaseContext Builder::BaseContext::EndDict()
	{
		return builder_.EndDict();
	}

	Builder::BaseContext Builder::BaseContext::EndArray()
	{
		return builder_.EndArray();
	}

	Builder::DictItemContext Builder::DictValueContext::Value(Node val)
	{
		builder_.Value(std::move((val)));
		return DictItemContext(*this);
	}

	Builder::DictValueContext Builder::DictItemContext::Key(std::string key)
	{
		return builder_.Key(std::move(key));
	}
	Builder::ArrayContext Builder::ArrayContext::Value(Node val)
	{
		builder_.Value(std::move(val));
		return ArrayContext(*this);
	}
}//json