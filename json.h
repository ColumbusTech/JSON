#pragma once

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cmath>

namespace
{
	bool SkipWhitespace(const char** String)
	{
		while (**String != 0 && (**String == ' ' || **String == '\n' || **String == '\r' || **String == '\t'))
		{
			(*String)++;
		}

		return **String != 0;
	}

	void ExtractString(const char** String, std::basic_string<char>& Out)
	{
		while (**String != '"')
		{
			Out += **String;
			(*String)++;
		}
	}

	double ParseInt(const char** String)
	{
		double Number = 0.0;

		while (**String != 0 && **String >= '0' && **String <= '9')
		{
			Number = Number * 10 + (*(*String)++ - '0');
		}

		return Number;
	}

	double ParseDecimal(const char** String)
	{
		double Number = 0.0;
		double Factor = 0.1;

		while (**String != 0 && **String >= '0' && **String <= '9')
		{
			int Digit = (*(*String)++ - '0');
			Number = Number + Digit * Factor;
			Factor *= 0.1;
		}

		return Number;
	}
}

namespace ColumbusJSON
{
	class Value
	{
	public:
		enum class Type
		{
			String,
			Bool,
			Null,
			Int,
			Float,
			Object,
			Array
		};
	private:
		std::basic_string<char> StringValue;
		bool BoolValue;
		int IntValue;
		float FloatValue;

		std::map<std::basic_string<char>, Value> Values;
		std::vector<Value> Array;

		Type ValueType;
	public:
		typedef std::map<std::basic_string<char>, Value>::const_iterator ValueConstIterator;
		typedef std::vector<Value>::const_iterator ArrayConstIterator;
	public:
		Value() : ValueType(Type::Object) {}
		Value(const std::basic_string<char>& Str) : StringValue(Str), BoolValue(false), IntValue(0), FloatValue(0.0f), ValueType(Type::String) {}
		Value(bool Bool) : StringValue(""), BoolValue(Bool), IntValue(0), FloatValue(0.0f), ValueType(Type::Bool) {}
		Value(std::nullptr_t Null) : StringValue(""), BoolValue(false), IntValue(0), FloatValue(0.0f), ValueType(Type::Null) {}
		Value(int Int) : StringValue(""), BoolValue(false), IntValue(Int), FloatValue(0.0f), ValueType(Type::Int) {}
		Value(float Float) : StringValue(""), BoolValue(false), IntValue(0), FloatValue(Float), ValueType(Type::Float) {}

		Value& operator=(char Ch)
		{
			StringValue = Ch;
			ValueType = Type::String;
			return *this;
		}

		Value& operator=(const char* Str)
		{
			StringValue = Str;
			ValueType = Type::String;
			return *this;
		}

		Value& operator=(const std::basic_string<char>& Str)
		{
			StringValue = Str;
			ValueType = Type::String;
			return *this;
		}

		Value& operator=(bool Bool)
		{
			BoolValue = Bool;
			ValueType = Type::Bool;
			return *this;
		}

		Value& operator=(std::nullptr_t Null)
		{
			ValueType = Type::Null;
			return *this;
		}

		Value& operator=(int Int)
		{
			IntValue = Int;
			ValueType = Type::Int;
			return *this;
		}

		Value& operator=(float Float)
		{
			FloatValue = Float;
			ValueType = Type::Float;
			return *this;
		}

