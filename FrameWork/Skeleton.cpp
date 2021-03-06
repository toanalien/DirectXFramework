﻿#include "Skeleton.h"
#include"Level3.h"
#include"Bones.h"

Skeleton::Skeleton(eStatus status, GVector2 pos, int direction) : BaseEnemy(eID::SKELETON) {
	_sprite = SpriteManager::getInstance()->getSprite(eID::SKELETON);
	if (direction > 0) {
		_movingDirection = eDirection::RIGHT;
	}
	else _movingDirection = eDirection::LEFT;
	_direction = direction;

	this->_listComponent.insert(pair<string, IComponent*>("Movement", new Movement(GVector2Zero, GVector2(0,0), this->_sprite)));
	this->setStatus(status);
	this->setPosition(pos);
	this->setScale(SCALE_FACTOR);
	this->setScaleX(direction * SCALE_FACTOR);
	this->setPhysicBodySide(eDirection::ALL);
	_isOnLand = false;
	
}
Skeleton::~Skeleton() {
}

void Skeleton::init() {
	this->setHitpoint(SKELETON_HITPOINT);
	this->setScore(SKELETON_SCORE);

	this->_listComponent.insert(pair<string, IComponent*>("Gravity",
		new Gravity(GVector2(0, -900), (Movement*)(this->getComponent("Movement")))));

	auto collisionBody = new CollisionBody(this);
	_listComponent["CollisionBody"] = collisionBody;


	_animations[eStatus::RUNNING] = new Animation(_sprite, 0.15f);
	_animations[eStatus::RUNNING]->addFrameRect(eID::SKELETON, "move_01", "move_02", NULL);

	_animations[JUMPING] = new Animation(_sprite, 0.15f);
	_animations[JUMPING]->addFrameRect(eID::SKELETON, "move_01", "move_02", NULL);


	this->setStatus(eStatus::RUNNING);
	_sprite->drawBounding(false);
	this->setPhysicBodySide(eDirection::ALL);
	
	auto gravity = (Gravity*)this->_listComponent["Gravity"];
	gravity->setStatus(eGravityStatus::FALLING_DOWN);
	_speed = 250;

}

void Skeleton::update(float deltatime) {

	if (this->getStatus() == DESTROY)
		return;

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
		return;
	}



	updateDirection();

	updateStatus();

	if (_throwingStopWatch == nullptr)
	{
		_throwingStopWatch = new StopWatch();
	}
	else if (_throwingStopWatch != nullptr && _throwingStopWatch->isStopWatch(2000))
	{
		createBones();
		SAFE_DELETE(_throwingStopWatch);
	}

	for (auto component : _listComponent) 
	{
		component.second->update(deltatime);
	}

	_animations[this->getStatus()]->update(deltatime);

	for (auto object : _listBones)
	{
		object->update(deltatime);
	}
	removeBones();

}


void Skeleton::draw(LPD3DXSPRITE spritehandle, Viewport *viewport) {
	if (this->isInStatus(eStatus::DESTROY)) return;

	if (_burning != NULL)
		_burning->draw(spritehandle, viewport);
	if (this->getStatus() == eStatus::DESTROY || this->getStatus() == eStatus::BURN)
		return;
	if (this->isInStatus(eStatus::JUMPING) || this->isInStatus(eStatus::RUNNING))
		_animations[this->getStatus()]->draw(spritehandle, viewport);

	for (auto object : _listBones)
	{
		object->draw(spritehandle, viewport);
	}

}

void Skeleton::release() {
	for (auto component : _listComponent) {
		delete component.second;
	}
	_listComponent.clear();

	if (this->_burning != NULL)
		this->_burning->release();
	SAFE_DELETE(this->_burning);

	SAFE_DELETE(this->_jumpStopWatch);
	SAFE_DELETE(this->_movingStopWatch);
	//SAFE_DELETE(this->_stopwatch);

	SAFE_DELETE(this->_sprite);
}

