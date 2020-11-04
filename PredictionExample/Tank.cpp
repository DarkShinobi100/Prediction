#include "Tank.h"



Tank::Tank(std::string color) : sf::Sprite()
{
	m_BodyTexture.loadFromFile("Assets/" + color + "Tank.png");
	m_BarrelTexture.loadFromFile("Assets/" + color + "Barrel.png");
	setTexture(m_BodyTexture);

	setOrigin(getTextureRect().width / 2, getTextureRect().height / 2);

	m_GhostSprite.setTexture(m_BodyTexture);
	m_GhostSprite.setColor(sf::Color(255, 255, 255, 128));
	m_GhostSprite.setOrigin(getTextureRect().width / 2, getTextureRect().height / 2);
	setGhostPosition( getPosition() );

	m_BarrelSprite.setTexture(m_BarrelTexture);
	m_BarrelSprite.setOrigin(6, 2);
	m_BarrelSprite.setPosition( getPosition() );
}


Tank::~Tank()
{
}

//Sets the tank's position to the latest network position
void Tank::Update(float dt)
{
	if (m_Messages.size() < 1)
		return;
	TankMessage latestMessage = m_Messages.back();
	setPosition( latestMessage.x, latestMessage.y );
}

void Tank::setPosition( float x, float y ) {
	sf::Sprite::setPosition(x, y);
	m_BarrelSprite.setPosition(getPosition());
}

//Use this to set the prediction position
void Tank::setGhostPosition( sf::Vector2f pos ) {
	m_GhostSprite.setPosition( pos );
}

//Draw the tank / or the ghost / or both
const void Tank::Render(sf::RenderWindow * window) {
	if(m_RenderMode > 0)
		window->draw(m_GhostSprite);
	if (m_RenderMode != 1) {
		window->draw(*this);
		window->draw(m_BarrelSprite);
	}
}

//Add a message to the tank's network message queue
void Tank::AddMessage(const TankMessage & msg) {
	m_Messages.push_back(msg);
}

//This method calculates and stores the position, but also returns it immediately for use in the main loop
//This is my where prediction would be... IF I HAD ANY
sf::Vector2f Tank::RunPrediction(float gameTime) {
	float predictedX = -1.0f;
	float predictedY = -1.0f;

	const int msize = m_Messages.size();
	if (msize < 3) {
		return sf::Vector2f(predictedX, predictedX);
	}
	const TankMessage& msg0 = m_Messages[msize - 1];
	const TankMessage& msg1 = m_Messages[msize - 2];
	const TankMessage& msg2 = m_Messages[msize - 3];

	// FIXME: Implement prediction here!
	// You have:
	// - the history of position messages received, in "m_Messages"
	//   (msg0 is the most recent, msg1 the 2nd most recent, msg2 the 3rd most recent)
	// - the current time, in "gameTime"
	//
	// You need to update:
	// - the predicted position at the current time, in "predictedX" and "predictedY"

	//No model prediction
	predictedX = msg0.x;
	predictedY = msg0.y;


	//Linear prediction
	//speed = distance/ Time

	sf::Vector2f velocity;
	sf::Vector2f distanceBetweenLastMessages;
	float timesinceLastMessages = gameTime - msg0.time;
	float timeBetweenLastMessages;


	distanceBetweenLastMessages.x = msg0.x - msg1.x;
	distanceBetweenLastMessages.y = msg0.y - msg1.y;
	timeBetweenLastMessages = msg0.time - msg1.time;

	velocity = distanceBetweenLastMessages / timeBetweenLastMessages;

	sf::Vector2f lastPosition = sf::Vector2f(msg0.x, msg0.y);

	//Displacement = speed * time
	sf::Vector2f Dispalcement;
	/*Dispalcement.x = velocity.x * (gameTime - msg0.time);
	Dispalcement.y = velocity.y * (gameTime - msg0.time);

	predictedX = lastPosition.x + Dispalcement.x;
	predictedY = lastPosition.y + Dispalcement.y;

	return sf::Vector2f( predictedX, predictedY );*/


	//Quadratic

	sf::Vector2f previousVelocity;
	sf::Vector2f distanceBetweenOlderMessages;
	float timeBetweenOlderMessages;

	distanceBetweenOlderMessages.x = msg1.x - msg2.x;
	distanceBetweenOlderMessages.y = msg1.y - msg2.y;

	timeBetweenOlderMessages = msg1.time - msg2.time;

	previousVelocity = distanceBetweenOlderMessages / timeBetweenOlderMessages;

	//acceleration = change in speed/change in time (between last 2 messages)
	sf::Vector2f acceleration;
	acceleration = (velocity - previousVelocity) / timeBetweenLastMessages;

	//displacement = velocity * timesinceLastMessages+0.5f * acceleration *(timesinceLastMessages^2)
	// s=ut+1/2at^2

	Dispalcement = (velocity * timesinceLastMessages) + (0.5f * acceleration * (timesinceLastMessages * timesinceLastMessages));

	predictedX = lastPosition.x + Dispalcement.x;
	predictedY = lastPosition.y + Dispalcement.y;

	return sf::Vector2f(predictedX, predictedY);
}

void Tank::Reset() {
	m_Messages.clear();
}
