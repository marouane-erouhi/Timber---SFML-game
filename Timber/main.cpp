#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>

enum class side { LEFT, RIGHT, NONE };

const int NUM_BRANCHES = 6;

void updateBranches(int seed);
void loadResources();
void restartGame();

using namespace sf;

side branchPositions[NUM_BRANCHES];
Texture textureBackground, textureTree, textureBee, textureCloud, textureBranch, texturePlayer, textureRIP, textureAxe, textureLog;
SoundBuffer chopBuffer, deathBuffer, ootBuffer;
Sprite spriteBackground, spriteTree, spriteBee, clouds[3], spritePlayer, spriteRIP, spriteAxe, spriteLog, branches[NUM_BRANCHES];
Sound chop, death, outOfTime;
//hud
Font konikap_font;//KOMIKAP_.ttf
Text scoreText, messageText;


int score = 0;
bool paused = true;
bool acceptInput = false;

float timeRemaining = 6.0f;//game
//hud
float timeBarStartWidth = 400, timeBarHeight = 80;
float timeBarWidthPreSecond = timeBarStartWidth / timeRemaining;

unsigned int screenWidth, screenHeight;
int main() {
	//create a video mode with resolution values
	VideoMode vm(1920, 1080);
	//create and open a window
	RenderWindow window(vm, "Timber game", Style::Fullscreen);

	screenWidth = window.getSize().x;
	screenHeight = window.getSize().y;


	loadResources();
	
	spriteBackground.setTexture(textureBackground);
	spriteBackground.setPosition(0,0);

	//tree
	spriteTree.setTexture(textureTree);
	spriteTree.setPosition(810, 0);

	//bee
	spriteBee.setTexture(textureBee);
	spriteBee.setPosition(0, 400);
	bool beeActive = false;//is bee moving
	float beeSpeed = 0.0f;

	//clouds
	bool cloudsActive[3];
	float cloudsSpeed[3] = { 0,0,0 };
	for (int i = 0; i < 3; i++) {
		clouds[i].setTexture(textureCloud);
		cloudsActive[i] = false;
		clouds[i].setPosition(0, 250.0f * i);
	}

	//hud
	scoreText.setFont(konikap_font);
	messageText.setFont(konikap_font);

	messageText.setString("Press enter to start!");
	scoreText.setString("Score = 0");

	messageText.setCharacterSize(75);
	scoreText.setCharacterSize(100);

	messageText.setFillColor(Color::White);
	scoreText.setFillColor(Color::White);

	FloatRect textRect = messageText.getLocalBounds();
	messageText.setOrigin(
		textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f
	);

	messageText.setPosition(screenWidth / 2.0f, screenHeight / 2.0f);
	scoreText.setPosition(20,20);

	//Time bar
	RectangleShape timeBar;

	timeBar.setSize(Vector2f(timeBarStartWidth, timeBarHeight));
	timeBar.setFillColor(Color::Red);
	timeBar.setPosition((screenWidth/2)-timeBarStartWidth/2, 980);


	Clock clock;

	for (int i = 0; i < NUM_BRANCHES; i++) {
		branches[i].setTexture(textureBranch);
		branches[i].setPosition(-2000,-2000);//out of screen

		branches[i].setOrigin(220,40);
	}

	//player code
	spritePlayer.setTexture(texturePlayer);
	spritePlayer.setPosition(580,720);
	side playerSide = side::LEFT;

	//gravestone
	spriteRIP.setTexture(textureRIP);
	spriteRIP.setPosition(600, 860);

	//axe
	spriteAxe.setTexture(textureAxe);
	spriteAxe.setPosition(700, 830);
	//axe postions
	const float AXE_POSITION_LEFT = 700;
	const float AXE_POSITION_RIGHT = 1075;

	//flying log
	spriteLog.setTexture(textureLog);
	spriteLog.setPosition(810, 720);

	bool logActive = false;
	float logSpeedX = 1000;
	float logSpeedY = -1500;


	//sounds
	chop.setBuffer(chopBuffer);
	death.setBuffer(deathBuffer);
	outOfTime.setBuffer(ootBuffer);

	//game loop
	while (window.isOpen()) {
		//event hadling
		Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case Event::KeyReleased:
				if (!paused) {
					acceptInput = true;

					//hide axe
					spriteAxe.setPosition(2000, spriteAxe.getPosition().y);
				}
				break;

			case Event::KeyPressed:
				switch (event.key.code) {
				case Keyboard::Escape:
					window.close();
					break;
				case Keyboard::Return:
					restartGame();
					break;
				case Keyboard::Right:
					if (acceptInput) {
						playerSide = side::RIGHT;
						score++;
						timeRemaining += (2 / score) + 0.15f;

						spriteAxe.setPosition(AXE_POSITION_RIGHT, spriteAxe.getPosition().y);
						spritePlayer.setPosition(1200, 720);

						updateBranches(score);

						//set the log flying to the left
						spriteLog.setPosition(810, 720);
						logSpeedX = -5000;
						logActive = true;

						acceptInput = false;

						//sound
						chop.play();
					}
					break;
				case Keyboard::Left:
					if (acceptInput) {
						playerSide = side::LEFT;
						score++;
						timeRemaining += (2 / score) + 0.15f;

						spriteAxe.setPosition(AXE_POSITION_LEFT, spriteAxe.getPosition().y);
						spritePlayer.setPosition(580, 720);

						updateBranches(score);

						//set the log flying to the right
						spriteLog.setPosition(810, 720);
						logSpeedX = 5000;
						logActive = true;

						acceptInput = false;

						//sound
						chop.play();
					}
					break;

				}//key
				break;
			}//event
		}

		//update scene
		if (!paused) {
			Time dt = clock.restart();

			//subtract dt from time
			timeRemaining -= dt.asSeconds();
			timeBar.setSize(Vector2f(timeBarWidthPreSecond * timeRemaining, timeBarHeight));

			//time runs out
			if (timeRemaining <= 0.0f) {
				paused = true;

				outOfTime.play();//sound

				messageText.setString("Out of time!!");
				//reposition the text
				FloatRect textRect = messageText.getLocalBounds();
				messageText.setOrigin(
					textRect.left + textRect.width / 2.0f,
					textRect.top + textRect.height / 2.0f
				);

				messageText.setPosition(screenWidth / 2.0f, screenHeight / 2.0f);
			}

			//moving bee
			if (!beeActive) {
				srand((int)time(0) * 10);
				beeSpeed = (rand() % 200) + 200;

				srand((int)time(0) * 10);
				float height = (rand() % 500) + 300;
				spriteBee.setPosition(screenWidth, height);
				beeActive = true;
			}
			else {
				spriteBee.setPosition(
					spriteBee.getPosition().x - (beeSpeed*dt.asSeconds()),
					spriteBee.getPosition().y
				);

				if (spriteBee.getPosition().x < -100) {
					beeActive = false;
				}
			}
			//moving clouds
			for (int i = 0; i < 3; i++) {
				if (!cloudsActive[i]) {

					// How fast is the cloud
					srand((int)time(0) * (10 + (10 * i)));
					cloudsSpeed[i] = (rand() % 200);

					// How high is the cloud
					srand((int)time(0) * (10 + (10 * i)));
					float height = (rand() % 150);
					clouds[i].setPosition(-200, height);
					cloudsActive[i] = true;
				}
				else {

					clouds[i].setPosition(
						clouds[i].getPosition().x +
						(cloudsSpeed[i] * dt.asSeconds()),
						clouds[i].getPosition().y);
					// Has the cloud reached the right hand edge of the screen?
					if (clouds[i].getPosition().x > screenWidth) {
						// Set it up ready to be a whole new cloud next frame
						cloudsActive[i] = false;
					}
				}
			}

			//update score
			std::stringstream ss;
			ss << "Score - " << score;
			scoreText.setString(ss.str());

			//update branch sprites
			for (int i = 0; i < NUM_BRANCHES; i++) {
				float height = i * 150;

				if (branchPositions[i] == side::LEFT) {
					//move sprite to left and flip it
					branches[i].setPosition(610, height);
					branches[i].setRotation(180);
				}
				else if (branchPositions[i] == side::RIGHT) {
					//move sprite to right and flip it
					branches[i].setPosition(1330, height);
					branches[i].setRotation(0);
				}
				else {
					branches[i].setPosition(3000,height);
				}
			}

			//handle flying log TODO:move into log class
			if (logActive) {
				spriteLog.setPosition(
					spriteLog.getPosition().x + ( logSpeedX * dt.asSeconds() ),
					spriteLog.getPosition().y + ( logSpeedY * dt.asSeconds() )
				);

				//has the log reached the right edge
				if (spriteLog.getPosition().x < -100 || spriteLog.getPosition().x > 2000) {
					logActive = false;
					spriteLog.setPosition(810,720);
				}
			}

			//handle lose condition
			if (branchPositions[5] == playerSide) {
				//death
				paused = true;
				acceptInput = false;

				death.play();

				//draw gravestone
				spriteRIP.setPosition(525, 760);

				//hide player
				spritePlayer.setPosition(2000, 660);

				//change message
				messageText.setString("SSQUISHED");
				FloatRect textRect = messageText.getLocalBounds();
				messageText.setOrigin(
					textRect.left + textRect.width / 2.0f,
					textRect.top + textRect.height / 2.0f
				);
				messageText.setPosition(screenWidth / 2.0f, screenHeight / 2.0f);
			}

		}//end update


		window.clear();//clear window
		//draw scenes and all
		window.draw(spriteBackground);

		for (int i = 0; i < 3; i++) {
			window.draw(clouds[i]);
		}

		for (int i = 0; i < NUM_BRANCHES; i++) {
			window.draw(branches[i]);
		}

		window.draw(spriteTree);
		window.draw(spritePlayer);
		window.draw(spriteAxe);
		window.draw(spriteLog);
		window.draw(spriteRIP);
		window.draw(spriteBee);

		//draw all text TODO:move to hud class
		window.draw(scoreText);
		if(paused)
			window.draw(messageText);
		window.draw(timeBar);

		//show window display
		window.display();
	}

	return 0;
}


