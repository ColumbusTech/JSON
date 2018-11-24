/*
* Part of JSON, a library for JSON parsing.
* https://github.com/ColumbusUtrigas/JSON
*
* Copyright (c) 2018 ColumbusUtrigas.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <algorithm>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <initializer_list>
#include <cstring>
#include <cstdint>
#include <cmath>

/*!
* @file json.h
* @brief All functional of library.
*/

static bool SkipWhitespace(const char** String)
{
	while (std::isspace(**String))
	{
		(*String)++;
	}

	return **String != 0;
}

static void ExtractString(const char** String, std::basic_string<char>& Out)
{
	while (**String != '"')
	{
		Out += **String;
		(*String)++;
	}
}

static double ParseInt(const char** String, bool& Err)
{
	double Number = 0.0;

	Err = false;

	while (**String != 0 && std::isdigit(**String))
	{
		Number = Number * 10 + (*(*String)++ - '0');

		if (!std::isdigit(**String) && !std::isspace(**String) && **String != '.' && **String != ',' && **String != 'e' && **String != 'E')
		{
			Err= true;
			return 0.0;
		}
	}

	return Number;
}

static double ParseDecimal(const char** String, bool& Err)
{
	double Number = 0.0;
	double Factor = 0.1;

	Err = false;

	while (**String != 0 && std::isdigit(**String))
	{
		int Digit = (*(*String)++ - '0');
		Number = Number + Digit * Factor;
		Factor *= 0.1;

		if (!std::isdigit(**String) && !std::isspace(**String) && **String != '.' && **String != ',' && **String != 'e' && **String != 'E')
		{
			Err= true;
			return 0.0;
		}
	}

	return Number;
}

namespace ColumbusJSON
{
	/**
	* @brief JSON parsing error code.
	*/
	enum class Error
	{
		None,
		NoFile,
		EmptyFile,
		InvalidString, 
		InvalidNumber,
		MissedColon,
		MissedComma,
		MissedQuot,
		MissedBracket,
		MissedBrace,
		EndOfFile,
		Undefined
	};

	/**
	* @brief Convert error code into C-string.
	* @param Err Error code.
	* @return Error string.
	*/
	const char* ErrorToString(Error Err)
	{
		switch (Err)
		{
			case Error::None:          return "None";          break;
			case Error::NoFile:        return "NoFile";        break;
			case Error::EmptyFile:     return "EmptyFile";     break;
			case Error::InvalidString: return "InvalidString"; break;
			case Error::InvalidNumber: return "InvalidNumber"; break;
			case Error::MissedColon:   return "MissedColon";   break;
			case Error::MissedComma:   return "MissedComma";   break;
			case Error::MissedQuot:    return "MissedQuot";    break;
			case Error::MissedBracket: return "MissedBracket"; break;
			case Error::MissedBrace:   return "MissedBrace";   break;
			case Error::EndOfFile:     return "EndOfFile";     break;
			case Error::Undefined:     return "Undefined";     break;
		}

		return "";
	}

	/**
	* Class containing tree of values.
	*/
	class Value
	{
	public:
		/** @brief Type of value. */
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
	protected:
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
		Value(const std::initializer_list<Value>& Arr) : Array(Arr.begin(), Arr.end()), ValueType(Type::Array) {}
		Value(const std::vector<Value>& Arr) : Array(Arr), ValueType(Type::Array) {}
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

		Value& operator=(const std::initializer_list<Value>& Arr)
		{
			Array.assign(Arr.begin(), Arr.end());
			ValueType = Type::Array;
			return *this;
		}