		bool Parse(const char** Str)
		{
			//Is a string
			if (**Str == '"')
			{
				(*Str)++;
				ExtractString(Str, StringValue);
				if (**Str != '"') return false;
				(*Str)++;
				ValueType = Type::String;
				return true;
			}

			//Is a bool
			if (memcmp(*Str, "true", 4) == 0 || memcmp(*Str, "false", 5) == true)
			{
				BoolValue = memcmp(*Str, "true", 4) == 0;
				(*Str) += BoolValue ? 4 : 5;
				ValueType = Type::Bool;
				return true;
			}

			//Is a null
			if (memcmp(*Str, "null", 4) == 0)
			{
				(*Str) += 4;
				ValueType = Type::Null;
				return true;
			}

			//Is a number
			if (**Str == '-' || (**Str >= '0' && **Str <= '9'))
			{
				bool Negative = **Str == '-';
				if (Negative) (*Str)++;

				double Number = ParseInt(Str);

				//Decimal
				if (**Str == '.')
				{
					(*Str)++;

					if (!(**Str >= '0' && **Str <= '9'))
					{
						return false;
					}

					Number += ParseDecimal(Str);
				}

				//Exponent
				if (**Str == 'e' || **Str == 'E')
				{
					(*Str)++;

					bool Negative = **Str == '-';
					if (Negative) (*Str)++;

					if (!(**Str >= '0' && **Str <= '9'))
					{
						return false;
					}

					int Exponent = ParseInt(Str);
					Exponent *= Negative ? -1 : 1;
					Number *= pow(10.0, Exponent);
				}

				Number *= Negative ? -1 : 1;

				double tmp;
				if (modf(Number, &tmp) == 0)
				{
					ValueType = Type::Int;
					IntValue = Number;
					return true;
				}
				else
				{
					ValueType = Type::Float;
					FloatValue = Number;
					return true;
				}

				return true;
			}

			//Is an array
			if (**Str == '[')
			{
				(*Str)++;

				while (**Str != 0)
				{
					if (!SkipWhitespace(Str)) return false;

					//An empty array
					if (Array.size() == 0 && **Str == ']')
					{
						(*Str)++;
						return true;
					}

					Value Val;
					if (!Val.Parse(Str))
					{
						Array.clear();
						return false;
					}

					Array.push_back(Val);

					if (!SkipWhitespace(Str))
					{
						Array.clear();
						return false;
					}

					//End of the array?
					if (**Str == ']')
					{
						(*Str)++;
						return true;
					}

					if (**Str != ',')
					{
						Array.clear();
						return false;
					}

					(*Str)++;
				}
			}

			//Is an object
			if (**Str == '{')
			{
				(*Str)++;

				while (**Str != 0)
				{
					if (!SkipWhitespace(Str))
					{
						Values.clear();
						return false;
					}

					//Empty object
					if (**Str == '}')
					{
						(*Str)++;
						ValueType = Type::Object;
						return true;
					}

					if (!SkipWhitespace(Str))
					{
						Values.clear();
						return false;
					}

					std::basic_string<char> Name;

					if (**Str == '"')
					{
						(*Str)++;
						ExtractString(Str, Name);
						if (**Str != '"')
						{
							Values.clear();
							return false;
						}

						(*Str)++;
						if (!SkipWhitespace(Str)) { Values.clear(); return false; }

						if (**Str == ':')
						{
							(*Str)++;
							Value Val;
							if (!SkipWhitespace(Str)) { Values.clear(); return false; }
							if (!Val.Parse(Str))      { Values.clear(); return false; }
							Values[Name] = Val;
						}
					}

					if (!SkipWhitespace(Str))
					{
						Values.clear();
						return false;
					}

					if (**Str == '}')
					{
						(*Str++);
						return true;
					}

					if (**Str != ',')
					{
						Values.clear();
						return false;
					}

					(*Str)++;
				}
			}

			return false;
		}

		void SetString(const std::basic_string<char>& Str)
		{
			StringValue = Str;
		}

		Type GetType() const
		{
			return ValueType;
		}

		bool IsString() const
		{
			return ValueType == Type::String;
		}

		bool IsBool() const
		{
			return ValueType == Type::Bool;
		}

		bool IsNull() const
		{
			return ValueType == Type::Null;
		}

		bool IsInt() const
		{
			return ValueType == Type::Int;
		}

		bool IsFloat() const
		{
			return ValueType == Type::Float;
		}