float Skeleton::checkCollision(BaseObject *object, float dt) {
	if (this->getStatus() == eStatus::DESTROY ||
		this->isInStatus(eStatus::DYING) || this->isInStatus(eStatus::BURN))
		return 0.0f;

	for (auto childObject : _listBones)
	{
		childObject->checkCollision(object, dt);
	}

	auto collisionBody = (CollisionBody*)_listComponent["CollisionBody"];
	eID objectId = object->getId();
	eDirection direction;
	if (objectId != eID::LAND && objectId != eID::SIMON && objectId != eID::WHIP && objectId != eID::ITEM) return 0.0f;
	if (objectId == eID::LAND) {
		if (collisionBody->checkCollision(object, direction, dt)) 
		{

			auto land = (Land*)object;
			_canJumpDown = land->canJump();
			_isOnLand = true;
			//if (prevObject != NULL && land != prevObject)
			//{
			//	if (land->getBounding().bottom > prevObject->getBounding().top)
			//	{
			//		_canJump = true;
			//		_jumpLand = land;

			//		if (land->getPosition().x < _movingBounding.left)
			//		{
			//			_jumpingDirection = eDirection::LEFT;
			//		}
			//		else if (land->getPosition().x > _movingBounding.right)
			//		{
			//			_jumpingDirection = eDirection::RIGHT;
			//		}
			//	}
			//}

			if (direction == eDirection::TOP /*&& this->getVelocity().y < 0*/) 
			{
				auto gravity = (Gravity*)this->_listComponent["Gravity"];
				gravity->setStatus(eGravityStatus::SHALLOWED);

		/*		auto movement = (Movement*)this->_listComponent["Movement"];
				movement->setVelocity(GVector2(this->getVelocity().x, 0));*/
				this->removeStatus(JUMPING);
				this->setStatus(RUNNING);
				//this->setStatus(eStatus::JUMP);
				_movingBounding = object->getBounding();
				prevObject = object;
			}
			else if (((direction == eDirection::LEFT)
				|| (direction == eDirection::RIGHT))
				&& (!this->isInStatus(eStatus::JUMPING) || (this->isInStatus(eStatus::JUMPING) && !_canJumpDown)))
			{
				//this->setStatus(eStatus::NORMAL);
				//vì khi có va chạm thì vật vẫn còn di chuyển
				//nên cần dùng hàm dưới để cập nhật lại vị trí khi simon giao với wall(land với hướng va chạm trái phải)
				float moveX, moveY;
				if (collisionBody->isCollidingIntersected(object, moveX, moveY, dt))
				{
					collisionBody->updateTargetPosition(object, direction, false, GVector2(moveX, moveY));
				}
				if (!this->isInStatus(eStatus::JUMPING))
				{
					auto move = (Movement*)this->_listComponent["Movement"];
					move->setVelocity(GVector2(this->getVelocity().x, this->getVelocity().y));
				}
				else
				{
					auto move = (Movement*)this->_listComponent["Movement"];
					move->setVelocity(GVector2(this->getVelocity().x, -400));
				}
				//enableGravity(false);
				//_preObject = otherObject;
			}
			else if (prevObject == object) 
			{
				auto gravity = (Gravity*)this->_listComponent["Gravity"];
				gravity->setStatus(eGravityStatus::FALLING_DOWN);
		
				prevObject = nullptr;
			}
		}

		return 0.0f;
	}
	else
	if (objectId == eID::SIMON) {
		if (collisionBody->checkCollision(object, direction, dt, false)) {

			((Simon*)object)->getHitted();
		}
	}
	else if (objectId == eID::WHIP && ((Whip*)object)->isHitting() && collisionBody->checkCollision(object, direction, dt, false)) {
		this->setStatus(eStatus::BURN);
	}
	else if (objectId == eID::ITEM && ((Item*)object)->getItemType() == eItemType::PICKED_UP && collisionBody->checkCollision(object, direction, dt, false))
	{
		this->setStatus(eStatus::BURN);
	}

	return 0.0f;
}

GVector2 Skeleton::getVelocity() {
	auto move = (Movement*)this->_listComponent["Movement"];
	return move->getVelocity();
}

