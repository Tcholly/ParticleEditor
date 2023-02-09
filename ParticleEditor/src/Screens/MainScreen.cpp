#include "MainScreen.h"
#include <Difu/Particles/ParticleEmitter.h>
#include <Difu/Utils/Logger.h>

#include <cmath>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <map>
#include <fmt/core.h>
#include <raylib.h>
#include <rlImGui.h>
#include <rlImGuiColors.h>
#include <imgui.h>
#include <string>

namespace MainScreen
{
	static ParticleEmitter emitter;
	static bool askSave = false;
	static bool askOpen = false;

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

	static void OutFloat(std::ofstream& out, const std::string& name, float value)
	{
		out << name << " = " << value << "\n";
	}

	static void OutVector2(std::ofstream& out, const std::string& name, Vector2 value)
	{
		out << name << " = { " << value.x << ", " << value.y << " }\n";
	}

	static void OutColor(std::ofstream& out, const std::string& name, Color value)
	{
		out << name << " = { " << (int)value.r << ", " << (int)value.g << ", " << (int)value.b << ", " << (int)value.a << " }\n";
	}

	static void Save(std::string filename)
	{
		std::ofstream out(filename);
		OutFloat(out, "lifetime", emitter.GetParticleLifetime());
		OutVector2(out, "resolution", emitter.GetParticleResolution());
		OutFloat(out, "minSizeFactor", emitter.GetParticleMinSizeFactor());
		OutFloat(out, "maxSizeFactor", emitter.GetParticleMaxSizeFactor());
		OutVector2(out, "velocity", emitter.GetSpawnVelocity());
		OutVector2(out, "acceleration", emitter.GetParticleAcceleration());
		OutFloat(out, "centripetalAcceleration", emitter.GetCentripetalAcceleration());
		OutFloat(out, "rotation", emitter.GetParticleSpawnRotation());
		OutFloat(out, "rotationVelocity", emitter.GetParticleSpawnRotationVelocity());
		OutFloat(out, "rotationAcceleration", emitter.GetParticleRotationAcceleration());
		OutColor(out, "startColor", emitter.GetStartColor());
		OutColor(out, "endColor", emitter.GetEndColor());
		OutFloat(out, "spawnInterval", emitter.GetSpawnInterval());
		OutFloat(out, "randomness", emitter.GetRandomness());
		OutFloat(out, "spread", emitter.GetSpread());
		out.close();

		LOG_INFO("Saved emitter to {}", filename);
		// TODO: Show that the file was saved
		// TODO: Ask for filename in a better way
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
		std::string colorStr = map.at(value);

		colorStr = trim(colorStr.substr(1, colorStr.size() - 2));

		size_t comma = colorStr.find(",");
		std::string rStr = trim(colorStr.substr(0, comma)); 
		colorStr = trim(colorStr.substr(comma + 1)); 

		comma = colorStr.find(",");
		std::string gStr = trim(colorStr.substr(0, comma)); 
		colorStr = trim(colorStr.substr(comma + 1)); 

		comma = colorStr.find(",");
		std::string bStr = trim(colorStr.substr(0, comma)); 
		std::string aStr = trim(colorStr.substr(comma + 1)); 

		int r = std::stoi(rStr);
		int g = std::stoi(gStr);
		int b = std::stoi(bStr);
		int a = std::stoi(aStr);

		return {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};
	}

	static void Open(std::string filename)
	{
		std::ifstream in(filename);
		std::stringstream ss;
		ss << in.rdbuf();
		in.close();

		std::map<std::string, std::string> exprs;

		std::string line;
		while (std::getline(ss, line))
		{
			size_t eqPos = line.find("=");
			if (eqPos == line.npos)
			{
				LOG_WARN("Couldn't resolve line: \"{}\"", line);
				continue;
			}
			std::string name = line.substr(0, eqPos);
			std::string value = line.substr(eqPos + 1);
			name = trim(name);
			value = trim(value);

			exprs[name] = value;
		}

		emitter.SetParticleLifetime(InGetFloat(exprs, "lifetime"));
		emitter.SetParticleResolution(InGetVector2(exprs, "resolution"));
		emitter.SetParticleMinSizeFactor(InGetFloat(exprs, "minSizeFactor"));
		emitter.SetParticleMaxSizeFactor(InGetFloat(exprs, "maxSizeFactor"));
		emitter.SetSpawnVelocity(InGetVector2(exprs, "velocity"));
		emitter.SetParticleAcceleration(InGetVector2(exprs, "acceleration"));
		emitter.SetCentripetalAcceleration(InGetFloat(exprs, "centripetalAcceleration"));
		emitter.SetParticleSpawnRotation(InGetFloat(exprs, "rotation"));
		emitter.SetParticleSpawnRotationVelocity(InGetFloat(exprs, "rotationVelocity"));
		emitter.SetParticleRotationAcceleration(InGetFloat(exprs, "rotationAcceleration"));
		emitter.SetStartColor(InGetColor(exprs, "startColor"));
		emitter.SetEndColor(InGetColor(exprs, "endColor"));
		emitter.SetSpawnInterval(InGetFloat(exprs, "spawnInterval"));
		emitter.SetRandomness(InGetFloat(exprs, "randomness"));
		emitter.SetSpread(InGetFloat(exprs, "spread"));
	}

	static char textBuffer[128];

