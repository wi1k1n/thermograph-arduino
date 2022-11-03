#ifndef SDK_H__
#define SDK_H__

#include <unordered_map>

typedef bool Bool;
typedef signed char Char;
typedef unsigned char UChar;
typedef short int Short;
typedef unsigned short int UShort;
typedef long int Int;
typedef unsigned long int UInt;
typedef long long int Long;
typedef unsigned long long int ULong;
typedef float Float;
typedef double Double;

using Byte = UChar;
using Int32 = Int;
using UInt32 = UInt;
using Int64 = Long;
using UInt64 = ULong;

template<typename K, typename V>
class Hashmap : public std::unordered_map<K, V> {};



#endif // SDK_H__