IComponent * Skeleton::getComponent(string componentName) {
	return _listComponent.find(componentName)->second;

}

void Skeleton::getHitted() {

}

void Skeleton::updateDirection() {
	if (!_isOnLand) return;
	BaseObject* _simon = ((Scene*)SceneManager::getInstance()->getCurrentScene())->getDirector()->getObjectTracker();
	GVector2 position = this->getPosition();

	if (_movingDirection == eDirection::LEFT && _simon->getPositionX() > position.x) {
		changeDirection(eDirection::RIGHT);
	}
	else if (_movingDirection == eDirection::RIGHT && _simon->getPositionX() < position.x) {
		changeDirection(eDirection::LEFT);
	}
}

void Skeleton::changeDirection(eDirection dir) {
	if (_movingDirection == dir)
		return;

	_movingDirection = dir;

	Movement *movement = (Movement*)this->getComponent("Movement");
	if (_movingDirection == eDirection::RIGHT) {
		if (this->getScale().x < 0) this->setScaleX(this->getScale().x * (-1));
	}
	else if (_movingDirection == eDirection::LEFT) {
		if (this->getScale().x > 0) this->setScaleX(this->getScale().x * (-1));
	}
}

void Skeleton::updateStatus()
{
	if (!_isOnLand) return;
	auto objectTracker = ((Level3*)SceneManager::getInstance()->getCurrentScene())->getSimon();
	int xSimon = objectTracker->getPositionX();
	int ySimon = objectTracker->getPositionY();
	if (!this->isInStatus(eStatus::JUMPING))
		moving();
	/*if (this->getPositionX() < xSimon - 220 || this->getPositionX() > xSimon + 220)
		jump();*/
}

void Skeleton::moving()
{
	if (_movingStopWatch == nullptr) 
	{
		_movingStopWatch = new StopWatch();
		return;
	}
	auto move = (Movement*)this->_listComponent["Movement"];

	if (_movingStopWatch != nullptr && _movingStopWatch->isStopWatch(500)) 
	{
		SAFE_DELETE(_movingStopWatch);
		if (this->getVelocity().x >= 0)
			move->setVelocity(GVector2(-_speed, this->getVelocity().y));
		else  if (this->getVelocity().x < 0)
			move->setVelocity(GVector2(_speed, this->getVelocity().y));
	}

	if (this->getPositionX() > _movingBounding.right - 16) {
		move->setVelocity(GVector2(-_speed, this->getVelocity().y));
	}
	else if (this->getPositionX() < _movingBounding.left + 16) {
		move->setVelocity(GVector2(_speed, this->getVelocity().y));
	}
}

void Skeleton::jump() 
{
	if (!_canJump) return;
	if (this->isInStatus(eStatus::JUMPING)) return;

	//if (_jumpStopWatch == nullptr) {
	//	_jumpStopWatch = new StopWatch();
	//}

	//if (_jumpStopWatch != nullptr && _jumpStopWatch->isStopWatch(2000)) {
		SAFE_DELETE(_jumpStopWatch);
		auto gravity = (Gravity*)this->_listComponent["Gravity"];
		gravity->setStatus(eGravityStatus::FALLING_DOWN);
		_canJump = false;
		this->setStatus(eStatus::JUMPING);
		auto move = (Movement*)this->_listComponent["Movement"];
		if (_jumpingDirection == eDirection::RIGHT)
			move->setVelocity(GVector2(85, 450));
		else move->setVelocity(GVector2(-85, 450));
	//}
}




void Skeleton::createBones()
{
	//if (_listObjects.size() > 2) return;
	Bones* bone = new Bones(this->getPosition(), _movingDirection);
	bone->init();
	//_snake = snake;
	_listBones.push_back(bone);
}

void Skeleton::removeBones()
{
	for (auto object : _listBones)
	{
		if (object->getStatus() == eStatus::DESTROY)
		{
			object->release();

			remove(_listBones.begin(), _listBones.end(), object);
			_listBones.pop_back();

			delete object;

			break;
		}
	}
}