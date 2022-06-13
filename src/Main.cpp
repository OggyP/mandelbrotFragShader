#include "Platform/Platform.hpp"
#include <complex>

#define maxIterations 1000

uint screenSize[2] = { 1500, 1000 };

double centerOfScreen[] = { -0.5, 0 };
double distancePerPixel = 0.002;

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

std::array<sf::Vector2f, maxIterations> iterate(std::complex<long double> c, float power);
std::array<sf::Vector2f, maxIterations> iterate(std::complex<long double> c, float power)
{
	std::complex<long double> z(0, 0);
	std::array<sf::Vector2f, maxIterations> iterations;
	for (size_t i = 0; i < maxIterations; i++)
	{
		z = pow(z, power) + c; // z = z^2 + c
		iterations[i] = complexNumberToScreen(z);
	}
	return iterations;
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
#if defined(__linux__)
	window.create(sf::VideoMode(screenSize[0] * screenScalingFactor, screenSize[1] * screenScalingFactor), "Mandelbrot Fractal!");
	window.create(sf::VideoMode(1680, 1050), "Mandelbrot Fractal!", sf::Style::Resize | sf::Style::Fullscreen | sf::Style::Close);
#else
	window.create(sf::VideoMode(0, 0), "Mandelbrot Fractal!", sf::Style::Resize | sf::Style::Close);
	platform.toggleFullscreen(window.getSystemHandle(), sf::Style::Fullscreen, false, sf::Vector2u(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height));
#endif
	window.create(sf::VideoMode(window.getSize().x, window.getSize().y), "Mandelbrot Fractal!");
	screenSize[0] = window.getSize().x;
	screenSize[1] = window.getSize().y;
	platform.setIcon(window.getSystemHandle());

	background.create(screenSize[0] * screenScalingFactor, screenSize[1] * screenScalingFactor);

	sf::RectangleShape rect;
	rect.setSize(sf::Vector2f(1, 1));

	sf::RectangleShape backgroundRect;
	backgroundRect.setSize(sf::Vector2f(screenSize[0], screenSize[1]));
	backgroundRect.setPosition(sf::Vector2f(0, 0));

	sf::CircleShape circle(4);
	circle.setFillColor(sf::Color::Red);
	circle.setOrigin(4, 4);

	sf::Font font;
	if (!font.loadFromFile("content/BAHNSCHRIFT.ttf"))
		std::cout << "Error Loading Font!\n";

	sf::Vertex line[2];
	line[0].color = sf::Color::White;
	line[1].color = sf::Color::White;

	sf::Text text;
	text.setFillColor(sf::Color::White);
	text.setCharacterSize(200);
	text.setFont(font);
	text.setCharacterSize(40);

	double setPower = 2;

	sf::Shader shader;
	shader.loadFromFile("content/mandelbrot.glsl", sf::Shader::Fragment);

	if (!shader.isAvailable())
		std::cout << "Shaders are not available\n";

	sf::Vector2i mousePos(0, 0);
	bool leftMouseDown = false;
	bool showAnimation = true;

	bool keyPressed = false;

	sf::Clock clock;

	sf::Event event;

	while (window.isOpen())
	{
		const float power = (showAnimation) ? pow(2, 0.01 * (clock.getElapsedTime().asSeconds() - 20)) : setPower;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			else if (event.type == sf::Event::MouseWheelMoved)
				distancePerPixel *= 1 + -0.1 * event.mouseWheel.delta;
		}

		background.clear();
		shader.setUniform("centerOfScreen", sf::Glsl::Vec2(centerOfScreen[0], centerOfScreen[1]));
		shader.setUniform("resolution", sf::Glsl::Vec2(screenSize[0], screenSize[1]));
		shader.setUniform("distancePerPixel", (float)distancePerPixel);
		shader.setUniform("power", power);
		background.draw(backgroundRect, &shader);

		const auto texture = background.getTexture();
		backgroundRect.setTexture(&texture);

		window.clear();
		window.draw(backgroundRect);
		auto mouseComplexCoords = screenToComplexNumber(mousePos.x, mousePos.y);

		text.setPosition(sf::Vector2f(10, 10));
		text.setString(std::to_string(mouseComplexCoords.real()).append(" real"));
		window.draw(text);
		text.setPosition(sf::Vector2f(10, 60));
		text.setString(std::to_string(mouseComplexCoords.imag()).append(" imag"));
		window.draw(text);
		text.setPosition(sf::Vector2f(10, 110));
		text.setString("z^" + std::to_string(power).append(" + c"));
		window.draw(text);

		const auto fullIteration = iterate(mouseComplexCoords, power);

		line[0].position = fullIteration[0];
		for (size_t i = 1; i < fullIteration.size(); i++)
		{
			const auto& point = fullIteration[i];
			line[1].position = point;
			window.draw(line, 2, sf::Lines);
			line[0].position = line[1].position;
		}

		for (size_t i = 0; i < fullIteration.size(); i++)
		{
			const auto iteration = fullIteration[i];
			circle.setPosition(iteration);
			window.draw(circle);
			// UNUSED(iteration);
		}

		window.display();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			if (!keyPressed)
				clock.restart();
			keyPressed = true;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
		{
			if (!keyPressed)
			{
				showAnimation = !showAnimation;
				setPower = 2;
			}
			keyPressed = true;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
		{
			if (!keyPressed)
			{
				setPower = pow(2, 0.01 * (clock.getElapsedTime().asSeconds() - 20));
				showAnimation = !showAnimation;
			}
			keyPressed = true;
		}
		else
			keyPressed = false;

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (leftMouseDown)
			{
				centerOfScreen[0] -= (double)(sf::Mouse::getPosition(window).x - mousePos.x) * distancePerPixel;
				centerOfScreen[1] -= (double)(sf::Mouse::getPosition(window).y - mousePos.y) * distancePerPixel;
			}
			leftMouseDown = true;
		}
		else
			leftMouseDown = false;

		mousePos = sf::Mouse::getPosition(window);
	}

	return 0;
}
