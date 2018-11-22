#ifndef MySFMLClasses_h
#define MySFMLClasses_h

#include "SFML\Graphics.hpp"
#include <string>
#define reduce(foo) (foo)
#define COLOUR_OFFSET -230	
#define RED_MULTIPLIER 0.09				// Multiply the number of hits in a pixel by these values
#define GREEN_MULTIPLIER 0.11				// (needs to be lower the greater the number of source
#define BLUE_MULTIPLIER 0.18				// values and iterations, else image will be too bright)
using namespace sf;

class MySprite
	:public Sprite
{
public:
	Texture texture;

	MySprite(std::string textureFile, float xPosition = 0, float yPosition = 0)
	{
		texture.loadFromFile(textureFile);
		this->setPosition(xPosition, yPosition);
		this->setTexture(texture);
	}
};

class MyText
	:public Text
{
public:
	Font font;
	MyText(std::string text, std::string fontFile, int fontSize, float xPosition, float yPosition)
	{
		font.loadFromFile(fontFile);
		this->setString(text);
		this->setFont(font);
		this->setCharacterSize(fontSize);
		this->setPosition(xPosition, yPosition);
	}
};

class MyRectangleShape
	:public RectangleShape
{
public:
	MyRectangleShape() {}
	MyRectangleShape(float width, float height, float xPosition, float yPosition, Color fillColor = Color(100, 100, 100), float thickness = 5, Color outlineColor = Color(40, 40, 40))
	{
		this->setSize(Vector2f(width, height));
		this->setPosition(xPosition, yPosition);
		this->setFillColor(fillColor);
		this->setOutlineThickness(thickness);
		this->setOutlineColor(outlineColor);
	}
};

class Button
{
public:
	MyText text;
	MyRectangleShape rectangle;

	Button(std::string title, std::string fontFile, int fontSize, float xPositionF, float yPositionF, float width, float height, float xPosition, float yPosition, Color fillColor = Color(100, 100, 100), float thickness = 5, Color outlineColor = Color(40, 40, 40))
		:text(title, fontFile, fontSize, xPositionF, yPositionF),
		rectangle(width, height, xPosition, yPosition, fillColor, thickness, outlineColor)
	{
	}

};

class TextField
{
public:
	MyText text;
	MyRectangleShape rectangle;

	TextField(std::string title, std::string fontFile, int fontSize, float xPositionF, float yPositionF, float width, float height, float xPosition, float yPosition, Color fillColor = Color(100, 100, 100), float thickness = 5, Color outlineColor = Color(40, 40, 40))
		:text(title, fontFile, fontSize, xPositionF, yPositionF),
		rectangle(width, height, xPosition, yPosition, fillColor, thickness, outlineColor)
	{
	}

};

#endif // !MySFMLClasses_hpp
