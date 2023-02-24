#include "MainScreen.h"

#include "Utils/ParticleSerializer.h"
#include "Utils/ConsoleLog.h"

#include <Difu/Particles/ParticleEmitter.h>
#include <Difu/Utils/Logger.h>

#include <cmath>
#include <fmt/core.h>
#include <raylib.h>
#include <rlImGui.h>
#include <rlImGuiColors.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <string>
#include <nfd.hpp>

namespace MainScreen
{
	static ParticleEmitter emitter;
	static bool askSave = false;
	static bool askOpen = false;
	static RenderTexture2D viewportTexture;
	static ImVec2 viewportPosition = { 0.0f, 0.0f };
	static ImVec2 viewportSize = { 0.0f, 0.0f };
	static bool viewportFocused = false;
	static ConsoleLog log;

	static void PrintFunction(std::string value)
	{
		log.Print(value);
	}


	static void Load()
	{
		log.Load({10.0f, GetScreenHeight() - 310.0f, 300.0f, 300.0f}, 7.0f, {123, 201, 34, 255});
		Logger::Bind(&PrintFunction);
		Particle baseParticle;
		baseParticle.lifetime = 1.0f;
		baseParticle.velocity = {100.0f, 0.0f};
		emitter = ParticleEmitter({0.0f, 0.0f}, {100.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 0.0f, 0.0f, 0.0f, BLACK, WHITE, {1.0f, 1.0f}, 1.0f, 20.0f, 1.0f, 0.1f, 1.0f, 2 * PI);
		emitter.StartEmitting();

		SetExitKey(0);

		NFD::Init();
		rlImGuiSetup(false);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}

	static void Unload()
	{
		rlImGuiShutdown();
		log.Unload();
		NFD::Quit();
	}

	// static float t = 0.0f;

	static void Update(float dt)
	{
		// t += dt;
		// emitter.SetCentripetalAcceleration(std::sin(t) * 200);
		// TODO: Add feature to bind value to a finction of time

		emitter.Update(dt);
		log.Update(dt);

		bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

		if (ctrl && IsKeyPressed(KEY_S))
		{
			askSave = true;	
		}

		if (ctrl && IsKeyPressed(KEY_O))
			askOpen = true;
		if (viewportFocused && CheckCollisionPointRec(GetMousePosition(), {viewportPosition.x, viewportPosition.y, viewportSize.x, viewportSize.y}) && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
		{
			SetMouseOffset(-viewportPosition.x, -viewportPosition.y);
			emitter.SetSpawnPosition(GetMousePosition());
			SetMouseOffset(0, 0);
		}
	}

	static void RenderViewport()
	{
		BeginTextureMode(viewportTexture);
		ClearBackground(WHITE);
		emitter.Render();
		EndTextureMode();
	}

	static void OnViewportResize(int width, int height)
	{
		emitter.SetSpawnPosition({width / 2.0f, height / 2.0f});

		UnloadRenderTexture(viewportTexture);
		viewportTexture = LoadRenderTexture(width, height);

		RenderViewport();
		log.SetDestinationBounds({10.0f, height - 310.0f, (float)width - 20.0f, 300.0f});
	}

	static void Render()
	{
		ClearBackground(WHITE);
		
		RenderViewport();

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

		// Dockspace
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		// Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		ImVec2 viewportNewSize = ImGui::GetContentRegionAvail();
		if (viewportNewSize.x != viewportSize.x || viewportNewSize.y != viewportSize.y)
		{
			viewportSize = viewportNewSize;
			OnViewportResize(viewportSize.x, viewportSize.y);
		}
		rlImGuiImageRect(&viewportTexture.texture, viewportSize.x, viewportSize.y, {0.0f, 0.0f, viewportSize.x, -viewportSize.y});
		viewportFocused = ImGui::IsWindowFocused();
		viewportPosition = ImGui::GetWindowPos();
		ImGui::End();
		ImGui::PopStyleVar();
		

		// Properties
		ImGui::Begin("Property editor");

		float lifetime = emitter.GetParticleLifetime();
		ImGui::DragFloat("Lifetime", &lifetime, 0.01f);	
		emitter.SetParticleLifetime(lifetime);

		Vector2 particleResolution = emitter.GetParticleResolution();
		ImGui::DragFloat2("Resolution", &particleResolution.x, 0.01f);
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




			ImGui::Begin("Save as...");

			static std::string emitterNameBuf;
			static std::string filenameBuf;
			ImGui::InputTextWithHint("Emitter name", "MyEmitter", &emitterNameBuf, ImGuiInputTextFlags_EscapeClearsAll);
			

			if (ImGui::Button("..."))
			{
				NFD::UniquePath outPath;
				nfdfilteritem_t filterItem[2] = {{"Save file", "save"}, {"Text file", "txt"}};
				nfdresult_t result = NFD::SaveDialog(outPath, filterItem, 2, filenameBuf.c_str(), filenameBuf.c_str());

				if (result == NFD_OKAY)
					filenameBuf = outPath.get();
				else if (result != NFD_CANCEL)
        			LOG_ERROR(NFD::GetError());
			}


			ImGui::SameLine();
			bool entered = ImGui::InputTextWithHint("Filename", "out.txt", &filenameBuf, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll);
			if (entered || ImGui::Button("Save"))
			{
				// TODO: Show that the file was saved
				// TODO: Ask for filename in a better way
				ParticleSerializer::Serialize(filenameBuf, emitterNameBuf, emitter);
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				askSave = false;

			ImGui::End();
		}

		// Open dialog
		if (askOpen)
		{
			ImGui::Begin("Open");

			static std::string filenameBuf;
			if (ImGui::Button("..."))
			{
				NFD::UniquePath outPath;
				nfdfilteritem_t filterItem[2] = {{"Save file", "save"}, {"Text file", "txt"}};
				nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 2, filenameBuf.c_str());
				if (result == NFD_OKAY)
					filenameBuf = outPath.get();
				else if (result != NFD_CANCEL)
        			LOG_ERROR(NFD::GetError());
			}
			ImGui::SameLine();
			bool entered = ImGui::InputTextWithHint("Filename", "in.txt", &filenameBuf, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll);
			if (entered || ImGui::Button("Open"))
			{
				askOpen = false;
				ParticleSerializer::Deserialize(filenameBuf, &emitter);
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				askOpen = false;

			ImGui::End();
		}
		rlImGuiEnd();

		log.Render(true);
	}

	static void OnResize(int width, int height)
	{
		(void)width;
		(void)height;
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
