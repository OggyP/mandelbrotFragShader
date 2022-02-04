#include "Platform/Platform.hpp"
#include <complex>

#define maxIterations 1000

const int screenSize[2] = { 1500, 1000 };

double centerOfScreen[] = { -0.5, 0 };
double distancePerPixel = 0.002;

std::array<std::complex<long double>, maxIterations>
iterate(std::complex<long double> c);
std::array<std::complex<long double>, maxIterations> iterate(std::complex<long double> c)
{
	std::complex<long double> z(0, 0);
	std::array<std::complex<long double>, maxIterations> iterations;
	for (size_t i = 0; i < maxIterations; i++)
	{
		z = pow(z, 2) + c; // z = z^2 + c
		iterations[i] = z;
	}
	return iterations;
}

u_int goesToInfinity(std::complex<long double> c);
u_int goesToInfinity(std::complex<long double> c)
{
	std::complex<long double> z(0, 0);
	for (size_t i = 0; i < maxIterations; i++)
	{
		z = pow(z, 2) + c; // z = z^2 + c
		if (pow(z.real(), 2) + pow(z.imag(), 2) > 4)
			return i;
	}
	return -1;
}

std::complex<long double> screenToComplexNumber(u_int x, u_int y);
std::complex<long double> screenToComplexNumber(u_int x, u_int y)
{
	return std::complex<long double>((x - (screenSize[0] / 2.0f)) * distancePerPixel + centerOfScreen[0], (y - (screenSize[1] / 2.0f)) * distancePerPixel + centerOfScreen[1]);
}

sf::Vector2f complexNumberToScreen(std::complex<long double> complexNum);
sf::Vector2f complexNumberToScreen(std::complex<long double> complexNum)
{
	return sf::Vector2f((complexNum.real() - centerOfScreen[0]) / distancePerPixel + (screenSize[0] / 2.0f), (complexNum.imag() - centerOfScreen[1]) / distancePerPixel + (screenSize[1] / 2.0f));
}

int main()
{
	std::cout << centerOfScreen[1] << "\n";

	util::Platform platform;

#if defined(_DEBUG)
	std::cout << "Hello World!" << std::endl;
#endif

	sf::RenderTexture background;
	sf::RenderWindow window;
	// in Windows at least, this must be called before creating the window
	float screenScalingFactor = platform.getScreenScalingFactor(window.getSystemHandle());
	// Use the screenScalingFactor
	window.create(sf::VideoMode(screenSize[0] * screenScalingFactor, screenSize[1] * screenScalingFactor), "Mandelbrot Fractal!");
	platform.setIcon(window.getSystemHandle());

	background.create(screenSize[0] * screenScalingFactor, screenSize[1] * screenScalingFactor);

	sf::RectangleShape rect;
	rect.setSize(sf::Vector2f(1, 1));

	sf::RectangleShape backgroundRect;
	backgroundRect.setSize(sf::Vector2f(screenSize[0], screenSize[1]));
	backgroundRect.setPosition(sf::Vector2f(0, 0));

	sf::CircleShape circle(2);
	circle.setFillColor(sf::Color::Red);
	circle.setOrigin(2, 2);

	sf::Font font;
	if (!font.loadFromFile("content/BAHNSCHRIFT.ttf"))
		std::cout << "Error Loading Font!\n";

	sf::Text text;
	text.setFillColor(sf::Color::White);
	text.setCharacterSize(200);
	text.setFont(font);
	text.setCharacterSize(40);

	sf::Shader shader;
	shader.loadFromFile("content/mandelbrot.glsl", sf::Shader::Fragment);

	if (!shader.isAvailable())
	{
		std::cout << "Shaders are not available\n";
	}

	bool update = true;
	bool keyPressed = false;

	sf::Event event;

	while (window.isOpen())
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (update)
		{
			update = false;
			background.clear();
			shader.setUniform("centerOfScreen", sf::Glsl::Vec2(centerOfScreen[0], centerOfScreen[1]));
			shader.setUniform("resolution", sf::Glsl::Vec2(screenSize[0], screenSize[1]));
			shader.setUniform("distancePerPixel", (float)distancePerPixel);
			background.draw(backgroundRect, &shader);
			// for (u_int x = 0; x < (u_int)screenSize[0]; x++)
			// {
			// 	for (u_int y = 0; y < (u_int)screenSize[1]; y++)
			// 	{
			// 		const auto currentPixel = screenToComplexNumber(x, y);
			// 		const u_int iterations = goesToInfinity(currentPixel);
			// 		if (iterations != (u_int)-1)
			// 		{
			// 			auto useIterations = iterations % 512;
			// 			u_int8_t colour = (iterations <= 255) ? 255 - useIterations : useIterations - 255;
			// 			rect.setFillColor(sf::Color(colour, colour, colour));
			// 			rect.setPosition(sf::Vector2f(x, y));
			// 			background.draw(rect);

			// 			// background.display();
			// 			// const auto backgroundTexture = background.getTexture();
			// 			// backgroundRect.setTexture(&backgroundTexture);
			// 			// window.draw(backgroundRect);
			// 			// window.display();
			// 		}
			// 	}
			// 	background.display();
			// 	const auto backgroundTexture = background.getTexture();
			// 	backgroundRect.setTexture(&backgroundTexture);
			// 	window.draw(backgroundRect);
			// 	window.display();
			// }
		}

		const auto texture = background.getTexture();
		backgroundRect.setTexture(&texture);

		window.clear();
		window.draw(backgroundRect);
		auto mousePos = sf::Mouse::getPosition(window);
		auto mouseComplexCoords = screenToComplexNumber(mousePos.x, mousePos.y);

		text.setPosition(sf::Vector2f(10, 10));
		text.setString(std::to_string(mouseComplexCoords.real()).append(" real"));
		window.draw(text);
		text.setPosition(sf::Vector2f(10, 60));
		text.setString(std::to_string(mouseComplexCoords.imag()).append(" imag"));
		window.draw(text);

		const auto fullIteration = iterate(mouseComplexCoords);
		for (size_t i = 0; i < fullIteration.size(); i++)
		{
			const auto iteration = fullIteration[i];
			circle.setPosition(complexNumberToScreen(iteration));
			window.draw(circle);
			// UNUSED(iteration);
		}

		window.display();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
		{
			if (!keyPressed)
			{
				update = true;
				distancePerPixel /= 1.3;
			}
			keyPressed = true;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
		{
			if (!keyPressed)
			{
				update = true;
				distancePerPixel *= 1.3;
			}
			keyPressed = true;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			if (!keyPressed)
			{
				update = true;
				centerOfScreen[1] -= distancePerPixel * 0.1 * screenSize[0];
			}
			keyPressed = true;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			if (!keyPressed)
			{
				update = true;
				centerOfScreen[1] += distancePerPixel * 0.1 * screenSize[0];
			}
			keyPressed = true;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			if (!keyPressed)
			{
				update = true;
				centerOfScreen[0] += distancePerPixel * 0.1 * screenSize[0];
			}
			keyPressed = true;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			if (!keyPressed)
			{
				update = true;
				centerOfScreen[0] -= distancePerPixel * 0.1 * screenSize[0];
			}
			keyPressed = true;
		}
		else
			keyPressed = false;
	}

	return 0;
}
