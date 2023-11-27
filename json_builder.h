#pragma once
#include "json.h"
#include <iostream>
#include <map>
#include <set>
#include <string>
namespace json {



	class Json { };

	class Builder {
		class ArrayContext;
		class DictValueContext;
		class DictItemContext;
		class BaseContext;
	public:
		Builder() = default;
		Node& Build();
		DictValueContext Key(std::string key);
		BaseContext Value(Node val);
		DictItemContext StartDict();
		ArrayContext StartArray();
		BaseContext EndDict();
		BaseContext EndArray();

	private: //
		bool NodeAddFlag() {
			return nodes_stack_.empty() || nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsString();
		};
		Node root_;
		std::vector<Node*> nodes_stack_;
		size_t dicts = 0;
		size_t arrays = 0;
	
	private: //methods
		//Json* json;
		class BaseContext {

		public:
			BaseContext(Builder& builder) :builder_(builder) {};
			Node& Build();
			DictValueContext Key(std::string key);
			BaseContext Value(Node val);
			DictItemContext StartDict();
			ArrayContext StartArray();
			BaseContext EndDict();
			BaseContext EndArray();
		protected:
			Builder& builder_;
		};
	
	class ArrayContext :public BaseContext {
	public:
		ArrayContext(BaseContext base) :BaseContext(base) {};
		Node& Build() = delete;
		DictValueContext Key(std::string key) = delete;
		BaseContext EndDict() = delete;
		ArrayContext Value(Node val);
	};

	class DictItemContext :public BaseContext {
	public:
		DictItemContext(BaseContext base) :BaseContext(base) {};
		Node& Build() = delete;
		DictValueContext Key(std::string key);
		DictValueContext Value(Node val) = delete;
		DictItemContext StartDict() = delete;
		ArrayContext StartArray() = delete;
		BaseContext EndArray() = delete;
	};
	
	class DictValueContext :public BaseContext {
	public:
		DictValueContext(BaseContext base) : BaseContext(base) {};
		Node& Build() = delete;
		DictValueContext Key(std::string key) = delete;
		BaseContext EndDict() = delete;
		BaseContext EndArray() = delete;
		DictItemContext Value(Node val);
	};
	
	};
}//json