		bool IsObject() const
		{
			return ValueType == Type::Object;
		}

		bool IsArray() const
		{
			return ValueType == Type::Array;
		}

		const char* GetString() const
		{
			return StringValue.c_str();
		}

		bool GetBool() const
		{
			return BoolValue;
		}

		int GetInt() const
		{
			return IntValue;
		}

		float GetFloat() const
		{
			return FloatValue;
		}

		bool HasValue(const std::basic_string<char>& Key) const
		{
			return Values.find(Key) != Values.end();
		}

		int ChildrenCount() const
		{
			return Values.size();
		}

		int ArraySize() const
		{
			return Array.size();
		}

		ValueConstIterator ValueBegin() const
		{
			return Values.begin();
		}

		ValueConstIterator ValueEnd() const
		{
			return Values.end();
		}

		ArrayConstIterator ArrayBegin() const
		{
			return Array.begin();
		}

		ArrayConstIterator ArrayEnd() const
		{
			return Array.end();
		}

		Value& operator[](const std::basic_string<char>& Key)
		{
			ValueType = Type::Object;
			Array.clear();
			StringValue.clear();

			return Values[Key];
		}

		Value& operator[](int Index)
		{
			ValueType = Type::Array;
			Values.clear();
			StringValue.clear();

			if (Index >= Array.size())
			{
				Array.push_back({});
			}

			return Array[Index];
		}

		friend std::ostream& operator<<(std::ostream& Stream, const Value& Val)
		{
			static int Iteration = 0;

			auto Tabs = [&]()->void { for(int i = 0; i < Iteration; i++) Stream << '\t'; };

			std::string Str;

			switch (Val.GetType())
			{
				case Value::Type::String: Str = std::string("\"") + Val.GetString() + std::string("\""); break;
				case Value::Type::Bool:   Str = Val.GetBool() ? "true" : "false"; break;
				case Value::Type::Null:   Str = "null"; break;
				case Value::Type::Int:    Str = std::to_string(Val.GetInt()); break;
				case Value::Type::Float:  Str = std::to_string(Val.GetFloat()); break;
			}

			if (Val.GetType() == Value::Type::Object)
			{
				Tabs();
				Stream << '{' << std::endl;

				Iteration++;
				for (auto it = Val.ValueBegin(); it != Val.ValueEnd(); ++it)
				{
					Tabs();

					if (it->second.GetType() != Value::Type::Object)
					{
						Stream << '"' << it->first << '"' << ": " << it->second;
					}
					else
					{
						Stream << '"' << it->first << '"' << ':' << std::endl << it->second;
					}

					if (++it != Val.ValueEnd()) Stream << ','; --it;
					Stream << std::endl;
				}
				Iteration--;

				Tabs();
				Stream << '}';
			}
			else if (Val.GetType() == Value::Type::Array)
			{
				Stream << '[';

				Iteration++;
				for (auto it = Val.ArrayBegin(); it != Val.ArrayEnd(); ++it)
				{
					if (it->GetType() == Value::Type::Object) Stream << std::endl;
					Stream << *it;
					if (++it != Val.ArrayEnd()) Stream << ", "; --it;
				}
				Iteration--;
				
				Stream << ']';
			}
			else
			{
				Stream << Str;
			}

			return Stream;
		}

		~Value() {}
	};

	class JSON
	{
	private:
		Value Root;
	public:
		bool Parse(const char* String)
		{
			if (!SkipWhitespace(&String)) return false;
			if (!Root.Parse(&String)) return false;

			return true;
		}

		bool Save(const char* Filename)
		{
			std::ofstream ofs(Filename);
			ofs << Root << std::endl;
			return true;
		}

		int ChildrenCount() const
		{
			return Root.ChildrenCount();
		}

		Value& operator[](const std::basic_string<char>& Key)
		{
			return Root[Key];
		}

		Value& operator[](int Index)
		{
			return Root[Index];
		}
	};
}










