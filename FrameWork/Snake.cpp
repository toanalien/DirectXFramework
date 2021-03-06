﻿#include "Snake.h"
#include "PlayScene.h"

Snake::Snake(GVector2 pos, eDirection direction) : BaseEnemy(eID::SNAKE) {
	_sprite = SpriteManager::getInstance()->getSprite(eID::SNAKE);
	GVector2 veloc;
	if (direction == eDirection::RIGHT)
		veloc = GVector2(SNAKE_SPEED, 0);
	else  veloc = GVector2((-1) * SNAKE_SPEED, 0);

	Movement* movement = new Movement(GVector2Zero, veloc, _sprite);
	_listComponent["Movement"] = movement;

	Gravity* gravity = new Gravity(GVector2(0, -500), movement);
	//gravity->setStatus(eGravityStatus::SHALLOWED);
	_listComponent["Gravity"] = gravity;

	auto collisionBody = new CollisionBody(this);
	_listComponent["CollisionBody"] = collisionBody;


	this->setPosition(pos);
	_direction = direction;
	this->setScale(2.0f);
}


Snake::~Snake() {}

void Snake::init() {
	this->setHitpoint(SNAKE_HITPOINT);
	this->setScore(SNAKE_SCORE);

	_animations[eStatus::RUNNING] = new Animation(_sprite, 0.1f);
	_animations[eStatus::RUNNING]->addFrameRect(eID::SNAKE, "move_01","move_02", NULL);
	_animations[eStatus::FALLING] = new Animation(_sprite, 0.1f);
	_animations[eStatus::FALLING]->addFrameRect(eID::SNAKE, "move_01", NULL);
	_animations[eStatus::DYING] = new Animation(_sprite, 0.1f);
	_animations[eStatus::DYING]->addFrameRect(eID::SNAKE, "move_02", NULL);


	//*Test
	//this->setPosition(GVector2(100,100));
	this->setStatus(eStatus::RUNNING);
	_sprite->drawBounding(false);
	this->setPhysicBodySide(eDirection::ALL);
}

void Snake::draw(LPD3DXSPRITE spritehandle, Viewport* viewport) {
	if (this->isInStatus(eStatus::DESTROY)) return;

	if (_burning != NULL)
		_burning->draw(spritehandle, viewport);
	if (this->getStatus() == eStatus::DESTROY || this->getStatus() == eStatus::BURN)
		return;

	_animations[RUNNING]->draw(spritehandle, viewport);

}

void Snake::release() {
	for (auto component : _listComponent) {
		delete component.second;
	}
	_listComponent.clear();

	//SAFE_DELETE(this->_stopwatch);
	if (_burning != nullptr)
		_burning->release();
	SAFE_DELETE(_burning);
	SAFE_DELETE(this->_sprite);
}

IComponent* Snake::getComponent(string componentName) {
	return _listComponent.find(componentName)->second;
}

void Snake::update(float deltatime) 
{

	if (this->getStatus() == DESTROY)
		return;

	checkPosition();

	if (this->getStatus() == DYING)
	{
		this->die();
		//return;
	}

	if (this->getHitpoint() <= 0) {
		this->setStatus(eStatus::BURN);
	}

	// Bị nướng
	if (this->getStatus() == eStatus::BURN) {
		if (_burning == nullptr) {
			auto pos = this->getPosition();
			_burning = new HitEffect(2, pos);
			_burning->init();
		}
		else {
			_burning->update(deltatime);
			if (_burning->getStatus() == eStatus::DESTROY) {
				this->setStatus(eStatus::DESTROY);
			}
		}
	}

	for (auto it : _listComponent) 
	{
		it.second->update(deltatime);
	}

	_animations[RUNNING]->update(deltatime);
}


float Snake::checkCollision(BaseObject * otherObject, float dt) {
	if (this->getStatus() == eStatus::DESTROY || this->isInStatus(eStatus::BURN) || this->isInStatus(eStatus::DYING))
		return 0.0f;

	auto collisionBody = (CollisionBody*)_listComponent["CollisionBody"];
	eID otherObjectId = otherObject->getId();
	eDirection direction;
	if (otherObjectId != eID::LAND && otherObjectId != eID::SIMON && otherObjectId != eID::ITEM
		&& otherObjectId != eID::WHIP) return 0.0f;
	if (collisionBody->checkCollision(otherObject, direction, dt,false))
	{
		if (otherObjectId == eID::LAND && direction == eDirection::TOP)
		{
			float moveX, moveY;
			if (collisionBody->isCollidingIntersected(otherObject, moveX, moveY, dt))
			{
				collisionBody->updateTargetPosition(otherObject, direction, false, GVector2(moveX, moveY));
			}

			auto gravity = (Gravity*)this->_listComponent["Gravity"];
			gravity->setStatus(eGravityStatus::SHALLOWED);
		}
		else if (otherObjectId == eID::LAND && (direction == eDirection::RIGHT || direction == eDirection::LEFT))
		{
			this->setStatus(DYING);
		}
		else if (otherObjectId == eID::SIMON)
		{
			((Simon*)otherObject)->getHitted(1);	
			this->setStatus(eStatus::DYING);
		}
		else if (otherObjectId == eID::ITEM || (otherObjectId == eID::WHIP && ((Whip*)otherObject)->isHitting()))
		{
			this->setStatus(BURN);
		}
	}
	return 0.0f;
	
}

void Snake::updateCurrentAnimateIndex() {

	if (this->isInStatus(eStatus::RUNNING)) 
	{
		_currentAnimationIndex = eStatus::RUNNING;
	}
}

GVector2 Snake::getVelocity() {
	auto move = (Movement*)this->_listComponent["Movement"];
	return move->getVelocity();
}

void Snake::die()
{
	auto movement = (Movement*)this->_listComponent["Movement"];
	movement->setVelocity(GVector2(0,-200));
	auto gravity = (Gravity*)this->_listComponent["Gravity"];
	gravity->setStatus(eGravityStatus::FALLING_DOWN);
	gravity->setGravity(GVector2(0, -900));
}

void Snake::checkPosition()
{
	auto viewportTracker = ((PlayScene*)SceneManager::getInstance()->getCurrentScene())->getViewport();
	RECT vpBound = viewportTracker->getBounding();

	if (this->getPositionX() < vpBound.left || this->getPositionX() > vpBound.right
		|| this->getPositionY() < vpBound.bottom)
		this->setStatus(DESTROY);
}
