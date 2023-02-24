#include "ConsoleLog.h"

ConsoleLog::ConsoleLog()
{
}

void ConsoleLog::Load(Rectangle _destination, float _messageLifetime, Color _messageColor)
{
	messageColor = _messageColor;
	messageLifetime = _messageLifetime;
	destination = _destination;
	outputTexture = LoadRenderTexture(destination.width, destination.height);
}

void ConsoleLog::Unload()
{
	UnloadRenderTexture(outputTexture);
}

void ConsoleLog::Print(const std::string &value)
{
	std::pair<float, std::string> toAdd;
	toAdd.first = messageLifetime;
	toAdd.second = value;
	content.emplace_back(toAdd);
}

void ConsoleLog::Update(float dt)
{
	if (content.size() > 0)
	{
		for (int i = content.size() - 1; i >= 0; i--)
		{
			content[i].first -= dt;
			if (content[i].first < 0.0f)
				content.erase(content.begin() + i);
		}
	}
}

void ConsoleLog::Render(bool bottom_is_latest)
{
	if (content.size() < 1)
		return;

	int beginY = 0;
	int increment = 20;
	if (bottom_is_latest)
	{
		beginY = destination.height - increment;
		increment *= -1;
	}

	BeginTextureMode(outputTexture);
	ClearBackground({0, 0, 0, 0});
	for (int i = content.size() - 1; i >= 0; i--)
	{
		DrawText(content[i].second.c_str(), 0, beginY + increment * i, 20, messageColor);	
	}
	EndTextureMode();

	DrawTexturePro(outputTexture.texture, {0.0f, 0.0f, destination.width, -destination.height}, destination, {0.0f, 0.0f}, 0.0f, WHITE);
}

void ConsoleLog::SetMessageColor(Color color)
{
	messageColor = color;
}

void ConsoleLog::SetMessageLifetime(float lifetime)
{
	messageLifetime = lifetime;
}

void ConsoleLog::SetDestinationBounds(Rectangle bounds)
{
	destination = bounds;
	UnloadRenderTexture(outputTexture);
	outputTexture = LoadRenderTexture(destination.width, destination.height);
}
