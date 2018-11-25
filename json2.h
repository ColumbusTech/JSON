#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <iterator>
#include <algorithm>
#include <cctype>
#include <cmath>

enum class Error
{
	None,
	InvalidString,
	InvalidNumber,
	MissedColon,
	MissedComma,
	MissedQuot,
	MissedBracket,
	MissedBrace,
	Undefined
};

class Node
{
private:
	enum class ValueType
	{
		Variant,
		Array,
		Object
	};

	std::variant<std::string, int, float, bool, std::nullptr_t> Variant;
	std::vector<Node> Array;
	std::map<std::string, Node> Object;

	ValueType Type;
private:
	void Save(std::ostream& Stream, const std::string& V) const { Stream << '"' << V << '"'; }
	void Save(std::ostream& Stream, int V) const { Stream << V; }
	void Save(std::ostream& Stream, float V) const { Stream << V; }
	void Save(std::ostream& Stream, bool V) const { Stream << (V ? "true" : "false"); }
	void Save(std::ostream& Stream, std::nullptr_t V) const { Stream << "null"; }

	Error Parse(const char** Position)
	{
		auto SkipSpace = [&]() { while (std::isspace(**Position)) (*Position)++; };
		auto ExtractString = [&]() { std::string S; while (**Position != '"') { S += **Position; (*Position)++; }; return S; };
		auto ExtractInt = [&]() { int N = 0; while (std::isdigit(**Position)) N = N * 10 + (*(*Position)++ - '0'); return N; };
		auto ExtractDec = [&]() { double N = 0.0, F = 0.1; while (std::isdigit(**Position)) { N = N + (*(*Position)++ - '0') * F; F *= 0.1; } return N; };
		
		SkipSpace();

		if (**Position == '"')
		{
			(*Position)++;
			Variant = ExtractString();
			Type = ValueType::Variant;
			return Error::None;
		}

		if (std::equal(*Position, *Position + 4, "true") || std::equal(*Position, *Position + 5, "false"))
		{
			Variant = std::equal(*Position, *Position + 4, "true");
			*Position += std::equal(*Position, *Position + 4, "true") ? 4 : 5;
			Type = ValueType::Variant;
			return Error::None;
		}

		if (std::equal(*Position, *Position + 4, "null"))
		{
			Variant = nullptr;
			*Position += 4;
			Type = ValueType::Variant;
			return Error::None;
		}

		if (**Position == '-' || (**Position >= '0' && **Position <= '9'))
		{
			bool Neg = **Position == '-';
			if (Neg) (*Position)++;

			double Number = (int)ExtractInt();

			if (**Position == '.')
			{
				(*Position)++;
				
				if (**Position < '0' || **Position > '9')
				{
					return Error::InvalidNumber;
				}

				Number += ExtractDec();				
			}

			if (**Position == 'e' || **Position == 'E')
			{
				(*Position)++;
				bool ExpNeg = **Position == '-';
				if (ExpNeg) (*Position)++;

				if (**Position < '0' || **Position > '9')
				{
					return Error::InvalidNumber;
				}

				int Exponent = ExtractInt();
				Exponent *= ExpNeg ? -1 : 1;
				Number *= pow(10, Exponent);
			}

			Number *= Neg ? -1 : 1;
			
			double tmp;
			if (modf(Number, &tmp) == 0)
			{
				Type = ValueType::Variant;
				Variant = (int)Number;
			}
			else
			{
				Type = ValueType::Variant;
				Variant = (float)Number;
			}

			return Error::None;
		}

		if (**Position == '{')
		{
			(*Position)++;
			SkipSpace();
			if (**Position == '}') { Type = ValueType::Object; return Error::None; }
		}

		return Error::None;
	}
public:
	Node() : Type(ValueType::Object) {}

	template <typename T>
	Node& operator=(T Val)
	{
		Variant = Val;
		Type = ValueType::Variant;
		return *this;
	}

	template <typename T>
	const T& Get() const
	{
		return std::get<T>(Variant);
	}

	Node& operator[](int Index)
	{
		if (Type != ValueType::Array)
		{
			Object.clear();
			Type = ValueType::Array;
		}

		if (Index >= Array.size()) Array.emplace_back();

		return Array[Index];
	}

	Node& operator[](const char* Key)
	{
		if (Type != ValueType::Object)
		{
			Array.clear();
			Type = ValueType::Object;
		}

		return Object[Key];
	}

	Node& operator[](const std::string& Key)
	{
		if (Type != ValueType::Object)
		{
			Array.clear();
			Type = ValueType::Object;
		}

		return Object[Key];
	}

	friend std::istream& operator>>(std::istream& Stream, Node& Val)
	{
		std::string Str = std::string(std::istreambuf_iterator<char>(Stream), {});
		const char* S = Str.c_str();
		auto E = Val.Parse(&S);

		if (E != Error::None) throw E;

		return Stream;
	}
	
	friend std::ostream& operator<<(std::ostream& Stream, const Node& Val)
	{
		static int Level = 0;
		auto Tabs = [&]() { for (int i = 0; i < Level; i++) Stream << '\t'; };

		if (Val.Type == Node::ValueType::Object)
		{
			if (Level != 0) Stream << std::endl;
			Tabs();
			Stream << '{' << std::endl;
			Level++;

			int Count = 0;

			for (auto& N : Val.Object)
			{
				Tabs();

				Stream << '"' << N.first << "\": " << N.second <<
					(++Count != Val.Object.size() ? "," : "") << std::endl;
			}

			Level--;
			Tabs();
			Stream << '}';
		}
		else if (Val.Type == Node::ValueType::Variant)
		{
			std::visit([&](const auto& V) { Val.Save(Stream, V); }, Val.Variant);
		}
		else if (Val.Type == Node::ValueType::Array)
		{
			Stream << '[';

			int Counter = 0;

			for (auto& E : Val.Array)
			{
				Stream << E << (++Counter == Val.Array.size() ? "" : ", ");
			}

			Stream << ']';
		}

		Stream << std::endl;
		return Stream;
	}
};

class JSON
{
private:
	Node Root;
public:
	Node& operator[](const char* Key)
	{
		return Root[Key];
	}

	friend std::ostream& operator<<(std::ostream& Stream, const JSON& J)
	{
		Stream << J.Root;
		return Stream;
	}

	friend std::istream& operator>>(std::istream& Stream, JSON& J)
	{
		Stream >> J.Root;
		return Stream;
	}
};


