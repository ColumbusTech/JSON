# JSON
Simple and very fast C++ library for JSON parsing

The master hosted on github: https://github.com/ColumbusUtrigas/JSON

## It is small
You need only to include **json.h** in your project and you may use it!

```cpp
#include "json.h"

using namespace columbus_json;

...
```
## Documentation
To generate detailed documentation of library use
```
doxygen
```

## Using

**Just include json.h in your project and use it**

## Loading

```cpp
try
{
	std::ifstream file("test.json");
	columbus_json::JSON j(file);
} catch (columbus_json::Error Err)
{
	std::cout << columbus_json::ErrorToString(Err) << std::endl;
}
```

### Or

```cpp
try
{
	std::ifstream file("test.json");
	columbus_json::JSON j;

	file >> j;
} catch (columbus_json::Error Err)
{
	std::cout << columbus_json::ErrorToString(Err) << std::endl;
}
```

### So, if you have JSON like this

```json
{
	"Int": 123,
	"Float": 3.141592,
	"String": "First string",
	"ArrayOfInts": [1, 2, 3, 4, 5],
	"Object":
	{
		"Int": 321,
		"Float": 2.7,
		"String": "Second string"
	}
}
```

### You may load it like this

```cpp
std::ifstream file("test.json");
columbus_json::JSON j(file);

//JSON parsed and ready to use

std::cout << std::boolalpha;

std::cout << j["Int"].Is<int>() << std::endl;     //true
std::cout << j["Float"].Is<float>() << std::endl; //true
std::cout << j["String"].Is<int>() << std::endl;  //false

//Main content
std::cout << j["Int"].Get<int>() << std::endl;            //123
std::cout << j["Float"].Get<float>() << std::endl;        //3.141592
std::cout << j["String"].Get<std::string>() << std::endl; //First string

//Array of five ints
for (auto& a : j["Object"]["Array"].Get<columbus_json::Array_t>())
{
	std::cout << a.Get<int>() << " "; //1..5
}
std::cout << std::endl;

//And sub-object content
std::cout << j["Object"]["Int"].Get<int>() << std::endl;            //321
std::cout << j["Object"]["Float"].Get<float>() << std::endl;        //2.7
std::cout << j["Object"]["String"].Get<std::string>() << std::endl; //Second string
```
## Saving

```cpp
std::ofstream file("save.json");
file << j;
```

### You have this code

```cpp
columbus_json::JSON j;

j["Int"] = 2;
j["Bool"] = true;
j["Null"] = nullptr;
j["Float"] = 123.321;
j["String"] = "String";
j["Array"][0] = 1;
j["Array"][1] = 2;
j["Array"][2] = 3;
j["Array"][3] = 4;
j["Array"][4] = 5;

//You may use initializer list for arrays and it should be look like
//j["Array"] = {1, 2, 3, 4, 5};

j["Object"]["Int"] = 321;

std::ofstream file("save.json");
file << j;

//You may also save it like this
//std::ofstream ofs("save.json");
//ofs << j << std::endl;
```

### And it should be saved like this

```json
{
	"Array": [1, 2, 3, 4, 5],
	"Bool": true,
	"Float": 123.320999,
	"Int": 2,
	"Null": null,
	"Object":
	{
		"Int": 321
	},
	"String": "String"
}
```

