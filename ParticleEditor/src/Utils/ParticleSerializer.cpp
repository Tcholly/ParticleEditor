#include "ParticleSerializer.h"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <cstddef>
#include <cerrno>

#include <Difu/Utils/Logger.h>

namespace ParticleSerializer
{
	std::string trim(const std::string& str)
	{
		size_t first = str.find_first_not_of(" \t");
		if (std::string::npos == first)
		{
			return str;
		}
		size_t last = str.find_last_not_of(" \t");
		return str.substr(first, (last - first + 1));
	}

	static int ParseHexDigit(char hex)
	{
		if (hex > 47 && hex < 58)
			return hex - 48;
		if (hex > 64 && hex < 71)
			return 10 + hex - 65;
		if (hex > 96 && hex < 103)
			return 10 + hex - 97;
	
		return -1;
	}
	
	static int ParseHexNumber(const std::string& hex)
	{
		int result = 0;
	
		for (char ch : hex)
		{
			int digit = ParseHexDigit(ch);
			if (digit < 0)
				return -1;
			
			result = result * 0x10 + digit;
		}
	
		return result;
	}

	static std::string DecToHex(unsigned char value)
	{
		std::string result;
	
		unsigned char temp = value / 0x10;
		if (temp < 10)
			result += (char)(48 + temp);
		else
			result += (char)(55 + temp);
	
		temp = value % 0x10;
		if (temp < 10)
			result += 48 + temp;
		else
			result += 55 + temp;
	
		return result;
	}
	
	static std::string ColorToAARRGGBB(const Color& color)
	{
		std::string result;
		result += DecToHex(color.a);
		result += DecToHex(color.r);
		result += DecToHex(color.g);
		result += DecToHex(color.b);
		return  result;
	}

	static void OutFloat(std::ofstream& out, const std::string& name, float value)
	{
		out << "\t" << name << " : float : " << value << ";\n";
	}

	static void OutVector2(std::ofstream& out, const std::string& name, Vector2 value)
	{
		out << "\t" << name << " : vector2f : { " << value.x << ", " << value.y << " };\n";
	}

	static void OutColor(std::ofstream& out, const std::string& name, Color value)
	{
		out << "\t" << name << " : color : #" << ColorToAARRGGBB(value) << ";\n";
	}

	// TODO: Error checking
	float InGetFloat(std::map<std::string, std::string>& map, const std::string& value)
	{
		std::string floatStr = map.at(value);
		return std::stof(floatStr);
	}

	Vector2 InGetVector2(std::map<std::string, std::string>& map, const std::string& value)
	{
		std::string vector2Str = map.at(value);
		vector2Str = trim(vector2Str.substr(1, vector2Str.size() - 2));

		size_t comma = vector2Str.find(",");
		std::string xStr = trim(vector2Str.substr(0, comma)); 
		std::string yStr = trim(vector2Str.substr(comma + 1)); 

		float x = std::stof(xStr);
		float y = std::stof(yStr);

		return {x, y};
	}

	Color InGetColor(std::map<std::string, std::string>& map, const std::string& value)
	{
		std::string hex = map.at(value);

		hex = hex.substr(1);

		if (hex.size() != 8)
		{
			Logger::Error("Unkown color format for {} : {} : Hex number is too big", value, map.at(value));
			return BLACK;
		}

		std::string aStr = hex.substr(0, 2);
		std::string rStr = hex.substr(2, 2);
		std::string gStr = hex.substr(4, 2);
		std::string bStr = hex.substr(6, 2);
		
		int a = ParseHexNumber(aStr);
		int r = ParseHexNumber(rStr);
		int g = ParseHexNumber(gStr);
		int b = ParseHexNumber(bStr);

		if (a < 0 || r < 0 || g < 0 || b < 0)
		{
			Logger::Error("Unkown color format for {} : {} : Failed to convert hex", value, map.at(value));
			return BLACK;
		}

		return {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};
	}