void updateBranches(int seed) {
	for (int i = NUM_BRANCHES - 1; i > 0; i--) {
		branchPositions[i] = branchPositions[i - 1];
	}

	srand((int)time(0) + seed);
	int r = rand() % 5;
	switch (r) {
	case 0:
		branchPositions[0] = side::LEFT;
		break;
	case 1:
		branchPositions[0] = side::RIGHT;
		break;
	default:
		branchPositions[0] = side::NONE;
		break;
	}
}


void loadResources() {
	//font
	konikap_font.loadFromFile("fonts/KOMIKAP_.ttf");
	//textures
	textureBackground.loadFromFile("graphics/background.png");
	textureTree.loadFromFile("graphics/tree.png");
	textureBee.loadFromFile("graphics/bee.png");
	textureCloud.loadFromFile("graphics/cloud.png");
	textureBranch.loadFromFile("graphics/branch.png");
	texturePlayer.loadFromFile("graphics/player.png");
	textureRIP.loadFromFile("graphics/rip.png");
	textureAxe.loadFromFile("graphics/axe.png");
	textureLog.loadFromFile("graphics/log.png");
	//sounds
	chopBuffer.loadFromFile("sound/chop.wav");
	deathBuffer.loadFromFile("sound/death.wav");
	ootBuffer.loadFromFile("sound/out_of_time.wav");
}

void restartGame() {
	paused = false;
	score = 0;
	timeRemaining = 6.0f;

	for (int i = 0; i < NUM_BRANCHES; i++) {
		branchPositions[i] = side::NONE;
	}
	//make sure gravestone is hidden
	spriteRIP.setPosition(675, 2000);

	//set player pos
	spritePlayer.setPosition(580, 720);
	acceptInput = true;
}