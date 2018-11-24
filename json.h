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
#include <type_traits>
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

	typedef std::basic_string<char> String;

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

		typedef std::map<std::basic_string<char>, Value> Object;
		typedef std::vector<Value> Array;

		typedef Object::const_iterator ObjectConstIterator;
		typedef Array::const_iterator  ArrayConstIterator;

		typedef std::initializer_list<Value> ArrayInitializerList;
		typedef std::initializer_list<std::pair<std::basic_string<char>, Value>> ObjectInitializerList;

		typedef ObjectConstIterator obj_iter;
		typedef ArrayConstIterator  arr_iter;
	protected:
		String StringValue;
		bool BoolValue;
		int IntValue;
		float FloatValue;

		Object ObjectValue;
		Array ArrayValue;

		Type ValueType;
	public:
		Value() :                                           BoolValue(false), IntValue(0),   FloatValue(0.0f),  ValueType(Type::Object) {}
		Value(const String& Str) :                          BoolValue(false), IntValue(0),   FloatValue(0.0f),  StringValue(Str), ValueType(Type::String) {}
		Value(const Object& Obj) :                          BoolValue(false), IntValue(0),   FloatValue(0.0f),  ObjectValue(Obj), ValueType(Type::Object) {}
		Value(const ObjectInitializerList& Obj) :           BoolValue(false), IntValue(0),   FloatValue(0.0f),  ObjectValue(Obj.begin(), Obj.end()), ValueType(Type::Object) {}
		Value(const obj_iter& Begin, const obj_iter& End) : BoolValue(false), IntValue(0),   FloatValue(0.0f),  ObjectValue(Begin, End), ValueType(Type::Object) {}
		Value(const Array& Arr) :                           BoolValue(false), IntValue(0),   FloatValue(0.0f),  ArrayValue(Arr), ValueType(Type::Array) {}
		Value(const ArrayInitializerList& Arr) :            BoolValue(false), IntValue(0),   FloatValue(0.0f),  ArrayValue(Arr.begin(), Arr.end()), ValueType(Type::Array) {}
		Value(const arr_iter& Begin, const arr_iter& End) : BoolValue(false), IntValue(0),   FloatValue(0.0f),  ArrayValue(Begin, End), ValueType(Type::Array) {}
		Value(bool Bool) :                                  BoolValue(Bool),  IntValue(0),   FloatValue(0.0f),  ValueType(Type::Bool) {}
		Value(std::nullptr_t Null) :                        BoolValue(false), IntValue(0),   FloatValue(0.0f),  ValueType(Type::Null) {}
		Value(int Int) :                                    BoolValue(false), IntValue(Int), FloatValue(0.0f),  ValueType(Type::Int) {}
		Value(float Float) :                                BoolValue(false), IntValue(0),   FloatValue(Float), ValueType(Type::Float) {}

		Value& operator=(char Ch)
		{
			Clear();
			StringValue = Ch;
			ValueType = Type::String;
			return *this;
		}

		Value& operator=(const char* Str)
		{
			Clear();
			StringValue = Str;
			ValueType = Type::String;
			return *this;
		}

		Value& operator=(const String& Str)
		{
			Clear();
			StringValue = Str;
			ValueType = Type::String;
			return *this;
		}

		Value& operator=(const ObjectInitializerList& Obj)
		{
			Clear();
			ObjectValue.insert(Obj.begin(), Obj.end());
			ValueType = Type::Array;
			return *this;
		}

		Value& operator=(const ArrayInitializerList& Arr)
		{
			Clear();
			ArrayValue.assign(Arr.begin(), Arr.end());
			ValueType = Type::Array;
			return *this;
		}

		Value& operator=(const Array& Arr)
		{
			Clear();
			ArrayValue = Arr;
			ValueType = Type::Array;
			return *this;
		}

		Value& operator=(bool Bool)
		{
			Clear();
			BoolValue = Bool;
			ValueType = Type::Bool;
			return *this;
		}

		Value& operator=(std::nullptr_t Null)
		{
			Clear();
			ValueType = Type::Null;
			return *this;
		}

		Value& operator=(int Int)
		{
			Clear();
			IntValue = Int;
			ValueType = Type::Int;
			return *this;
		}

		Value& operator=(float Float)
		{
			Clear();
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

			//Is an object
			if (**Str == '{')
			{
				(*Str)++;

				while (**Str != 0)
				{
					if (!SkipWhitespace(Str))
					{
						ObjectValue.clear();
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
						ObjectValue.clear();
						return Error::None;
					}

					String Name;

					if (**Str == '"')
					{
						(*Str)++;
						ExtractString(Str, Name);
						if (**Str != '"')
						{
							ObjectValue.clear();
							return Error::MissedQuot;
						}

						(*Str)++;
						if (!SkipWhitespace(Str)) { ObjectValue.clear(); return Error::EndOfFile; }

						if (**Str == ':')
						{
							(*Str)++;
							Value Val;
							if (!SkipWhitespace(Str)) { ObjectValue.clear(); return Error::EndOfFile; }
							Error Err = Val.Parse(Str);
							if (Err != Error::None)   { ObjectValue.clear(); return Err; }
							ObjectValue[Name] = Val;
						} else return Error::MissedColon;
					}

					if (!SkipWhitespace(Str))
					{
						ObjectValue.clear();
						return Error::EndOfFile;
					}

					if (**Str == '}')
					{
						(*Str++);
						return Error::None;
					}

					if (**Str != ',')
					{
						ObjectValue.clear();
						return Error::MissedComma;
					}

					(*Str)++;
				}
			}

			//Is an array
			if (**Str == '[')
			{
				(*Str)++;

				while (**Str != 0)
				{
					if (!SkipWhitespace(Str)) return Error::EndOfFile;

					//An empty array
					if (ArrayValue.size() == 0 && **Str == ']')
					{
						(*Str)++;
						return Error::None;
					}

					Value Val;
					Error Err = Val.Parse(Str);
					if (Err != Error::None)
					{
						ArrayValue.clear();
						return Err;
					}

					ArrayValue.push_back(Val);

					if (!SkipWhitespace(Str))
					{
						ArrayValue.clear();
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
						ArrayValue.clear();
						return Error::MissedBracket;
					}

					if (**Str != ',')
					{
						ArrayValue.clear();
						return Error::MissedComma;
					}

					(*Str)++;
				}
			}

			return Error::Undefined;
		}

		/**
		* @brief Clears object, string, array and sets type to empty object.
		*/
		void Clear()
		{
			StringValue.clear();
			BoolValue = false;
			IntValue = 0;
			FloatValue = 0.0f;
			ArrayValue.clear();
			ObjectValue.clear();
			ValueType = Type::Object;
		}
		/**
		* @brief Sets the value to string, clears all others.
		* @note Clears array and object values.
		*/
		void SetString(const String& Val)
		{
			*this = Val;
		}
		/**
		* @brief Sets the value to bool, clears all others.
		* @note Clears string, array and object values.
		*/
		void SetBool(bool Val)
		{
			*this = Val;
		}
		/**
		* @brief Sets the value to null, clears all others.
		* @note Clears string, array and object values.
		*/
		void SetNull()
		{
			*this = nullptr;
		}
		/**
		* @brief Sets the value to int, clears all others.
		* @note Clears string, array and object values.
		*/
		void SetInt(int Val)
		{
			*this = Val;
		}
		/**
		* @brief Sets the value to float, clears all others.
		* @note Clears string, array and object values.
		*/
		void SetFloat(float Val)
		{
			*this = Val;
		}
		/**
		* @brief Sets the value of object, clears all others/
		* @note Clears string and array values.
		*/
		void SetObject(const Object& Val)
		{
			*this = Val;
		}
		/**
		* @brief Sets the value of object, clears all others/
		* @note Clears string and array values.
		*/
		void SetObject(const ObjectInitializerList& Val)
		{
			*this = Val;
		}
		/**
		* @brief Sets the value of object, clears all others/
		* @note Clears string and array values.
		*/
		void SetObject(const ObjectConstIterator& Begin, const ObjectConstIterator& End)
		{
			Clear();
			ObjectValue.insert(Begin, End);
			ValueType = Type::Object;
		}
		/**
		* @brief Sets the value to array, clears all others.
		* @note Clears string and object values.
		*/
		void SetArray(const Array& Val)
		{
			*this = Val;
		}
		/**
		* @brief Sets the value to array, clears all others.
		* @note Clears string and object values.
		*/
		void SetArray(const ArrayInitializerList& Val)
		{
			*this = Val;
		}
		/**
		* @brief Sets the value to array, clears all others.
		* @note Clears string and object values.
		*/
		void SetArray(const ArrayConstIterator& Begin, const ArrayConstIterator& End)
		{
			Clear();
			ArrayValue.assign(Begin, End);
			ValueType = Type::Array;
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
		/**
		* @breif Finds value in object values.
		*/
		bool HasValue(const String& Key) const
		{
			return ObjectValue.find(Key) != ObjectValue.end();
		}
		/**
		* @brief Returns count of object values.
		*/
		int ChildrenCount() const
		{
			return ObjectValue.size();
		}
		/**
		* @brief Returns count of array elements.
		*/
		int ArraySize() const
		{
			return ArrayValue.size();
		}
		/**
		* @brief Returns iterator on first element of object values.
		*/
		ObjectConstIterator ObjectBegin() const
		{
			return ObjectValue.begin();
		}
		/**
		* @brief Returns iterator on element after after the last element of object values.
		*/
		ObjectConstIterator ObjectEnd() const
		{
			return ObjectValue.end();
		}
		/**
		* @brief Returns iterator on first element of array.
		*/
		ArrayConstIterator ArrayBegin() const
		{
			return ArrayValue.begin();
		}
		/**
		* @brief Returns iterator on the element after the last element of sub-values.
		*/
		ArrayConstIterator ArrayEnd() const
		{
			return ArrayValue.end();
		}
		/**
		* @brief Returns generic value.
		* @note All of this values you can obtain via cast operator.
		*/
		template <typename T>
		T Get() const;
		/**
		* @brief Sets value to string.
		* @note If value type is not string it is converts to string.
		* @return The same reference as *Out*.
		*/
		String& Get(String& Out) const
		{
			switch (ValueType)
			{
			case Value::Type::String: Out = StringValue;                break;
			case Value::Type::Bool:   Out = std::to_string(BoolValue);  break;
			case Value::Type::Int:    Out = std::to_string(IntValue);   break;
			case Value::Type::Float:  Out = std::to_string(FloatValue); break;
			}

			return Out;
		}
		/**
		* @brief Sets value to generic value.
		* @note If value type is not T it is converts to T.
		* @return The same reference as *Out*.
		*/
		template <typename T>
		typename std::enable_if<std::is_arithmetic<T>::value, T>::type& Get(T& Out) const
		{
			switch (ValueType)
			{
			case Value::Type::Bool:  Out = (T)BoolValue;  break;
			case Value::Type::Int:   Out = (T)IntValue;   break;
			case Value::Type::Float: Out = (T)FloatValue; break;
			}

			return Out;
		}
		/**
		* @brief Access to object value.
		* @details Creates new object value if value named `Key` does not exist.
		* @note This value automaticly becomes of **Object type**.
		* Clears array and string values.
		*
		* @param Key name of object value.
		*/
		Value& operator[](const String& Key)
		{
			ValueType = Type::Object;
			ArrayValue.clear();
			StringValue.clear();

			return ObjectValue[Key];
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
			ObjectValue.clear();
			StringValue.clear();

			if (Index >= ArrayValue.size())
			{
				ArrayValue.push_back({});
			}

			return ArrayValue[Index];
		}

		/**
		* @brief Parses tree in begining of *Val* into std::ostream.
		*/
		friend std::ostream& operator<<(std::ostream& Stream, const Value& Val)
		{
			static int Iteration = 0;

			auto Tabs = [&]()->void { for(int i = 0; i < Iteration; i++) Stream << '\t'; };

			std::string Str;

			switch (Val.ValueType)
			{
			case Value::Type::String: Str = '\"' + Val.StringValue + '\"';    break;
			case Value::Type::Bool:   Str = Val.BoolValue ? "true" : "false"; break;
			case Value::Type::Null:   Str = "null";                           break;
			case Value::Type::Int:    Str = std::to_string(Val.IntValue);     break;
			case Value::Type::Float:  Str = std::to_string(Val.FloatValue);   break;
			}

			if (Val.GetType() == Value::Type::Object)
			{
				Tabs();
				Stream << '{' << std::endl;

				Iteration++;
				for (auto it = Val.ObjectBegin(); it != Val.ObjectEnd(); ++it)
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

					if (++it != Val.ObjectEnd()) Stream << ','; --it;
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

		template <typename T, typename Enable = void>
		struct traits {};

		template <typename T>
		struct traits<T, typename std::enable_if<
			std::is_same          <typename std::decay<T>::type, std::basic_string<char>>::value ||
			std::is_same          <typename std::decay<T>::type, char*>::value ||
			std::is_integral      <typename std::decay<T>::type>::value ||
			std::is_floating_point<typename std::decay<T>::type>::value>::type>
		{ typedef T type; };

		template <typename T>
		operator T() const
		{
			return Get<T>();
		}

		template <typename T, typename = typename traits<T>::type>
		friend T& operator<<(T& Out, const Value& Val)
		{
			Val.Get(Out);
			return Out;
		}

		~Value() {}
	};

	/**
	* @brief Returns the string value.
	* @note If the value type is not a string, it is conerted to a string type.
	*/
	template <> String Value::Get() const
	{
		switch (ValueType)
		{
		case Value::Type::String: return StringValue;                  break;
		case Value::Type::Bool:   return BoolValue ? "true" : "false"; break;
		case Value::Type::Null:   return "null";                       break;
		case Value::Type::Int:    return std::to_string(IntValue);     break;
		case Value::Type::Float:  return std::to_string(FloatValue);   break; 
		}

		return "";
	}
	/**
	* @brief Returns the bool value.
	* @note If the value type is not a bool, it is conerted to a bool type.
	*/
	template <> bool Value::Get() const
	{
		switch (ValueType)
		{
		case Value::Type::Bool:  return BoolValue;        break;
		case Value::Type::Int:   return (bool)IntValue;   break;
		case Value::Type::Float: return (bool)FloatValue; break;
		}

		return false;
	}
	/**
	* @brief Returns the int value.
	* @note If the value type is not an intool, it is conerted to an int type.
	*/
	template <> int Value::Get() const
	{
		switch (ValueType)
		{
		case Value::Type::Bool:  return (int)BoolValue;  break;
		case Value::Type::Int:   return IntValue;        break;
		case Value::Type::Float: return (int)FloatValue; break;
		}

		return 0;
	}
	/**
	* @brief Returns the float value.
	* @note If the value type is not a float, it is conerted to a float type.
	*/
	template <> float Value::Get() const
	{
		switch (ValueType)
		{
		case Value::Type::Bool:  return (float)BoolValue; break;
		case Value::Type::Int:   return (float)IntValue;  break;
		case Value::Type::Float: return FloatValue;       break;
		}

		return 0.0f;
	}
	/**
	* @brief Returns the object value.
	* @note If the value type is not an object, it is **NOT** conerted to an object type.
	*/
	template <> Value::Object Value::Get() const
	{
		if (ValueType == Value::Type::Object)
		{
			return ObjectValue;
		}

		return Value::Object();
	}
	/**
	* @brief Returns the array value.
	* @note If the value type is not an array, it is **NOT** conerted to an array type.
	*/
	template <> Value::Array Value::Get() const
	{
		if (ValueType == Value::Type::Array)
		{
			return ArrayValue;
		}

		return Value::Array();
	}
 
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
		Error Parse(const String& Text)
		{
			if (Text.empty()) return Error::EmptyFile;
			if (std::all_of(Text.begin(), Text.end(), [](char c)->bool{return std::isspace(c);})) return Error::EmptyFile;

			const char* Str = Text.c_str();

			if (!SkipWhitespace(&Str)) return Error::EndOfFile;

			return Root.Parse(&Str);
		}
		/**
		* @brief Loads and parses JSON.
		* @param Filename Filename to load.
		* @return Error code.
		*/
		Error Load(const String& Filename)
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
		bool Save(const String& Filename)
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
		* @brief Access to root's object value.
		* @details Creates new sub-value if value named `Key` does not exist.
		* @note Root automaticly becomes of **Object type**.
		* Clears array and string values.
		*
		* @param Key name of sub-value.
		*/
		Value& operator[](const String& Key)
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



