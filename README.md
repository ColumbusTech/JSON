# JSON
Simple and very fast C++ library for JSON parsing

The master hosted on github: https://github.com/ColumbusTech/JSON

## It is small
You need only to include **json.h** in your project and you may use it!

```cpp
#include "json.h"

using namespace ColumbusJSON;

...
```
## Documentation
To generate detailed documentation of library use
```
doxygen
```

## Using

**Just include json.h in your project and use it**

### Loading

#### Simple loading

```cpp
ColumbusJSON::JSON j;

auto Err = j.Load("test.json");
```

#### Loading with file string

```cpp
ColumbusJSON::JSON j;

std::ifstream ifs("test.json");
std::string string = std::string(std::istreambuf_iterator<char>(ifs),
                                 std::istreambuf_iterator<char>());
ifs.close();

auto Err = j.Parse(str);
```

#### Loading with stream

```cpp
ColumbusJSON::JSON j;

std::ifstream ifs("test.json");

try
{
	ifs >> j;
} catch (ColumbusJSON::Error Err)
{
	std::cout << ColumbusJSON::ErrorToString(Err) << std::endl;
}
```

#### So, if you have JSON like this

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

#### You may load it like this

```cpp
ColumbusJSON::JSON j;
auto Err = j.Load("test.json");

//JSON parsed and ready to use

//Main content
printf("%i\n", j["Int"].GetInt());       //123
printf("%f\n", j["Float"].GetFloat());   //3.141592
printf("%s\n", j["String"].GetString()); //First string


//Array of five ints
for (int i = 0; i < j["ArrayOfInts"].ArraySize(); i++)
{
	printf("%i ", j["ArrayOfInts"][i].GetInt()); //1..5
}
printf("\n");

//And sub-object content
printf("%i\n", j["Object"]["Int"].GetInt());       //321
printf("%f\n", j["Object"]["Float"].GetFloat());   //2.7
printf("%s\n", j["Object"]["String"].GetString()); //Second string
```
### Saving

#### Simple saving

```cpp
j.Save("save.json");
```

#### Saving with stream

```cpp
std::ofstream ofs("save.json");

ofs << j << std::endl;
```

#### You have this code

```cpp
ColumbusJSON::JSON j;

j["Int"] = 2;
j["Bool"] = true;
j["Null"] = nullptr;
j["Float"] = 123.321f;
j["String"] = "String";
j["Array"][0] = 1;
j["Array"][1] = 2;
j["Array"][2] = 3;
j["Array"][3] = 4;
j["Array"][4] = 5;

//You may use initializer list for arrays and it should be look like
//j["Array"] = {1, 2, 3, 4, 5};

j["Object"]["Int"] = 321;

j.Save("save.json");

//You may also save it like this
//std::ofstream ofs("save.json");
//ofs << j << std::endl;
```

#### And it should be saved like this

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

