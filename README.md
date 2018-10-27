# JSON
Simple and fast library parsing JSON

## It is small
You need only to include **json.h** in your project and you may use it!

c++
```
#include "json.h"

using namespace ColumbusJSON;

...
```

## Using

### To load JSON file in your program

#### So, if you have JSON like this

```json
{
	"Int": 123,
	"Float": 3.141592,
	"String": "Fuck you",
	"ArrayOfInts": [1, 2, 3, 4, 5],
	"Object":
	{
		"Int": 321,
		"Float": 2.7,
		"String": "Oh shit im sorry"
	}
}
```

#### You may load it like this

```c++
std::ifstream ifs("test.json");
std::string str = std::string(std::istreambuf_iterator<char>(ifs),
                              std::istreambuf_iterator<char>());

//File loaded

ColumbusJSON::JSON j;
j.Parse(str.c_str());
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
printf("%f\n", j["Object"]["float"].GetFloat());   //2.7
printf("%s\n", j["Object"]["String"].GetString()); //Second string
```