		Value& operator=(const std::vector<Value>& Arr)
		{
			Array = Arr;
			ValueType = Type::Array;
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
		/**
		* @brief Recursive method. Parses JSON file.
		* @param Str pointer on C-string contains JSON file.
		* @return Error code.
		*/
		Error Parse(const char** Str)
		{
			//Is a string
			if (**Str == '"')
			{
				(*Str)++;
				ExtractString(Str, StringValue);
				if (**Str != '"') return Error::InvalidString;
				(*Str)++;
				ValueType = Type::String;
				return Error::None;
			}

			//Is a bool
			if (memcmp(*Str, "true", 4) == 0 || memcmp(*Str, "false", 5) == true)
			{
				BoolValue = memcmp(*Str, "true", 4) == 0;
				(*Str) += BoolValue ? 4 : 5;
				ValueType = Type::Bool;
				return Error::None;
			}

			//Is a null
			if (memcmp(*Str, "null", 4) == 0)
			{
				(*Str) += 4;
				ValueType = Type::Null;
				return Error::None;
			}

			//Is a number
			if (**Str == '-' || (**Str >= '0' && **Str <= '9'))
			{
				bool Negative = **Str == '-';
				if (Negative) (*Str)++;

				bool Err;
				double Number = ParseInt(Str, Err);
				if (Err) return Error::InvalidNumber;

				//Decimal
				if (**Str == '.')
				{
					(*Str)++;

					if (!(**Str >= '0' && **Str <= '9'))
					{
						return Error::InvalidNumber;
					}

					Number += ParseDecimal(Str, Err);
					if (Err) return Error::InvalidNumber;
				}

				//Exponent
				if (**Str == 'e' || **Str == 'E')
				{
					(*Str)++;

					bool Negative = **Str == '-';
					if (Negative) (*Str)++;

					if (!(**Str >= '0' && **Str <= '9'))
					{
						return Error::InvalidNumber;
					}

					int Exponent = ParseInt(Str, Err);
					if (Err) return Error::InvalidNumber;
					Exponent *= Negative ? -1 : 1;
					Number *= pow(10.0, Exponent);
				}

				Number *= Negative ? -1 : 1;

				double tmp;
				if (modf(Number, &tmp) == 0)
				{
					ValueType = Type::Int;
					IntValue = Number;
					return Error::None;
				}
				else
				{
					ValueType = Type::Float;
					FloatValue = Number;
					return Error::None;
				}

				return Error::None;
			}

			//Is an array
			if (**Str == '[')
			{
				(*Str)++;

				while (**Str != 0)
				{
					if (!SkipWhitespace(Str)) return Error::EndOfFile;

					//An empty array
					if (Array.size() == 0 && **Str == ']')
					{
						(*Str)++;
						return Error::None;
					}

					Value Val;
					Error Err = Val.Parse(Str);
					if (Err != Error::None)
					{
						Array.clear();
						return Err;
					}

					Array.push_back(Val);

					if (!SkipWhitespace(Str))
					{
						Array.clear();
						return Error::EndOfFile;
					}

					//End of the array?
					if (**Str == ']')
					{
						(*Str)++;
						return Error::None;
					}
					else
					{
						Array.clear();
						return Error::MissedBracket;
					}

					if (**Str != ',')
					{
						Array.clear();
						return Error::MissedComma;
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
						return Error::EndOfFile;
					}

					//Empty object
					if (**Str == '}')
					{
						(*Str)++;
						ValueType = Type::Object;
						return Error::None;
					}

					if (!SkipWhitespace(Str))
					{
						Values.clear();
						return Error::None;
					}

					std::basic_string<char> Name;

					if (**Str == '"')
					{
						(*Str)++;
						ExtractString(Str, Name);
						if (**Str != '"')
						{
							Values.clear();
							return Error::MissedQuot;
						}

						(*Str)++;
						if (!SkipWhitespace(Str)) { Values.clear(); return Error::EndOfFile; }

						if (**Str == ':')
						{
							(*Str)++;
							Value Val;
							if (!SkipWhitespace(Str)) { Values.clear(); return Error::EndOfFile; }
							Error Err = Val.Parse(Str);
							if (Err != Error::None)   { Values.clear(); return Err; }
							Values[Name] = Val;
						} else return Error::MissedColon;
					}

					if (!SkipWhitespace(Str))
					{
						Values.clear();
						return Error::EndOfFile;
					}

					if (**Str == '}')
					{
						(*Str++);
						return Error::None;
					}

					if (**Str != ',')
					{
						Values.clear();
						return Error::MissedComma;
					}

					(*Str)++;
				}
			}

			return Error::Undefined;
		}

		/**
		* @brief Clears sub-values, string, array and sets type to empty object.
		*/
		void Clear()
		{
			StringValue.clear();
			Array.clear();
			Values.clear();
			ValueType = Type::Object;
		}
		/**
		* @brief Sets string value.
		* @note Clears array and object values.
		*/
		void SetString(const std::basic_string<char>& Str)
		{
			Clear();
			ValueType = Type::String;
			StringValue = Str;
		}
		/**
		* @brief Sets bool value.
		* @note Clears string, array and object values.
		*/
		void SetBool(bool Val)
		{
			Clear();
			ValueType = Type::Bool;
			BoolValue = Val;
		}
		/**
		* @brief Sets null value, clears each other.
		* @note Clears string, array and object values.
		*/
		void SetNull()
		{
			Clear();
			ValueType = Type::Null;
		}
		/**
		* @brief Sets int value, clears each other.
		* @note Clears string, array and object values.
		*/
		void SetInt(int Val)
		{
			Clear();
			ValueType = Type::Int;
			IntValue = Val;
		}
		/**
		* @brief Sets float value, clears each other.
		* @note Clears string, array and object values.
		*/
		void SetFloat(float Val)
		{
			Clear();
			ValueType = Type::Float;
			FloatValue = Val;
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
		/**
		* @brief Returns count of sub-values.
		*/
		int ChildrenCount() const
		{
			return Values.size();
		}
		/**
		* @brief Returns count of array elements.
		*/
		int ArraySize() const
		{
			return Array.size();
		}
		/**
		* @brief Returns iterator on first element of sub-values.
		*/
		ValueConstIterator ValueBegin() const
		{
			return Values.begin();
		}
		/**
		* @brief Returns iterator on element after after the last element of sub-values.
		*/
		ValueConstIterator ValueEnd() const
		{
			return Values.end();
		}
		/**
		* @brief Returns iterator on first element of array.
		*/
		ArrayConstIterator ArrayBegin() const
		{
			return Array.begin();
		}
		/**
		* @brief Returns iterator on the element after the last element of sub-values.
		*/
		ArrayConstIterator ArrayEnd() const
		{
			return Array.end();
		}
		/**
		* @brief Access to sub-value.
		* @details Creates new sub-value if value named `Key` does not exist.
		* @note This value automaticly becomes of **Object type**.
		* Clears array and string values.
		*
		* @param Key name of sub-value.
		*/
		Value& operator[](const std::basic_string<char>& Key)
		{
			ValueType = Type::Object;
			Array.clear();
			StringValue.clear();

			return Values[Key];
		}
		/**
		* @brief Access to array.
		* @details If Index greater than the size of array it expands the array by **only one** element and returns a reference to it.
		* @note This value automaticly becomes of **Array type**.
		* Clears object and string values.
		*
		* @param Index index of array element.
		*/
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
 
 	/**
 	* Class containing root value.
 	*/
	class JSON
	{
	protected:
		Value Root;
	public:
		/**
		* @brief Parses JSON from string.
		* @param String String with JSON-file.
		* @return Error code.
		*/
		Error Parse(const std::basic_string<char>& String)
		{
			if (String.empty()) return Error::EmptyFile;
			if (std::all_of(String.begin(), String.end(), [](char c)->bool{return std::isspace(c);})) return Error::EmptyFile;

			const char* Str = String.c_str();

			if (!SkipWhitespace(&Str)) return Error::EndOfFile;

			return Root.Parse(&Str);
		}
		/**
		* @brief Loads and parses JSON.
		* @param Filename Filename to load.
		* @return Error code.
		*/
		Error Load(const std::basic_string<char>& Filename)
		{
			std::ifstream ifs(Filename.c_str());
			if (!ifs.is_open()) return Error::NoFile;

			std::string str = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
			ifs.close();

			return Parse(str);
		}
		/**
		* @brief Writes JSON file.
		* @param Filename Filename to save.
		*/
		bool Save(const std::basic_string<char>& Filename)
		{
			std::ofstream ofs(Filename);
			ofs << Root << std::endl;
			return true;
		}
		/**
		* @brief Returns count of root's sub-values.
		*/
		int ChildrenCount() const
		{
			return Root.ChildrenCount();
		}
		/**
		* @brief Access to root's sub-value.
		* @details Creates new sub-value if value named `Key` does not exist.
		* @note Root automaticly becomes of **Object type**.
		* Clears array and string values.
		*
		* @param Key name of sub-value.
		*/
		Value& operator[](const std::basic_string<char>& Key)
		{
			return Root[Key];
		}
		/**
		* @brief Access to root's array.
		* @details If Index greater than the size of array it expands the array by **only one** element and returns a reference to it.
		* @note Root automaticly becomes of **Array type**.
		* Clears object and string values.
		*
		* @param Index index of array element.
		*/
		Value& operator[](int Index)
		{
			return Root[Index];
		}

		friend std::istream& operator>>(std::istream& Stream, JSON& J)
		{
			std::string str = std::string(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
			Error Err = J.Parse(str.c_str());

			if (Err != Error::None)
			{
				throw Err;
			}

			return Stream;
		}

		friend std::ostream& operator<<(std::ostream& Stream, const JSON& J)
		{
			Stream << J.Root;
			return Stream;
		}
	};
}










