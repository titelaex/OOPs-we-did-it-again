module Models.Age;
import Models.Age;
import <string>;

using namespace Models;

std::string Models::AgeToString(Age a)
{
	switch (a)
	{
	case Age::AGE_I: return "I";
	case Age::AGE_II: return "II";
	case Age::AGE_III: return "III";
	default: return "Unknown";
	}
}
