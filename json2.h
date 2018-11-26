#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <iterator>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace columbus_json
{
	enum class Error;
	class Node;
	class JSON;

	typedef std::vector<Node> Array_t;
	typedef std::map<std::string, Node> Object_t;

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
		Array_t  Array;
		Object_t Object;

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

				if (**Position != '"') return Error::MissedQuot;
				(*Position)++;

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

				while (**Position != 0)
				{
					SkipSpace();
					if (**Position == '}') { Type = ValueType::Object; return Error::None; }

					if (**Position == '"')
					{
						(*Position)++;
						std::string Name = ExtractString();

						if (**Position != '"') { Object.clear(); return Error::MissedQuot; }
						(*Position)++;
						SkipSpace();

						if (**Position != ':') { Object.clear(); return Error::MissedColon; }
						(*Position)++;
						SkipSpace();

						Node New;
						Error Err = New.Parse(Position);
						if (Err != Error::None) { Object.clear(); return Err; }

						Type = ValueType::Object;
						Object[Name] = New;

						SkipSpace();

						if (**Position == '}') { (*Position)++;  return Error::None; }
						if (**Position != ',') { Object.clear(); return Error::MissedComma; }
						(*Position)++;
					}
				}
			}

			if (**Position == '[')
			{
				(*Position)++;

				while (**Position != 0)
				{
					SkipSpace();
					if (Array.size() == 0 && **Position == ']') { Type == ValueType::Array; return Error::None; }

					Node New;
					Error Err = New.Parse(Position);
					if (Err != Error::None) {Array.clear(); return Err; }

					Type = ValueType::Array;
					Array.push_back(New);

					SkipSpace();

					if (**Position == ']') { (*Position)++; return Error::None; }
					if (**Position != ',') { Array.clear(); return Error::MissedComma;}
					(*Position)++;
				}
			}

			return Error::None;
		}
	public:
		Node() : Type(ValueType::Object) {}
		Node(std::string    Val) : Variant(Val), Type(ValueType::Variant) {}
		Node(int            Val) : Variant(Val), Type(ValueType::Variant) {}
		Node(float          Val) : Variant(Val), Type(ValueType::Variant) {}
		Node(bool           Val) : Variant(Val), Type(ValueType::Variant) {}
		Node(std::nullptr_t Val) : Variant(Val), Type(ValueType::Variant) {}
		Node(const Array_t& Arr) : Array(Arr),   Type(ValueType::Array)   {}
		Node(const std::initializer_list<Node>& Arr) :
		     Array(Arr.begin(), Arr.end()), Type(ValueType::Array) {}

		Node& operator=(const char* Val)
		{
			Variant = (std::string)Val;
			Type = ValueType::Variant;
			return *this;
		}

		Node& operator=(const std::string& Val)
		{
			Variant = Val;
			Type = ValueType::Variant;
			return *this;
		}

		Node& operator=(int Val)
		{
			Variant = Val;
			Type = ValueType::Variant;
			return *this;
		}

		Node& operator=(double Val)
		{
			Variant = (float)Val;
			Type = ValueType::Variant;
			return *this;
		}

		Node& operator=(bool Val)
		{
			Variant = Val;
			Type = ValueType::Variant;
			return *this;
		}

		Node& operator=(std::nullptr_t Val)
		{
			Variant = Val;
			Type = ValueType::Variant;
			return *this;
		}

		Node& operator=(const Array_t& Arr)
		{
			Array = Arr;
			Type = ValueType::Array;
			return *this;
		}

		Node& operator=(const std::initializer_list<Node>& Arr)
		{
			Array.clear();
			Array.assign(Arr.begin(), Arr.end());
			Type = ValueType::Array;
			return *this;
		}

		template <typename T>
		const T& Get() const;

		template <typename T>
		bool Is() const;

		template <typename T>
		operator const T&() const
		{
			return Get<T>();
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

			if (Level == 0) Stream << std::endl;
			return Stream;
		}
	};

	template <> const std::string&    Node::Get() const { return std::get<std::string>   (Variant); }
	template <> const int&            Node::Get() const { return std::get<int>           (Variant); }
	template <> const float&          Node::Get() const { return std::get<float>         (Variant); }
	template <> const bool&           Node::Get() const { return std::get<bool>          (Variant); }
	template <> const std::nullptr_t& Node::Get() const { return std::get<std::nullptr_t>(Variant); }
	template <> const Object_t&       Node::Get() const { return Object; }
	template <> const Array_t&        Node::Get() const { return Array;  }

	template <> bool Node::Is<std::string>()    const { return Type == ValueType::Variant && std::holds_alternative<std::string>   (Variant); }
	template <> bool Node::Is<int>()            const { return Type == ValueType::Variant && std::holds_alternative<int>           (Variant); }
	template <> bool Node::Is<float>()          const { return Type == ValueType::Variant && std::holds_alternative<float>         (Variant); }
	template <> bool Node::Is<bool>()           const { return Type == ValueType::Variant && std::holds_alternative<bool>          (Variant); }
	template <> bool Node::Is<std::nullptr_t>() const { return Type == ValueType::Variant && std::holds_alternative<std::nullptr_t>(Variant); }
	template <> bool Node::Is<Object_t>()       const { return Type == ValueType::Object; }
	template <> bool Node::Is<Array_t> ()       const { return Type == ValueType::Array;  }

	class JSON
	{
	private:
		Node Root;
	public:
		JSON() = default;
		JSON(const JSON&) = default;
		JSON(JSON&&) = default;
		JSON(std::istream& Stream)
		{
			Stream >> Root;
		}

		template <typename T, typename = typename std::enable_if<
		          std::is_same<typename std::decay<T>::type, std::string>::value ||
		          std::is_same<typename std::decay<T>::type, char*>::value>::type>
		Node& operator[](const T& Key)
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
}


