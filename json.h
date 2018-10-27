#include <string>
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
	Value() {}
	Value(const std::basic_string<char>& Str) : StringValue(Str), BoolValue(false), IntValue(0), FloatValue(0.0f) {}

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

	Value& operator[](const std::basic_string<char>& Key)
	{
		return Values[Key];
	}

	Value& operator[](int Index)
	{
		return Array[Index];
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










