#pragma once

#include <vector>
#include <string>
#include <raylib.h>

class ConsoleLog
{
public:
	ConsoleLog();

	void Load(Rectangle destination, float messageLifetime, Color messageColor);
	void Unload();

	void Print(const std::string& value);

	void Update(float dt);
	void Render(bool bottom_is_latest);

	void SetMessageColor(Color color);
	void SetMessageLifetime(float lifetime);
	void SetDestinationBounds(Rectangle bounds);

private:
	std::vector<std::pair<float, std::string>> content;
	float messageLifetime = 5.0f;
	Color messageColor = BLACK;
	Rectangle destination = {0.0f, 0.0f, 100.0f, 100.0f};
	RenderTexture2D outputTexture;
};