	static void Load()
	{
		Particle baseParticle;
		baseParticle.lifetime = 1.0f;
		baseParticle.velocity = {100.0f, 0.0f};
		emitter = ParticleEmitter({0.0f, 0.0f}, {100.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 0.0f, 0.0f, 0.0f, BLACK, WHITE, {1.0f, 1.0f}, 1.0f, 20.0f, 1.0f, 0.1f, 1.0f, 2 * PI);
		emitter.StartEmitting();

		SetExitKey(0);

		rlImGuiSetup(false);
	}

	static void Unload()
	{
		rlImGuiShutdown();
	}

	// static float t = 0.0f;

	static void Update(float dt)
	{
		// t += dt;
		// emitter.SetCentripetalAcceleration(std::sin(t) * 200);
		// TODO: Add feature to bind value to a finction of time

		emitter.Update(dt);

		bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

		if (ctrl && IsKeyPressed(KEY_S))
			askSave = true;	

		if (ctrl && IsKeyPressed(KEY_O))
			askOpen = true;
		if (!ImGui::GetIO().WantCaptureMouse && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
			emitter.SetSpawnPosition(GetMousePosition());
	}

	static void Render()
	{
		ClearBackground(WHITE);
		emitter.Render();

		rlImGuiBegin();

		// Menu Bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save", "ctrl+s"))
					askSave = true;

				if (ImGui::MenuItem("Open", "ctrl+o"))
					askOpen = true;

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Properties
		bool open = true;
		ImGui::Begin("Property editor", &open, 0);

		float lifetime = emitter.GetParticleLifetime();
		ImGui::InputFloat("Lifetime", &lifetime, 0.01f);	
		emitter.SetParticleLifetime(lifetime);

		Vector2 particleResolution = emitter.GetParticleResolution();
		ImGui::InputFloat2("Resolution", &particleResolution.x);
		emitter.SetParticleResolution(particleResolution);

		float minSizeFactor = emitter.GetParticleMinSizeFactor();
		ImGui::InputFloat("Min. Size Factor", &minSizeFactor, 0.01);
		emitter.SetParticleMinSizeFactor(minSizeFactor);

		float maxSizeFactor = emitter.GetParticleMaxSizeFactor();
		ImGui::InputFloat("Max. Size Factor", &maxSizeFactor, 0.01);
		emitter.SetParticleMaxSizeFactor(maxSizeFactor);

		Vector2 particleVelocity = emitter.GetSpawnVelocity();
		ImGui::InputFloat2("Velocity", &particleVelocity.x);
		emitter.SetSpawnVelocity(particleVelocity);

		Vector2 particleAcceleration = emitter.GetParticleAcceleration();
		ImGui::InputFloat2("Acceleration", &particleAcceleration.x);
		emitter.SetParticleAcceleration(particleAcceleration);

		float centripetalAcceleration = emitter.GetCentripetalAcceleration();
		ImGui::InputFloat("Centripetal Acceleration", &centripetalAcceleration, 0.01f);
		emitter.SetCentripetalAcceleration(centripetalAcceleration);
		
		float particleRotation = emitter.GetParticleSpawnRotation();
		ImGui::InputFloat("Rotation", &particleRotation, 0.01f);
		emitter.SetParticleSpawnRotation(particleRotation);

		float particleRotationVelocity = emitter.GetParticleSpawnRotationVelocity();
		ImGui::InputFloat("Rotational Velocity", &particleRotationVelocity, 0.01f);
		emitter.SetParticleSpawnRotationVelocity(particleRotationVelocity);

		float rotationAcceleration = emitter.GetParticleRotationAcceleration();
		ImGui::InputFloat("Rotational Acceleration", &rotationAcceleration, 0.01f);
		emitter.SetParticleRotationAcceleration(rotationAcceleration);

		Color startColor = emitter.GetStartColor();
		ImVec4 imStartColor = rlImGuiColors::Convert(startColor);
		ImGui::ColorEdit4("Start Color", &imStartColor.x);
		startColor = rlImGuiColors::Convert(imStartColor);
		emitter.SetStartColor(startColor);

		Color endColor = emitter.GetEndColor();
		ImVec4 imEndColor = rlImGuiColors::Convert(endColor);
		ImGui::ColorEdit4("End Color", &imEndColor.x);
		endColor = rlImGuiColors::Convert(imEndColor);
		emitter.SetEndColor(endColor);

		float spawnInterval = emitter.GetSpawnInterval();
		ImGui::InputFloat("Interval", &spawnInterval);
		if (spawnInterval > 0.0f)
			emitter.SetSpawnInterval(spawnInterval);

		float randomness = emitter.GetRandomness();
		ImGui::InputFloat("Randomness", &randomness);
		emitter.SetRandomness(randomness);

		float spread = emitter.GetSpread();
		ImGui::InputFloat("Spread", &spread);
		emitter.SetSpread(spread);

		ImGui::End();

		// Save dialog
		if (askSave)
		{
			ImGui::Begin("Save as...", &open);

			if (ImGui::InputTextWithHint("Filename", "out.txt", textBuffer, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll) || ImGui::Button("Save"))
			{
				askSave = false;
				Save(std::string(textBuffer));
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				askSave = false;

			ImGui::End();
		}

		// Open dialog
		if (askOpen)
		{
			ImGui::Begin("Open", &open);

			if (ImGui::InputTextWithHint("Filename", "in.txt", textBuffer, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll) || ImGui::Button("Open"))
			{
				askOpen = false;
				Open(std::string(textBuffer));
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				askOpen = false;

			ImGui::End();
		}
		rlImGuiEnd();
	}

	static void OnResize(int width, int height)
	{
		emitter.SetSpawnPosition({width / 2.0f, height / 2.0f});
	}

	Screen GetScreen()
	{
		Screen screen;

		screen.LoadFunction = &Load;
		screen.UnloadFunction = &Unload;
		screen.UpdateFunction = &Update;
		screen.RenderFunction = &Render;
		screen.OnResize = &OnResize;

		return screen;
	}
}