	void Serialize(const std::string& filename, const std::string& emitter_name, const ParticleEmitter& emitter)
	{
		std::ofstream out(filename);
		if (!out)
		{
			Logger::Error("Couldn't open file {}: {}", filename, std::strerror(errno));
			return;
		}

		out << emitter_name << "\n{\n";
		OutFloat(out, "LIFETIME", emitter.GetParticleLifetime());
		OutVector2(out, "RESOLUTION", emitter.GetParticleResolution());
		OutFloat(out, "MIN_SIZE_FACTOR", emitter.GetParticleMinSizeFactor());
		OutFloat(out, "MAX_SIZE_FACTOR", emitter.GetParticleMaxSizeFactor());
		OutVector2(out, "VELOCITY", emitter.GetSpawnVelocity());
		OutVector2(out, "ACCELERATION", emitter.GetParticleAcceleration());
		OutFloat(out, "CENTRIPETAL_ACCELERATION", emitter.GetCentripetalAcceleration());
		OutFloat(out, "ROTATION", emitter.GetParticleSpawnRotation());
		OutFloat(out, "ROTATION_VELOCITY", emitter.GetParticleSpawnRotationVelocity());
		OutFloat(out, "ROTATION_ACCELERATION", emitter.GetParticleRotationAcceleration());
		OutColor(out, "START_COLOR", emitter.GetStartColor());
		OutColor(out, "END_COLOR", emitter.GetEndColor());
		OutFloat(out, "SPAWN_INTERVAL", emitter.GetSpawnInterval());
		OutFloat(out, "RANDOMNESS", emitter.GetRandomness());
		OutFloat(out, "SPREAD", emitter.GetSpread());
		out << "}";
		out.close();

		LOG_INFO("Saved emitter to {} as {}", filename, emitter_name);
	}

	void Deserialize(const std::string& filename, ParticleEmitter* emitter)
	{
		std::ifstream in(filename);
		if (!in)
		{
			Logger::Error("Could not open {}: {}", filename, std::strerror(errno));
			return;
		}
		std::stringstream ss;
		ss << in.rdbuf();
		in.close();

		std::map<std::string, std::string> exprs;

		std::string line;
		std::getline(ss, line);
		std::string name = line;
		std::getline(ss, line);
		if (line[0] != '{')
			LOG_WARN("Second line should only consist of {");
		while (std::getline(ss, line))
		{
			size_t sepPos = line.find(":");
			if (sepPos == line.npos)
			{
				if (line[0] != '}')
					LOG_WARN("Couldn't resolve line: \"{}\"", line);
				continue;
			}
			std::string name = line.substr(0, sepPos);
			std::string rest = line.substr(sepPos + 1);
			name = trim(name);
			rest = trim(rest);

			sepPos = rest.find(":");
			if (sepPos == rest.npos)
			{
				LOG_WARN("Couldn't resolve line: \"{}\"", line);
				continue;
			}
			std::string value = rest.substr(sepPos + 1, rest.size() - sepPos - 2);
			value = trim(value);

			exprs[name] = value;
		}

		emitter->SetParticleLifetime(InGetFloat(exprs, "LIFETIME"));
		emitter->SetParticleResolution(InGetVector2(exprs, "RESOLUTION"));
		emitter->SetParticleMinSizeFactor(InGetFloat(exprs, "MIN_SIZE_FACTOR"));
		emitter->SetParticleMaxSizeFactor(InGetFloat(exprs, "MAX_SIZE_FACTOR"));
		emitter->SetSpawnVelocity(InGetVector2(exprs, "VELOCITY"));
		emitter->SetParticleAcceleration(InGetVector2(exprs, "ACCELERATION"));
		emitter->SetCentripetalAcceleration(InGetFloat(exprs, "CENTRIPETAL_ACCELERATION"));
		emitter->SetParticleSpawnRotation(InGetFloat(exprs, "ROTATION"));
		emitter->SetParticleSpawnRotationVelocity(InGetFloat(exprs, "ROTATION_VELOCITY"));
		emitter->SetParticleRotationAcceleration(InGetFloat(exprs, "ROTATION_ACCELERATION"));
		emitter->SetStartColor(InGetColor(exprs, "START_COLOR"));
		emitter->SetEndColor(InGetColor(exprs, "END_COLOR"));
		emitter->SetSpawnInterval(InGetFloat(exprs, "SPAWN_INTERVAL"));
		emitter->SetRandomness(InGetFloat(exprs, "RANDOMNESS"));
		emitter->SetSpread(InGetFloat(exprs, "SPREAD"));

		LOG_INFO("Succesfully opened {}", filename);
	}
}
